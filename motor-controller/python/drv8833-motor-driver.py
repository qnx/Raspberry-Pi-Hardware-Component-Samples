#!/usr/bin/env python3
"""
Copyright (c) 2025, BlackBerry Limited. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

Motor Driver Module for QNX using rpi_gpio.
 
This module demonstrates how to control the motor driver using a PWM signal on the enable (EN)
pin for speed control and digital outputs for motor direction. The DRV8833 motor driver’s speed
is controlled via a PWM signal on the enable (nSLEEP) pin, while the other pins set the motor
direction.
  
IMPORTANT:
  The motor enable (EN) pin MUST be configured for PWM output so that adjusting the duty cycle
  changes the motor speed.
"""
 
import rpi_gpio as GPIO  # Raspberry Pi GPIO module
import time
import sys
 
# GPIO pin assignments (using BCM numbering)
MOTOR_EN_PIN  = 18   # Motor enable (nSLEEP) pin, PWM controlled
MOTOR_RP_PIN  = 10   # Right Motor "Forward" input (AIN1)
MOTOR_RN_PIN  = 9    # Right Motor "Reverse" input (AIN2)
MOTOR_LP_PIN  = 8    # Left Motor "Forward" input (BIN1)
MOTOR_LN_PIN  = 11   # Left Motor "Reverse" input (BIN2)
 
# PWM frequency in Hz for motor speed control
PWM_FREQ = 1000
 
class MotorDriver:
    """
    Class to manage the motor driver.
    """
    def __init__(self):
        """
        Initializes the motor driver by configuring the GPIO pins.
 
        - Sets the GPIO mode to BCM.
        - Configures the direction pins as digital outputs and initializes them to LOW.
        - Sets up the enable pin for PWM output.
        """
        print("[motor_init] Initializing motor driver...")
        # Use BCM numbering for GPIO pins.
        GPIO.setmode(GPIO.BCM)
 
        # Configure motor direction pins as outputs.
        GPIO.setup(MOTOR_RP_PIN, GPIO.OUT)
        GPIO.setup(MOTOR_RN_PIN, GPIO.OUT)
        GPIO.setup(MOTOR_LP_PIN, GPIO.OUT)
        GPIO.setup(MOTOR_LN_PIN, GPIO.OUT)
 
        # Initialize direction pins to LOW (safe state).
        GPIO.output(MOTOR_RP_PIN, GPIO.LOW)
        GPIO.output(MOTOR_RN_PIN, GPIO.LOW)
        GPIO.output(MOTOR_LP_PIN, GPIO.LOW)
        GPIO.output(MOTOR_LN_PIN, GPIO.LOW)
 
        # Set up the enable pin for PWM output.
        # IMPORTANT: MOTOR_EN_PIN must be configured for PWM to allow dynamic speed control.
        GPIO.setup(MOTOR_EN_PIN, GPIO.OUT)
        self.pwm = GPIO.PWM(MOTOR_EN_PIN, PWM_FREQ)
        self.pwm.start(0)  # Start PWM with a 0% duty cycle (motor off)
 
        time.sleep(0.1)  
        print("[motor_init] Motor driver initialization complete.")
 
    def set_speed(self, speed_percent):
        """
        Updates the PWM duty cycle on the enable pin to control motor speed.
 
        :param speed_percent: Desired motor speed as a percentage (0.0 to 100.0)
        """
        self.pwm.ChangeDutyCycle(speed_percent)
 
    def disable(self):
        """
        Disables the motor driver by setting the PWM duty cycle to 0%.
        """
        self.set_speed(0)
 
    def right_forward(self):
        """
        Sets the right motor to move forward.
        Right Motor Forward: HIGH on MOTOR_RP_PIN, LOW on MOTOR_RN_PIN.
        """
        GPIO.output(MOTOR_RP_PIN, GPIO.HIGH)
        GPIO.output(MOTOR_RN_PIN, GPIO.LOW)
 
    def right_reverse(self):
        """
        Sets the right motor to move in reverse.
        Right Motor Reverse: LOW on MOTOR_RP_PIN, HIGH on MOTOR_RN_PIN.
        """
        GPIO.output(MOTOR_RP_PIN, GPIO.LOW)
        GPIO.output(MOTOR_RN_PIN, GPIO.HIGH)
 
    def left_forward(self):
        """
        Sets the left motor to move forward.
        Note: Left motor wiring is reversed.
        Left Motor Forward: LOW on MOTOR_LP_PIN, HIGH on MOTOR_LN_PIN.
        """
        GPIO.output(MOTOR_LP_PIN, GPIO.LOW)
        GPIO.output(MOTOR_LN_PIN, GPIO.HIGH)
 
    def left_reverse(self):
        """
        Sets the left motor to move in reverse.
        Note: Left motor wiring is reversed.
        Left Motor Reverse: HIGH on MOTOR_LP_PIN, LOW on MOTOR_LN_PIN.
        """
        GPIO.output(MOTOR_LP_PIN, GPIO.HIGH)
        GPIO.output(MOTOR_LN_PIN, GPIO.LOW)
 
    def stop(self):
        """
        Stops both motors by setting all direction pins to LOW and disabling PWM speed.
        """
        GPIO.output(MOTOR_RP_PIN, GPIO.LOW)
        GPIO.output(MOTOR_RN_PIN, GPIO.LOW)
        GPIO.output(MOTOR_LP_PIN, GPIO.LOW)
        GPIO.output(MOTOR_LN_PIN, GPIO.LOW)
        self.disable()
 
    def turn_left(self):
        """
        Pivots the robot to the left by setting the left motor in reverse and the right motor forward.
        """
        self.left_reverse()
        self.right_forward()
 
    def turn_right(self):
        """
        Pivots the robot to the right by setting the left motor forward and the right motor in reverse.
        """
        self.left_forward()
        self.right_reverse()
 

def main():
    """
    Main function to demonstrate motor control.
 
    This routine:
      - Initializes the motor driver.
      - Ramps the PWM duty cycle (speed) from 0% to 100% and then from 100% down to 0%.
      - Tests reverse drive and pivot turning maneuvers.
      - Disables the motor and cleans up on completion or interruption.
    """
    print("=== DRV8833 Motor Driver PWM Demo (Enable Pin Speed Control) ===")
    driver = MotorDriver()
 
    try:
        # Set both motors for forward drive.
        driver.right_forward()
        driver.left_forward()
 
        # Reverse drive test.
        print("\n--- Driving reverse at 50% speed ---")
        driver.right_reverse()
        driver.left_reverse()
        driver.set_speed(50)
        time.sleep(2)
        driver.stop()
        time.sleep(1)
 
        # Pivot turn: left.
        print("\n--- Pivot Turning LEFT at 30% speed ---")
        driver.turn_left()
        driver.set_speed(30)
        time.sleep(2)
        driver.stop()
        time.sleep(1)
 
        # Pivot turn: right.
        print("\n--- Pivot Turning RIGHT at 30% speed ---")
        driver.turn_right()
        driver.set_speed(30)
        time.sleep(2)
        driver.stop()
        time.sleep(1)
 
        print("\n=== Test Complete ===")
 
    except KeyboardInterrupt:
        print("\nSIGINT received, disabling motor driver...")
        driver.disable()
 
    finally:
        sys.exit(0)
 
if __name__ == '__main__':
    main()
