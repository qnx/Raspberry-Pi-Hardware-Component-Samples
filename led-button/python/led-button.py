import rpi_gpio as GPIO  # Import the QNX Raspberry Pi GPIO module for controlling GPIO pins
import time # Import the time module for adding delays

def buttonPressed(pin):
    # Check if the button (pin 20) is pressed
    if GPIO.input(20) == GPIO.LOW:
        # Turn on the LED (pin 16)
        GPIO.output(16, GPIO.HIGH) 
    else:
        # Turn off the LED (pin 16)
        GPIO.output(16, GPIO.LOW) 

# Set GPIO pin 16 as an output pin for controlling an LED
GPIO.setup(16, GPIO.OUT)
# Ensure the LED is off initially
GPIO.output(16, GPIO.LOW)

# Set GPIO pin 20 as an input with an internal pull-up resistor
GPIO.setup(20, GPIO.IN,GPIO.PUD_UP)
# Add an event listener to detect button state changes on pin 20
GPIO.add_event_detect(20, GPIO.BOTH, callback=buttonPressed)

# Keep the script running to continuously monitor button presses
while True:
    time.sleep(1) # Sleep to reduce CPU usage