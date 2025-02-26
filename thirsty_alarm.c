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
#define buzzer 21

//definição número de leds na matriz
#define pixels_matriz 25

//variáveis de situação de cada uma das opções
bool estado_op1 = true;
bool estado_op2 = false;
bool estado_op3 = false;
//variável para determinar o fim da parte de cadastramento da planta
bool fim_cadastro = false;
//variável para achar tipo de planta
int tipo;

//estrutura para ilustrações na matriz de led
typedef struct {
    double frames[25][pixels_matriz];
    int num_frames;
    double r, g, b;
    int fps;
    int identifier;
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

Ilustracao welcome = {
    .frames = {
        //1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
        {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    },
    .num_frames = 25,
    .r = 0.0,
    .g = 1.0,
    .b = 0.2,
    .fps = 4,
    .identifier = 0
};

Ilustracao sun_op1 = {
    .frames = {
        //1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
        {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0}
    },
    .num_frames = 2,
    .r = 1.0,
    .g = 0.5,
    .b = 0.0,
    .fps = 3,
    .identifier = 1
};

Ilustracao rain_min_op2 = {
    .frames = {
        //1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    },
    .num_frames = 5,
    .r = 0.2,
    .g = 0.0,
    .b = 1.0,
    .fps = 5,
    .identifier = 2
};

Ilustracao rain_max_op3 = {
    .frames = {
        //1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
        {0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0},
        {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0},
        {1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0},
        {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0}
    },
    .num_frames = 5,
    .r = 0.2,
    .g = 0.0,
    .b = 1.0,
    .fps = 5,
    .identifier = 3
};

Ilustracao humidity_alarm = {
    .frames = {
        //1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
        {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
    },
    .num_frames = 3,
    .r = 1.0,
    .g = 0.0,
    .b = 0.0,
    .fps = 6,
    .identifier = 4
};

Ilustracao zerar_matriz = {
    .frames = {
        //1    2    3    4    5    6    7    8    9   10   11   12   13   14   15   16   17   18   19   20   21   22   23   24   25
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    },
    .num_frames = 1,
    .r = 0.0,
    .g = 0.0,
    .b = 0.0,
    .fps = 1,
    .identifier = 5  
};

void executar_ilustracao(PIO pio, uint sm, Ilustracao *ilustracao) {
    int frame_delay = 1000 / ilustracao->fps; // Calcula o tempo entre frames em milissegundos
    for (int frame = 0; frame < ilustracao->num_frames; frame++) {
        for (int i = 0; i < pixels_matriz; i++) {
            double intensidade = ilustracao->frames[frame][i];
            uint32_t valor_led = cores_matriz(ilustracao->b * intensidade, ilustracao->r * intensidade, ilustracao->g * intensidade);
            pio_sm_put_blocking(pio, sm, valor_led);
        }
        sleep_ms(frame_delay); // Usa o tempo calculado com base no FPS
    }
}

void executar_inicio(PIO pio, uint sm, ssd1306_t ssd){
    ssd1306_draw_string(&ssd, "WELCOME", 37, 31);
    ssd1306_send_data(&ssd);
    executar_ilustracao(pio, sm, &welcome);
    sleep_ms(1000);
}

void exibir_op1(PIO pio, uint sm, ssd1306_t ssd, bool estado){
    if(estado == true){
        ssd1306_draw_string(&ssd, "SELECT A PLANT", 7, 1);
        ssd1306_line(&ssd, 0, 10, 127, 10, true);
        ssd1306_line(&ssd, 0, 53, 127, 53, true);
        ssd1306_draw_string(&ssd, "PICK A  NEXT B", 7, 55);
        ssd1306_line(&ssd, 40, 59, 44, 59, true);
        ssd1306_line(&ssd, 104, 59, 108, 59, true);
        ssd1306_draw_string(&ssd, "SUCCULENT", 27, 20);
        ssd1306_draw_string(&ssd, "OR", 55, 29);
        ssd1306_draw_string(&ssd, "CACTUS", 41, 38);
        ssd1306_send_data(&ssd);
        executar_ilustracao(pio, sm, &sun_op1);
    } else return;
}

void exibir_op2(PIO pio, uint sm, ssd1306_t ssd, bool estado){
    if(estado == true){
        ssd1306_draw_string(&ssd, "SELECT A PLANT", 7, 1);
        ssd1306_line(&ssd, 0, 10, 127, 10, true);
        ssd1306_line(&ssd, 0, 53, 127, 53, true);
        ssd1306_draw_string(&ssd, "PICK A  NEXT B", 7, 55);
        ssd1306_line(&ssd, 40, 59, 44, 59, true);
        ssd1306_line(&ssd, 104, 59, 108, 59, true);
        ssd1306_draw_string(&ssd, "EVERGREEN", 25, 26);
        ssd1306_send_data(&ssd);
        executar_ilustracao(pio, sm, &rain_min_op2);
    } else return;
}

void exibir_op3(PIO pio, uint sm, ssd1306_t ssd, bool estado){
    if(estado == true){
        ssd1306_draw_string(&ssd, "SELECT A PLANT", 7, 1);
        ssd1306_line(&ssd, 0, 10, 127, 10, true);
        ssd1306_line(&ssd, 0, 53, 127, 53, true);
        ssd1306_draw_string(&ssd, "PICK A  NEXT B", 7, 55);
        ssd1306_line(&ssd, 40, 59, 44, 59, true);
        ssd1306_line(&ssd, 104, 59, 108, 59, true);
        ssd1306_draw_string(&ssd, "TROPICAL", 27, 26);
        ssd1306_send_data(&ssd);
        executar_ilustracao(pio, sm, &rain_max_op3);
    } else return;
}

int get_planta(Ilustracao *ilustracao){
    return ilustracao->identifier;
}

void executar_escolhas(PIO pio, uint sm, ssd1306_t ssd){
    ssd1306_draw_string(&ssd, "SELECT A PLANT", 7, 1);
    ssd1306_line(&ssd, 0, 10, 127, 10, true);
    ssd1306_line(&ssd, 0, 53, 127, 53, true);
    ssd1306_draw_string(&ssd, "PICK A  NEXT B", 7, 55);
    ssd1306_line(&ssd, 40, 59, 44, 59, true);
    ssd1306_line(&ssd, 104, 59, 108, 59, true);
    exibir_op1(pio, sm, ssd, estado_op1);
    exibir_op2(pio, sm, ssd, estado_op2);
    exibir_op3(pio, sm, ssd, estado_op3);
    ssd1306_send_data(&ssd);
}

void gpio_irq_handler(uint gpio, uint32_t eventos){
    if(gpio == button_b){
        if (estado_op1 == true && estado_op2 == false && estado_op3 == false){
            estado_op2 = true;
            estado_op1 = false;
            estado_op3 = false;
        } else if (estado_op1 == false && estado_op2 == true && estado_op3 == false){
            estado_op3 = true;
            estado_op1 = false;
            estado_op2 = false;
        } else if (estado_op1 == false && estado_op2 == false && estado_op3 == true){
            estado_op1 = true;
            estado_op2 = false;
            estado_op3 = false;
        }
  
    } 
    
    else if (gpio == button_a){
        if (estado_op1 == true){
            tipo = get_planta(&sun_op1);
            fim_cadastro = true;
        } else if (estado_op2 == true){
            tipo = get_planta(&rain_min_op2);
            fim_cadastro = true;
        } else if (estado_op3 == true){
            tipo = get_planta(&rain_max_op3);
            fim_cadastro = true;
        }
    }
}

void buzzer_alarm(uint frequency, bool on_or_off) {
    uint32_t period_us = 1000000 / frequency; 
    uint32_t half_period_us = period_us / 2;  
    if(on_or_off == true){
        gpio_put(buzzer, 1);
        sleep_us(half_period_us);
        gpio_put(buzzer, 0);
        sleep_us(half_period_us);
    } else gpio_put(buzzer, 0);
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
    //inicializa buzzer
    gpio_init(buzzer);
    gpio_set_dir(buzzer, true);
    gpio_put(buzzer, false);

    ssd1306_t ssd; 
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, i2c_port);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    //inicio do programa
    executar_inicio(pio, sm, ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    gpio_set_irq_enabled_with_callback(button_b, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(button_a, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    
    while (fim_cadastro == false){
        executar_escolhas(pio, sm, ssd);
        ssd1306_fill(&ssd, false);
        ssd1306_send_data(&ssd);
    }
    
    while (true) {
        int humidity_value = adc_read() / 40.95;
        char str_x[3];
        sprintf(str_x, "%d", humidity_value);
        ssd1306_fill(&ssd, false);
        ssd1306_send_data(&ssd);
        if(tipo == 1){
            if(humidity_value < 40){
                ssd1306_draw_string(&ssd, "LOW HUMIDITY", 17, 22);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                executar_ilustracao(pio, sm, &humidity_alarm);
                gpio_put(led_red, true);
                buzzer_alarm(4000, true);
            } else if (humidity_value > 50){
                ssd1306_draw_string(&ssd, "HIGH HUMIDITY", 13, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                executar_ilustracao(pio, sm, &humidity_alarm);
                gpio_put(led_red, true);
                buzzer_alarm(4000, true);
            } else {
                ssd1306_draw_string(&ssd, "GOOD HUMIDITY", 13, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                gpio_put(led_red, false);
                buzzer_alarm(4000, false);
                executar_ilustracao(pio, sm, &zerar_matriz);
            }
        
        } else if(tipo == 2){
            if(humidity_value < 60){
                ssd1306_draw_string(&ssd, "LOW HUMIDITY", 17, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                executar_ilustracao(pio, sm, &humidity_alarm);
                gpio_put(led_red, true);
                buzzer_alarm(4000, true);
            } else if (humidity_value > 70){
                ssd1306_draw_string(&ssd, "HIGH HUMIDITY", 13, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                executar_ilustracao(pio, sm, &humidity_alarm);
                gpio_put(led_red, true);
                buzzer_alarm(4000, true);
            } else {
                ssd1306_draw_string(&ssd, "GOOD HUMIDITY", 13, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                gpio_put(led_red, false);
                buzzer_alarm(4000, false);
                executar_ilustracao(pio, sm, &zerar_matriz);
            }
        
        } else if(tipo == 3){
            if(humidity_value < 60){
                ssd1306_draw_string(&ssd, "LOW HUMIDITY", 17, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                executar_ilustracao(pio, sm, &humidity_alarm);
                gpio_put(led_red, true);
                buzzer_alarm(4000, true);
            } else if (humidity_value > 80){
                ssd1306_draw_string(&ssd, "HIGH HUMIDITY", 13, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                executar_ilustracao(pio, sm, &humidity_alarm);
                gpio_put(led_red, true);
                buzzer_alarm(4000, true);
            } else {
                ssd1306_draw_string(&ssd, "GOOD HUMIDITY", 13, 26);
                ssd1306_draw_string(&ssd, str_x, 19, 35);
                ssd1306_draw_string(&ssd, "PERCENT", 44, 35);
                ssd1306_send_data(&ssd);
                gpio_put(led_red, false);
                buzzer_alarm(4000, false);
                executar_ilustracao(pio, sm, &zerar_matriz);
            }
        }
    }
}
