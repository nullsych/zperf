# python3.10.exe .\tools\tio.py

import serial, sys, glob

WORK_PORT = ''

def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
            print('Open ' + str(port) + ' sucessfully! Data:\n')
        except (OSError, serial.SerialException):
            pass
    return result

try:
    WORK_PORT = serial_ports()[0]
except IndexError:
    print("\nError: No valid COM port found!\n")
    sys.exit(0)

serialPort = serial.Serial(port=WORK_PORT, baudrate=115200, timeout=.1)

serialString = ""

while(1):

    try:
        if(serialPort.in_waiting > 0):
            serialString = serialPort.readline()
            print(serialString.decode('Ascii'), end='')

    except KeyboardInterrupt:
        print("\nCtrl-C, exiting..")
        sys.exit()

    except UnicodeDecodeError:
        print("\nCodec can't decode byte in position, try one more time...")