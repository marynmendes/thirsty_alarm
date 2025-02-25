#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
//arquivo .pio
#include "thirsty_alarm.pio.h"

//definições de gpio
#define humidity_sensor 26
#define matriz_led 7

//definição número de leds na matriz
#define pixels_matriz 25

//estrutura para ilustrações na matriz de led
typedef struct {
    double frames[5][pixels_matriz];
    int num_frames;
    double r, g, b;
    int fps;
} Ilustracao;

//configuração de cores 
uint32_t cores_matriz(double b, double r, double g) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

//inicialização do PIO para a matriz
void init_matriz_led(PIO pio, uint *offset, uint *sm) {
    *offset = pio_add_program(pio, &thirsty_alarm_program);
    if (*offset < 0) {
        printf("Erro ao carregar programa PIO.\n");
        return;
    }

    *sm = pio_claim_unused_sm(pio, true);
    if (*sm < 0) {
        printf("Erro ao requisitar state machine.\n");
        return;
    }

    thirsty_alarm_program_init(pio, *sm, *offset, matriz_led);
}

int main()
{
    PIO pio = pio0;
    uint offset, sm;

    stdio_init_all();
    
    //inicialização do joystick 
    adc_init();
    adc_gpio_init(humidity_sensor);  
    adc_select_input(0);
    //inicializa matriz de led
    init_matriz_led(pio, &offset, &sm);

    while (true) {
        sleep_ms(1000);
    }
}
