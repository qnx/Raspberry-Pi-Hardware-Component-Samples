#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/iomsg.h>

// include gpio interface funct
#include "public/rpi_gpio.h"
// define possible events types
enum sample_event_t
{
    EVENT_BUTTON_1,
};

// define GPIO pin number
#define RELAY_GPIO_PIN GPIO17
#define BUTTON_GPIO_PIN GPIO19

static int chid; // channel ID
static int coid; // connection ID

static bool init_channel(void)
{
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

static bool init_button_input(int gpio_pin, int button_event_id)
{
    if (rpi_gpio_setup_pull(gpio_pin, GPIO_IN, GPIO_PUD_DOWN))
    {
        perror("rpi_gpio_setup_pull");
        return false;
    }
    if (rpi_gpio_add_event_detect(gpio_pin, coid, GPIO_RISING | GPIO_FALLING, button_event_id))
    {
        perror("rpi_gpio_add_event_detect");
        return false;
    }
    return true;
}

static bool init_RELAY(int gpio_pin)
{
    if (rpi_gpio_setup(gpio_pin, GPIO_OUT))
    {
        perror("rpi_gpio_setup");
        return false;
    }
    return true;
}

static bool RELAY_on(int gpio_pin)
{
    if (rpi_gpio_output(gpio_pin, GPIO_HIGH))
    {
        perror("rpi_gpio_setup");
        return false;
    }
    return true;
}

static bool RELAY_off(int gpio_pin)
{
    if (rpi_gpio_output(gpio_pin, GPIO_LOW))
    {
        perror("rpi_gpio_setup");
        return false;
    }
    return true;
}

int main()
{

    // initializing the communication channel
    if (!init_channel())
    {
        return EXIT_FAILURE;
    }

    // initializing the button input
    if (!init_button_input(BUTTON_GPIO_PIN, EVENT_BUTTON_1))
    {
        return EXIT_FAILURE;
    }

    // initialize RELAY
    if (!init_RELAY(RELAY_GPIO_PIN))
    {
        return EXIT_FAILURE;
    }

    // track RELAY state to prevent unnecessary operations
    bool RELAY_is_on = false;

    // start with RELAY off
    if (!RELAY_off(RELAY_GPIO_PIN))
    {
        return EXIT_FAILURE;
    }

    // track the time of the last valid button press
    struct timespec last_button_event_time;
    last_button_event_time.tv_sec = 0;
    last_button_event_time.tv_nsec = 0;

    // Bounce threshold in nanoseconds (e.g.,100ms)
    const long debounce_threshold = 100000000;

    for (;;)
    {
        struct _pulse pulse;
        struct timespec current_time;
        if (MsgReceivePulse(chid, &pulse, sizeof(pulse), NULL) == -1)
        {
            perror("MsgReceivePulse()");
            return EXIT_FAILURE;
        }

        if (pulse.code != _PULSE_CODE_MINAVAIL)
        {
            fprintf(stderr, "Unexpected plse code %d\n", pulse.code);
            return EXIT_FAILURE;
        }

        switch (pulse.value.sival_int)
        {
        case EVENT_BUTTON_1:

            //print botton event
            printf("Button event\n");
            // get the current time
            clock_gettime(CLOCK_MONOTONIC, &current_time);

            if (!RELAY_is_on)
            {
                long time_diff_ns = (current_time.tv_sec - last_button_event_time.tv_sec) * 100000000L + (current_time.tv_nsec - last_button_event_time.tv_nsec);
                if (time_diff_ns < debounce_threshold)
                {
                    // ignore this event, as it's too close to the last one
                    break;
                }
            }
            last_button_event_time = current_time;

            if (RELAY_is_on)
            {
                // RELAY is on, turn it off
                if (!RELAY_off(RELAY_GPIO_PIN))
                {
                    return EXIT_FAILURE;
                }
                RELAY_is_on = false;
            }
            else
            {
                // RELAY is off, turn it on
                if (!RELAY_on(RELAY_GPIO_PIN))
                {
                    return EXIT_FAILURE;
                }
                RELAY_is_on = true;
            }
            break;
        }
    }
    return EXIT_SUCCESS;
}