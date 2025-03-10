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

// adapted from the rpi_ws281x library (https://github.com/jgarff/rpi_ws281x) 

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/neutrino.h>
#include "rpi_ws281x.h"

// LED matrix characteristics (modify to match your LED matrix size)
#define GPIO_PIN                LED_CHANNEL_0_DATA_PIN // pin 10
#define STRIP_TYPE              WS2811_STRIP_GBR		// WS2812/SK6812RGB integrated chip+leds

#define WIDTH                   32
#define HEIGHT                  8
#define LED_COUNT               (WIDTH * HEIGHT)

#define ARRAY_SIZE(stuff)       (sizeof(stuff) / sizeof(stuff[0]))

int width = WIDTH;
int height = HEIGHT;
int led_count = LED_COUNT;

bool clear_on_exit = true;
bool running = true;

ws2811_t ledstring =
{
    .freq = WS2811_TARGET_FREQ,
    .channel =
    {
        [0] =
        {
            .gpionum = GPIO_PIN,
            .invert = 0,
            .count = LED_COUNT,
            .strip_type = STRIP_TYPE,
            .brightness = 255,
        },
        // no second channel
        [1] =
        {
            .gpionum = -1,
            .invert = 0,
            .count = 0,
            .brightness = 0,
        },
    },
};

ws2811_led_t *matrix;

void matrix_render(void)
{
    int x, y;

    for (x = 0; x < width; x++)
    {
        for (y = 0; y < height; y++)
        {
            // this is for LED strip increments down from top left before advancing to next column and reversing direction
            ledstring.channel[0].leds[(x * height) + (((x&1) == 1)?(height-1-y):y)] = matrix[y * width + x];
        }
    }
}

void matrix_raise(void)
{
    int x, y;

    for (y = 0; y < (height - 1); y++)
    {
        for (x = 0; x < width; x++)
        {
            matrix[y * width + x] = matrix[(y + 1)*width + x];
        }
    }
}

void matrix_clear(void)
{
    int x, y;

    for (y = 0; y < (height ); y++)
    {
        for (x = 0; x < width; x++)
        {
            matrix[y * width + x] = 0;
        }
    }
}

int dotspos[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
ws2811_led_t dotcolors[] =
{
    0x00200000,  // red
    0x00201000,  // orange
    0x00202000,  // yellow
    0x00002000,  // green
    0x00002020,  // lightblue
    0x00000020,  // blue
    0x00100010,  // purple
    0x00200010,  // pink
};

ws2811_led_t dotcolors_rgbw[] =
{
    0x00200000,  // red
    0x10200000,  // red + W
    0x00002000,  // green
    0x10002000,  // green + W
    0x00000020,  // blue
    0x10000020,  // blue + W
    0x00101010,  // white
    0x10101010,  // white + W

};

void matrix_bottom(void)
{
    int i;

    for (i = 0; i < (int)(ARRAY_SIZE(dotspos)); i++)
    {
        dotspos[i]++;
        if (dotspos[i] > (width - 1))
        {
            dotspos[i] = 0;
        }

        if (ledstring.channel[0].strip_type == SK6812_STRIP_RGBW) {
            matrix[dotspos[i] + (height - 1) * width] = dotcolors_rgbw[i];
        } else {
            matrix[dotspos[i] + (height - 1) * width] = dotcolors[i];
        }
    }
}

static void ctrl_c_handler(int signum)
{
    (void)(signum);
    running = 0;
}

static void setup_handlers(void)
{
    struct sigaction sa =
    {
        .sa_handler = ctrl_c_handler,
    };

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

int main(int argc, char *argv[])
{
    ws2811_return_t ret;

    matrix = malloc(sizeof(ws2811_led_t) * width * height);
    matrix_clear();

    setup_handlers();

    if ((ret = ws2811_init(&ledstring)) != WS2811_SUCCESS)
    {
        fprintf(stderr, "ws2811_init failed: %s\n", ws2811_get_return_t_str(ret));
        return ret;
    }

    while (running)
    {
        matrix_raise();
        matrix_bottom();
        matrix_render();

       if ((ret = ws2811_render(&ledstring)) != WS2811_SUCCESS)
       {
           fprintf(stderr, "ws2811_render failed: %s\n", ws2811_get_return_t_str(ret));
           break;
        }

        // 30 frames /sec
        usleep(1000000 / 30);
    }

    if (clear_on_exit) {
	    matrix_clear();
	    matrix_render();
	    ws2811_render(&ledstring);
    }

    ws2811_fini(&ledstring);

    printf ("Exiting ...\n");

    return ret;
}