#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "hardware/adc.h"


//Incluindo programas das opções do menu
#include "joystick.c"
#include "buzzer_pwm1.c"
#include "PWM_LED_0.c"

//pinos e módulos controlador i2c selecionado
#define I2C_PORT i2c1
#define PINO_SCL 14
#define PINO_SDA 15

//botão do Joystick
const int SW = 22;  

//definição dos LEDs RGB
const uint BLUE_LED_PIN= 12;   // LED azul no GPIO 12
const uint RED_LED_PIN  = 13; // LED vermelho no GPIO 13
const uint GREEN_LED_PIN = 11;  // LED verde no GPIO 11

//variável para armazenar a posição do seletor do display
uint pos_y=12;

ssd1306_t disp;

uint countdown = 0; //verificar seleções para baixo do joystick
uint countup = 2; //verificar seleções para cima do joystick


//função para inicialização de todos os recursos do sistema
void inicializa(){
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    i2c_init(I2C_PORT, 400*1000);// I2C Inicialização. Usando 400Khz.
    gpio_set_function(PINO_SCL, GPIO_FUNC_I2C);
    gpio_set_function(PINO_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(PINO_SCL);
    gpio_pull_up(PINO_SDA);
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, I2C_PORT);

    //botão do Joystick
    gpio_init(SW);             // Inicializa o pino do botão
    gpio_set_dir(SW, GPIO_IN); // Configura o pino do botão como entrada
    gpio_pull_up(SW);
    
}

//função escrita no display.
void print_texto(char *msg, uint pos_x, uint pos_y, uint scale){
    ssd1306_draw_string(&disp, pos_x, pos_y, scale, msg);//desenha texto
    ssd1306_show(&disp);//apresenta no Oled
}

//o desenho do retangulo fará o papel de seletor
void print_retangulo(int x1, int y1, int x2, int y2){
    ssd1306_draw_empty_square(&disp,x1,y1,x2,y2);
    ssd1306_show(&disp);
}

void selecionaMenu(){
    char *text = ""; //texto do menu
    //texto do Menu
    ssd1306_clear(&disp);//Limpa a tela
    print_texto(text="Menu", 52, 2, 1);
    print_retangulo(2,pos_y,120,18);
    print_texto(text=" CODE 1 - JOYSTICK", 6, 18, 1.5);
    print_texto(text=" CODE 2 - BUZZER ", 6, 30, 1.5);
    print_texto(text=" CODE 3 - LED PWM", 6, 42, 1.5);
}


void joystick()
{
  uint16_t vrx_value, vry_value, sw_value; // Variáveis para armazenar os valores do joystick (eixos X e Y) e botão
  setup();                                 // Chama a função de configuração
  printf("Joystick-PWM\n");                // Exibe uma mensagem inicial via porta serial
  // Loop principal
  while (1)
  {
    joystick_read_axis(&vrx_value, &vry_value); // Lê os valores dos eixos do joystick
    // Ajusta os níveis PWM dos LEDs de acordo com os valores do joystick
    pwm_set_gpio_level(LED_B, vrx_value); // Ajusta o brilho do LED azul com o valor do eixo X
    pwm_set_gpio_level(LED_R, vry_value); // Ajusta o brilho do LED vermelho com o valor do eixo Y

    // Pequeno delay antes da próxima leitura
    sleep_ms(100); // Espera 100 ms antes de repetir o ciclo
  }
}

void buzzer() {
    stdio_init_all();
    pwm_init_buzzer(BUZZER_PIN);
    while(1){
      play_star_wars(BUZZER_PIN);     
    } 
}

void ledPWM()
{
    uint up_down = 1; // Variável para controlar se o nível do LED aumenta ou diminui
    stdio_init_all(); // Inicializa o sistema padrão de I/O
    setup_pwm();      // Configura o PWM
    while (true)
    {
        pwm_set_gpio_level(LED, led_level); // Define o nível atual do PWM (duty cycle)
        sleep_ms(1000);                     // Atraso de 1 segundo
        if (up_down)
        {
            led_level += LED_STEP; // Incrementa o nível do LED
            if (led_level >= PERIOD)
                up_down = 0; // Muda direção para diminuir quando atingir o período máximo
        }
        else
        {
            led_level -= LED_STEP; // Decrementa o nível do LED
            if (led_level <= LED_STEP)
                up_down = 1; // Muda direção para aumentar quando atingir o mínimo
        }
    }
}

int main()
{
    inicializa();     
   
    while(true){
        //trecho de código aproveitado de https://github.com/BitDogLab/BitDogLab-C/blob/main/joystick/joystick.c
        adc_select_input(0);
        uint adc_y_raw = adc_read();
        const uint bar_width = 40;
        const uint adc_max = (1 << 12) - 1;
        uint bar_y_pos = adc_y_raw * bar_width / adc_max; //bar_y_pos determinará se o Joystick foi pressionado para cima ou para baixo

        printf("Valor de y e: %d\n", bar_y_pos);
        //o valor de 18 é o estado de repouso do Joystick
        if(bar_y_pos < 10 && countdown <2){
            pos_y+=12;
            selecionaMenu();
            countdown+=1;
            countup-=1;            
        }else
            if(bar_y_pos > 26 && countup <2){
                pos_y-=12;
                selecionaMenu();
                countup+=1;
                countdown-=1;                 
            }
             

        //verifica se botão foi pressionado. Se sim, entra no switch case para verificar posição do seletor e chama acionamento dos leds.
        if(gpio_get(SW) == 0){            
                switch (pos_y){
                    case 12:                        
                        joystick();                                              
                    case 24:
                        buzzer();                          
                    case 36:
                        ledPWM();
                }                                             
        }         
       
       sleep_ms(250);
    }
    return 0;
}
