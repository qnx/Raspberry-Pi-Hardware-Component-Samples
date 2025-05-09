/*
 * Copyright (c) 2025, BlackBerry Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <time.h>
 #include <unistd.h>
 #include "rpi_gpio.h"
 
 // Define GPIO pins for rotary encoder and button
 #define CLK_GPIO GPIO17
 #define DT_GPIO GPIO18
 #define SW_GPIO GPIO27
 
 // Minimum time between valid button presses (in nanoseconds)
 #define DEBOUNCE_THRESHOLD_NS 200000000
 
 // Event identifiers for incoming GPIO events
 enum rotary_event_t {
     EVENT_BUTTON,
     EVENT_ROTARY_CLK,
     EVENT_ROTARY_DT,
 };
 
 static int chid;  // Channel ID for QNX message passing
 static int coid;  // Connection ID for receiving pulses
 
 // Flag to control main loop execution
 bool running = true;
 
 // Initialize QNX message channel and attach connection
 static bool init_channel(void) {
     chid = ChannelCreate(_NTO_CHF_PRIVATE);
     if (chid == -1)
     {
         perror("ChannelCreate");
         return false;
     }
 
     coid = ConnectAttach(0, 0, chid, _NTO_SIDE_CHANNEL, 0);
     if (coid == -1)
     {
         perror("ConnectAttach");
         return false;
     }
 
     return true;
 }
 
 // Configure a GPIO pin for input with pull-up and optional event detection
 static bool init_input(int gpio_pin, int event_id) {
     if (rpi_gpio_setup_pull(gpio_pin, GPIO_IN, GPIO_PUD_UP))
     {
         perror("rpi_gpio_setup_pull");
         return false;
     }
 
     // Set up event detection only if a valid event_id is provided
     if (event_id >= 0)
     {
         if (rpi_gpio_add_event_detect(gpio_pin, coid, GPIO_FALLING, event_id))
         {
             perror("rpi_gpio_add_event_detect");
             return false;
         }
     }
 
     return true;
 }
 
 // Read digital value from GPIO pin
 static int read_pin(int gpio_pin) {
     unsigned value;
     if (rpi_gpio_input(gpio_pin, &value)) {
         perror("rpi_gpio_input");
         return -1;
     }
     return value;
 }
 
 // Signal handler to gracefully exit the main loop
 static void ctrl_c_handler(int signum) {
     (void)(signum);
     running = false;
 }
 
 // Register signal handlers for SIGINT and SIGTERM
 static void setup_handlers(void) {
     struct sigaction sa =
     {
         .sa_handler = ctrl_c_handler,
     };
     sigaction(SIGINT, &sa, NULL);
     sigaction(SIGTERM, &sa, NULL);
 }
 
 // Return true if enough time has passed since last event
 bool debounce(struct timespec *last_event_time, long threshold_ns) {
     struct timespec now;
     clock_gettime(CLOCK_MONOTONIC, &now);
     long elapsed_ns = (now.tv_sec - last_event_time->tv_sec) * 1000000000L +
                       (now.tv_nsec - last_event_time->tv_nsec);
     if (elapsed_ns < threshold_ns) return false;
     *last_event_time = now;
     return true;
 }
 
 int main(void) {
 
     setup_handlers();
 
     // Set up communication channel
     if (!init_channel()) {
         return EXIT_FAILURE;
     }
 
     // Initialize rotary encoder and button pins
     if (!init_input(CLK_GPIO, EVENT_ROTARY_CLK))
     {
         return EXIT_FAILURE;
     }
 
     if (!init_input(DT_GPIO, EVENT_ROTARY_DT))
     {
         return EXIT_FAILURE;
     }
 
     if (!init_input(SW_GPIO, EVENT_BUTTON))
     {
         return EXIT_FAILURE;
     }
 
     // Read initial state of CLK to detect transitions
     int last_clk_state = read_pin(CLK_GPIO);
     if (last_clk_state == -1)
     {
         return EXIT_FAILURE;
     }
 
     int position = 0;  // Rotary encoder position counter
 
     // Time of last button press (for debouncing)
     struct timespec last_button_event_time = {0};
 
     while (running) {
 
         struct _pulse pulse;
 
         // Wait for a pulse from a GPIO event
         if (MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1)
         {
             perror("MsgReceivePulse()");
             return EXIT_FAILURE;
         }
 
         // Ignore unexpected pulses
         if (pulse.code != _PULSE_CODE_MINAVAIL)
         {
             fprintf(stderr, "Unexpected pulse code %d\n", pulse.code);
             return EXIT_FAILURE;
         }
 
         // Handle the specific GPIO event
         switch (pulse.value.sival_int)
         {
             case EVENT_BUTTON:
                 // Only accept if debounce threshold is met
                 if (!debounce(&last_button_event_time, DEBOUNCE_THRESHOLD_NS)) break;
 
                 printf("Button Pressed\n");
                 break;
 
             case EVENT_ROTARY_CLK:
             case EVENT_ROTARY_DT:
                 // Read both CLK and DT pin states
                 int clk_state = read_pin(CLK_GPIO);
                 int dt_state = read_pin(DT_GPIO);
                 if (clk_state == -1 || dt_state == -1)
                 {
                     return EXIT_FAILURE;
                 }
 
                 // Detect falling edge on CLK to determine rotation
                 if (clk_state == GPIO_LOW && last_clk_state == GPIO_HIGH)
                 {
                     // Direction depends on DT state at the time of CLK falling
                     if (dt_state == GPIO_HIGH)
                     {
                         position--;  // Counter-clockwise
                     } 
                     else
                     {
                         position++;  // Clockwise
                     }
 
                     printf("Rotated to %d\n", position);
                 }
 
                 last_clk_state = clk_state;
                 break;
         }
     }
 
     // Clean up GPIO resources
     rpi_gpio_cleanup();
     ConnectDetach(coid);
     ChannelDestroy(chid);

     return EXIT_SUCCESS;
 }