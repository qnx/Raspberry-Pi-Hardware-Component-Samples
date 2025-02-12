import rpi_gpio as GPIO
import time

# Define the required pins
GPIO_LED_PIN = 20 # GPIO pin 20, Controls the LED
GPIO_PIR_PIN = 21 # GPIO pin 21, Takes input from the PIR sensor

# Define the LED pin to be an output
GPIO.setup(GPIO_LED_PIN, GPIO.OUT)
# Define the PIR pin to be an input
GPIO.setup(GPIO_PIR_PIN, GPIO.IN)

def main():
    # The while loop continuously reads the data from the PIR sensor and turns the LED on if motion is detected
    while(True):
        # If the received value from the PIR sensor is one, motion is detected and the LED is turned on
        # Otherwise the LED is turned off
        if (GPIO.input(GPIO_PIR_PIN) == 1):
            GPIO.output(GPIO_LED_PIN, GPIO.OUT)
            print("Motion Detected!!!")
        else:
            GPIO.output(GPIO_LED_PIN, GPIO.LOW)
            print("No Motion")
        
        time.sleep(0.1) # Sleep for 100 ms

if __name__ == "__main__":
    main()