#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

//arquivo .pio
#include "thirsty_alarm.pio.h"

#define humidity_sensor 26

int main()
{
    stdio_init_all();
    
    //inicialização do joystick 
    adc_init();
    adc_gpio_init(humidity_sensor);  
    adc_select_input(0);

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
