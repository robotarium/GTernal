import serial.tools.list_ports

print([comport.device for comport in serial.tools.list_ports.comports() if 'ttyAMA' in comport.device][0])
