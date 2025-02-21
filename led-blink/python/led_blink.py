import rpi_gpio as GPIO  # Import the QNX Raspberry Pi GPIO module for controlling GPIO pins
import time  # Import the time module for adding delays
 
# Set GPIO pin 16 as an output pin
GPIO.setup(16, GPIO.OUT)
 
# Initially set GPIO pin 16 to LOW (turning it OFF)
GPIO.output(16, GPIO.LOW)
 
# Infinite loop to blink the LED
while True:
    GPIO.output(16, GPIO.HIGH)  # Turn LED ON
    time.sleep(0.5)  # Wait for 0.5 seconds
 
    GPIO.output(16, GPIO.LOW)  # Turn LED OFF
    time.sleep(0.5)  # Wait for 0.5 seconds