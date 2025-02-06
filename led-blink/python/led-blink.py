import rpi_gpio as GPIO  # Import the Raspberry Pi GPIO library for controlling GPIO pins
 
# Set GPIO pin 16 as an output pin
GPIO.setup(16, GPIO.OUT)
 
# Turn on the LED (Set GPIO pin 16 to HIGH)
GPIO.output(16, GPIO.HIGH)