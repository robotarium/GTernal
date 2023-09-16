import serial.tools.list_ports


def main():
    print([comport.device for comport in serial.tools.list_ports.comports() if 'ttyACM' in comport.device][0])

if __name__ == '__main__':
    main()
