import serial
import json
import logging
import threading
import time

global logger
logging.basicConfig(format='%(asctime)s - %(levelname)s - %(module)s - %(message)s')
logger = logging.getLogger('root')
logger.setLevel(logging.DEBUG)

# Constants
MAX_IN_WAITING = 500


def _json_to_bytes(message):
    """Dumps json data to ASCI

    Raises:
        Exception: If the message cannot be JSON encoded.

    """

    return json.dumps(message).encode('ASCII')


def _bytes_to_json(message):
    """Converts from ASCII to JSON

    Raises:
        Exception: If the message cannot be JSON decoded.

    """

    return json.loads(message.decode('ASCII'))


class GritsbotSerial:
    """Encapsulates serial communications to the microcontroller.

    Serial communications are based on a request/response architecture.  This class assumes nothing about the form of these, except that they are JSON-
    encodable.

    This class is made to be robust to errors and will restart the serial device if it encounters an error (e.g., if the cable is un/replugged).

    Attributes:
        _serial_dev (str): Path to the serial device.
        _baud_rate (int): The baud rate for the serial device.
        _timeout (int): Timeout for the serial reads in seconds.
        _serial_cv (threading.Condition): Condition variable for synchronizing class.
        _serial (serial.Serial): The pyserial object for serial communications.
        _serial_task_thread (threading.Thread): Runs the internal restart task.
        _stopped (bool): Whether the class has been stopped.
        _started (bool): Whether the class has been started.
        _needs_restart (bool): Whether the serial device should be restarted.

    """

    def __init__(self, serial_dev='/dev/ttyAMA0', baud_rate=500000, timeout=2):
        """Creates the serial communciations object.

        Args:
            serial_dev (str, optional): The path to the serial device.
            baud_rate (int): Baud rate for the serial device.
            timeout (int): Timeout for the serial read.

        Examples:
            >>> GritsbotSerial(serial_dev='/dev/ttyACM0', baud_rate=115200, timeout=5)

        """
        self._serial_dev = serial_dev
        self._baud_rate = baud_rate
        self._timeout = timeout

        # Serial-related attributes.  ALL OF THESE SHOULD BE CONTROLLED WHILE HOLDING THE LOCK
        self._serial_cv = threading.Condition()
        self._serial = None
        self._serial_task_thread = None
        self._stopped = False
        self._started = False
        self._needs_restart = True

    def serial_request(self, msg, timeout=5):
        """Makes a request on a serial line

        Args:
            msg: A JSON-encodable (by json.dumps) object

        Raises:
            RuntimeError: If the serial port has too many incoming bytes (specified by MAX_IN_WAITING); if the serial port cannot be written to or read from;
            if the serial port has not been initialized.

        Returns:
            dict: JSON-formatted dict containing the return message.

        Examples:
            >>> response = GritsbotSerial.serial_request(request, timeout=1)

        """
        with self._serial_cv:
            if(not self._started):
                error_msg = 'Serial connection must be started prior to calling this method.'
                logger.critical(error_msg)
                raise RuntimeError(error_msg)

            if(self._stopped):
                error_msg = 'Serial connection stopped!  Cannot use anymore.'
                logger.critical(error_msg)
                raise RuntimeError(error_msg)

            # Wait until the serial_task thread has restarted the serial device.
            while(self._needs_restart):
                if(not self._serial_cv.wait(timeout=timeout)):
                    # If the CV times out, it returns false
                    error_msg = 'Serial connection timed out!'
                    logger.critical(error_msg)
                    raise RuntimeError(error_msg)

            msg = _json_to_bytes(msg)
            self._serial.reset_input_buffer()

            try:
                self._serial.write(msg)
            except Exception as e:
                error_msg = 'Unable to write to the serial port.'
                logger.critical(error_msg)
                logger.critical(repr(e))

                # Signal the serial_task thread that the serial device should be restarted
                self._needs_restart = True
                self._serial_cv.notify_all()
                raise RuntimeError(error_msg)

            try:
                msg = self._serial.read()
                time.sleep(0.01) # Wait for 10ms to account for the delay in UART
            except Exception as e:
                error_msg = 'Unable to read from serial port'
                logger.critical(error_msg)
                logger.critical(repr(e))

                # Signal the serial_task thread that the serial device should be restarted
                self._needs_restart = True
                self._serial_cv.notify_all()
                raise RuntimeError(error_msg)

            if(self._serial.in_waiting > MAX_IN_WAITING):
                error_msg = 'Too many incoming bytes waiting on serial port ({})'.format(self._serial.in_waiting)
                logger.warning(error_msg)

                # Signal the serial_task thread that the serial device should be restarted
                self._needs_restart = True
                self._serial_cv.notify_all()
                raise RuntimeError(error_msg)

            # Once bytes are available, read the rest in.  We assume that the entire message is on
            # the line
            try:
                msg += self._serial.read(self._serial.in_waiting)
            except Exception as e:
                error_msg = 'Unable to read from the serial port.'
                logger.critical(error_msg)
                logger.critical(repr(e))

                # Signal the serial_task thread that the serial device should be restarted
                self._needs_restart = True
                self._serial_cv.notify_all()
                raise RuntimeError(error_msg)

            result = None
            try:
                result = _bytes_to_json(msg)
            except Exception as e:
                logger.warning('Unable to parse JSON message from serial port')
                logger.warning(repr(e))

            return result

    def start(self, timeout=5):
        """Starts the serial line by attempting to establish a serial for the specified device.

        This method should be called prior to other methods of this class.

        Raises:
            RuntimeError: If the serial connection cannot be established within timeout.

        """
        # Wait for initial connection
        with self._serial_cv:
            if(self._started):
                logger.critical('Cannot start the serial connection more than once!')
                raise RuntimeError()

            if(self._stopped):
                logger.critical('Cannot start the serial connection once stopped.')
                raise RuntimeError()

            # If it's already been started, we can't get to this part of the code
            self._started = True
            self._serial_task_thread = threading.Thread(target=self._serial_task)
            self._serial_task_thread.start()

            # Wait to acquire the serial connection once
            while(self._needs_restart):
                if(not self._serial_cv.wait(timeout=timeout)):
                    logger.critical('Initial serial connection timed out.')
                    # If we're in this state, then we have the lock, and the serial_task thread cannot acquire it.
                    # However, if we're very unlucky, the serial device my have been acquired by the time we get here.  Either way,
                    # it should be safe to just call stop and exit.

                    # We can call self.stop here, since the underlying lock is reentrant
                    self.stop()
                    raise RuntimeError()

    def stop(self):
        """Stops the serial connection.

        Closes the underlying pyserial connection.  If the serial connection has not been started, does nothing.

        """
        # If we've previously started serial, stop it.
        with self._serial_cv:
            self._stopped = True
            self._serial_cv.notify_all()

            # If the serial connection was never started, don't shut it down
            if(self._started):
                if(self._serial is not None):
                    self._serial.close()
                    self._serial = None

        # Thread won't finish until lock can be acquired, so we need to put this outside the lock
        self._serial_task_thread.join()

    def _serial_task(self):
        """Restarts the serial if the serial device stops responding.

        Only meant to be run by the interal thread!

        """
        while (not self._stopped):
            start_time = time.time()

            with self._serial_cv:
                while (not self._needs_restart and not self._stopped):
                    self._serial_cv.wait()

                if(self._stopped):
                    break
                else:
                    # Need to restart serial
                    if(self._serial is not None):
                        self._serial.close()
                        self._serial = None

                    try:
                        self._serial = serial.Serial(self._serial_dev, self._baud_rate, bytesize=serial.EIGHTBITS, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, timeout=self._timeout, xonxoff=False, rtscts=False, dsrdtr=False)
                        # If we succeeded, no longer need to restart serial
                        self._needs_restart = False
                        self._serial_cv.notify_all()
                    except Exception as e:
                        logger.critical('Could not get serial device ({})'.format(self._serial_dev))
                        logger.critical(repr(e))

            # Wait at least one second between retries
            time.sleep(max(0, 1 - (time.time() - start_time)))
