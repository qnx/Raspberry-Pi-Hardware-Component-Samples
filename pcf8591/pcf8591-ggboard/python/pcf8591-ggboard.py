#
# Copyright (c) 2025, BlackBerry Limited. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

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
