#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
//arquivo .pio
#include "thirsty_alarm.pio.h"

//definições de gpio
#define humidity_sensor 26
#define matriz_led 7
#define led_red 13
#define button_a 5
#define button_b 6
#define i2c_port i2c1
#define i2c_sda 14
#define i2c_scl 15
#define endereco 0x3C

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
    //inicialização do led RGD vermelho
    gpio_init(led_red);
    gpio_set_dir(led_red, true);
    gpio_put(led_red, false);
    //inicialização dos botões A e B
    gpio_init(button_a);
    gpio_set_dir(button_a, false);
    gpio_pull_up(button_a);
    gpio_init(button_b);
    gpio_set_dir(button_b, false);
    gpio_pull_up(button_b);
    //inicialização do display
    i2c_init(i2c_port, 400 * 1000);
    gpio_set_function(i2c_sda, GPIO_FUNC_I2C); 
    gpio_set_function(i2c_scl, GPIO_FUNC_I2C); 
    gpio_pull_up(i2c_sda);
    gpio_pull_up(i2c_scl);
    ssd1306_t ssd; 
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, i2c_port);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    while (true) {
        sleep_ms(1000);
    }
}
