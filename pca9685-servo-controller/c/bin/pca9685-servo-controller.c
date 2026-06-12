/**
 * Copyright (c) 2026, BlackBerry Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <pca9685.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_help(void) {
  printf("pca9685-servo-controller [options] [pin] on off\n");
  printf("CLI PWM controller for PCA9685 device\n");
  printf("\n");
  printf("Arguments:\n");
  printf("\tpin:  Which pin is targeted for command. If no pin is given all "
         "pin are set.\n");
  printf("\ton: When the signal should transition from low to high (0-4096)\n");
  printf("\toff: When the signal should transition from high to low (0-4096)\n");
  printf("\nOptions:\n");
  printf("\t-h:        Print this menu and exit.\n");
  printf("\t-a <addr>: I2C address of PCA9685 PWM controller (default 0x40)\n");
  printf("\t-b <bus> : I2C Bus of PCA9685 PWM controller (default 1)\n");
  printf("\t-f <freq>: Frequency of PWM; specify before setting any pins\n");
}

int main(int argc, char *argv[]) {
  int freq = -1;
  int pin = -1;
  int bus = PCA9685_DEFAULT_I2C_BUS;
  int addr = PCA9685_DEFAULT_I2C_ADDR;

  int opt;
  while ((opt = getopt(argc, argv, "ha:b:f:")) != -1) {
    switch (opt) {
    case 'h':
      print_help();
      exit(0);
    case 'a':
      addr = atoi(optarg);
      break;
    case 'b':
      bus = atoi(optarg);
      break;
    case 'f':
      freq = atoi(optarg);
      break;
    default:
      fprintf(stderr, "Unknown or Invalid argument\n");
      exit(1);
    }
  }

  if (optind + 1 >= argc) {
    fprintf(stderr, "Missing positional arguments\n");
    print_help();
    exit(1);
  }

  if (optind + 2 < argc) {
    pin = atoi(argv[optind]);
    optind++;
  }
  int on = atoi(argv[optind]);
  int off = atoi(argv[optind + 1]);
  if (off > 4096 || on > 4096 || off < 0 || on < 0) {
    fprintf(stderr, "Invalid values for off and/or on pwm values\n");
    print_help();
    exit(1);
  }

  /**
   * Before using the driver we must initialize it to ensure all our resources
   * are available. We are using the default address, and bus, but this can be
   * updated based on your use case.
   */
  pwm_driver_pca9685_t driver;
  int res = pwm_driver_pca9685_init(&driver, bus, addr);
  if (res != EOK) {
    fprintf(stderr, "Failed to init PCA9685 driver with: %s\n", strerror(res));
    return 1;
  }


  /**
   * Before using the PCA9685 driver it is a good idea
   * to reset it to ensure it is in a good state.
   *
   * This is not strictly required.
   */
  res = pwm_driver_pca9685_soft_reset(&driver);
  if (res != EOK) {
    fprintf(stderr, "Failed to soft reset %s\n", strerror(res));
    pwm_driver_pca9685_destroy(&driver);
    return 1;
  }

  if (freq != -1) {
    printf("Setting frequency to %d\n", freq);
    pwm_driver_pca9685_set_frequency(&driver, freq);
    if (res != EOK) {
      fprintf(stderr, "Failed to set frequency\n");
      pwm_driver_pca9685_destroy(&driver);
      return 1;
    }
  }

  if (pin == -1) {
    printf("Setting all pins to on=%d off=%d\n", on, off);
    pwm_driver_pca9685_set_pwm_all(&driver, on, off);
  } else {
    printf("Setting pin=%d to on=%d off=%d\n", pin, on, off);
    pwm_driver_pca9685_set_pwm(&driver, pin, on, off);
  }

  /** It takes time to move the servo so sleep before destroying the driver */
  sleep(1);

  /**
   * Whenever you are finished using the driver it is
   * important to destroy it to release the i2c resources.
   */
  pwm_driver_pca9685_destroy(&driver);
  return 0;
}
