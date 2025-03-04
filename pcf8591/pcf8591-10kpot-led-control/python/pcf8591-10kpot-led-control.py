import smbus
import time

adc = smbus.SMBus(1)

prev_value = 0
while True:
    value = adc.read_byte_data(0x48, 0x40)
    if prev_value != value:
        print('New 10K POT value:  {}'.format(value))
        prev_value = value

        adc.write_byte_data(0x48, 0x40, value)
        print('Outputting value to DAC (255 = 3.3V): {}'.format(value))

    time.sleep(0.1)
