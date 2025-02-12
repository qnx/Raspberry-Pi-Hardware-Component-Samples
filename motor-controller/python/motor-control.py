import time
import rpi_gpio as GPIO

# Define the pins required to control the motors
MOTOR_EN_PIN = 26 # GPIO pin 26, Enable pin is used to turn on and off the motors
MOTOR_RP = 10 # GPIO pin 10, Controls the input to the positive terminal off the right motor
MOTOR_RN = 9 # GPIO pin 9, Controls the input to the negative terminal off the right motor
MOTOR_LP = 8 # GPIO pin 8, Controls the input to the positive terminal off the left motor
MOTOR_LN = 11 # GPIO pin 11, Controls the input to the negative terminal off the left motor

# Initialize all of the defined pins to be outputs
GPIO.setup(MOTOR_EN_PIN, GPIO.OUT)
GPIO.setup(MOTOR_RP, GPIO.OUT)
GPIO.setup(MOTOR_RN, GPIO.OUT)
GPIO.setup(MOTOR_LP, GPIO.OUT)
GPIO.setup(MOTOR_LN, GPIO.OUT)

# Set our enable to pin to an on state allowing our motors to move
GPIO.output(MOTOR_EN_PIN, GPIO.HIGH)


# Drive Forward for 2 seconds
GPIO.output(MOTOR_RP, GPIO.HIGH)
GPIO.output(MOTOR_RN, GPIO.LOW)
GPIO.output(MOTOR_LP, GPIO.HIGH)
GPIO.output(MOTOR_LN, GPIO.LOW)

time.sleep(2)


# Drive Backwards for 2 seconds
GPIO.output(MOTOR_RP, GPIO.LOW)
GPIO.output(MOTOR_RN, GPIO.HIGH)
GPIO.output(MOTOR_LP, GPIO.LOW)
GPIO.output(MOTOR_LN, GPIO.HIGH)

time.sleep(2)

# Drive Left for 2 seconds
GPIO.output(MOTOR_RP, GPIO.HIGH)
GPIO.output(MOTOR_RN, GPIO.LOW)
GPIO.output(MOTOR_LP, GPIO.LOW)
GPIO.output(MOTOR_LN, GPIO.HIGH)

time.sleep(2)


# Drive Right for 2 seconds
GPIO.output(MOTOR_RP, GPIO.LOW)
GPIO.output(MOTOR_RN, GPIO.HIGH)
GPIO.output(MOTOR_LP, GPIO.HIGH)
GPIO.output(MOTOR_LN, GPIO.LOW)

time.sleep(2)

# To stop all the motors set the pins to Low
GPIO.output(MOTOR_EN_PIN, GPIO.LOW)
GPIO.output(MOTOR_RP, GPIO.LOW)
GPIO.output(MOTOR_RN, GPIO.LOW)
GPIO.output(MOTOR_LP, GPIO.LOW)
GPIO.output(MOTOR_LN, GPIO.LOW)