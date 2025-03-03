import smbus
import time

adc = smbus.SMBus(1)

while True:
    value = adc.read_byte_data(0x48, 0x40)
    value = adc.read_byte_data(0x48, 0x40)
    print('New 10K POT value:  {}'.format(value))

    value = adc.read_byte_data(0x48, 0x41)
    value = adc.read_byte_data(0x48, 0x41)
    print('New 5537 Photoresistor value:  {}'.format(value))

    value = adc.read_byte_data(0x48, 0x42)
    value = adc.read_byte_data(0x48, 0x42)
    print('New MF58 Thermistor value:  {}'.format(value))

    value = adc.read_byte_data(0x48, 0x43)
    value = adc.read_byte_data(0x48, 0x43)
    print('New AIN3 value:  {}'.format(value))

    time.sleep(0.1)
