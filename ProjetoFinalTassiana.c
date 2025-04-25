#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "hardware/adc.h"

//pinos e módulos controlador i2c selecionado
#define I2C_PORT i2c1
#define PINO_SCL 14
#define PINO_SDA 15

#define VRX_PIN 27    // Define o pino GP27 para o eixo X do joystick (Canal ADC1).
#define VRY_PIN 26    // Define o pino GP26 para o eixo Y do joystick (Canal ADC0).

#define LED_PIN_A 11      // Pino GPIO usado para o LED verde
#define LED_PIN_B 13      // Pino GPIO usado para o LED vermelho     

// Define os pinos dos botões
#define BUTTON_A_PIN 5  // Pino GPIO usado para o botão A.
#define BUTTON_B_PIN 6  // Pino GPIO usado para o botão B.

//Variáveis com os valores iniciais das grandezas monitoradas.
float (temperatura)= 30; //Variável com valor inicial para a temperatura.
float (umidade) = 74; //Variável com valor inicial para a umidade.
float (nivelRacao) = 100; //Variável com valor inicial para o nível ração.
 
ssd1306_t disp;

// Função de callback que será chamada a cada intervalo definido pelo temporizador.
// Esta função alterna o estado do LED e imprime uma mensagem na saída serial.
bool repeating_timer_callback(struct repeating_timer *t) {
    
    // Imprime na saída serial o valor atual de temperatura e umidade
    printf("Temp.: %.2f, Umid.: %.2f, Racao: %.2f\n", temperatura, umidade, nivelRacao);
    
    return true;
}

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

    // O pino GP26 está mapeado para o canal 0 do ADC, que será usado para ler o eixo X do joystick.
    adc_gpio_init(VRX_PIN);
    // O pino GP27 está mapeado para o canal 1 do ADC, que será usado para ler o eixo Y do joystick.
    adc_gpio_init(VRY_PIN);  
    
    // Configuração do botão A
    gpio_init(BUTTON_A_PIN); // Inicializa o pino do botão A
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_A_PIN); // Habilita o resistor de pull-up interno

    // Configuração do botão B
    gpio_init(BUTTON_B_PIN); // Inicializa o pino do botão B
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_B_PIN); // Habilita o resistor de pull-up interno   

    // Configuração do LED
    gpio_init(LED_PIN_A); // Inicializa o pino do LED
    gpio_set_dir(LED_PIN_A, GPIO_OUT); // Configura o pino como saída 

    gpio_init(LED_PIN_B); // Inicializa o pino do LED
    gpio_set_dir(LED_PIN_B, GPIO_OUT); // Configura o pino como saída      
}

//função escrita no display.
void print_texto(char *msg, uint pos_x, uint pos_y, uint scale){
    ssd1306_draw_string(&disp, pos_x, pos_y, scale, msg);//desenha texto
    ssd1306_show(&disp);//apresenta no Oled
}

void atualizaDisplay(float temperatura, float umidade, float nivelRacao) {
    char text[20]; // Buffer para armazenar o texto convertido
    ssd1306_clear(&disp);    // Limpa a tela

    // Converte e imprime os valores no display
    sprintf(text, " Temp.: %.1f", temperatura);
    print_texto(text, 6, 10, 1.5);

    sprintf(text, " Umid.: %.1f", umidade);
    print_texto(text, 6, 30, 1.5);

    sprintf(text, " Racao: %.1f", nivelRacao);
    print_texto(text, 6, 50, 1.5);    
}



int main()
{
    inicializa(); 
    atualizaDisplay(temperatura, umidade, nivelRacao);    
    
    // Declara uma estrutura para armazenar informações sobre o temporizador repetitivo.
    struct repeating_timer timer;
    // Configura um temporizador repetitivo que chama a função 'repeating_timer_callback' a cada 3 segundo (5000 ms).
    // Parâmetros: 3000: Intervalo de tempo em milissegundos (5 segundos). repeating_timer_callback: Função de callback que será chamada a cada intervalo.
    // Parâmetros: NULL: Dados adicionais que podem ser passados para a função de callback (não utilizado aqui).
    // Parâmetros: &timer: Ponteiro para a estrutura que armazenará informações sobre o temporizador.
    add_repeating_timer_ms(3000, repeating_timer_callback, NULL, &timer);

    while(true){
        
        // Esse canal corresponde ao eixo X do joystick (VRX).
        adc_select_input(1);
        uint16_t vrx_value = adc_read(); // Lê o valor do eixo X, de 0 a 4095.        

        // Esse canal corresponde ao eixo Y do joystick (VRY).
        adc_select_input(0);
        uint16_t vry_value = adc_read(); // Lê o valor do eixo Y, de 0 a 4095. 

        // Lê o estado do botão A
        // O valor lido será 0 se o botão estiver pressionado e 1 se não estiver.
        bool button_A_value = gpio_get(BUTTON_A_PIN) == 0; // 0 indica que o botão está pressionado.
        // Lê o estado do botão B
        // O valor lido será 0 se o botão estiver pressionado e 1 se não estiver.
        bool button_B_value = gpio_get(BUTTON_B_PIN) == 0; // 0 indica que o botão está pressionado.


        if(vrx_value>2800){
            temperatura += 5;            
            atualizaDisplay(temperatura, umidade, nivelRacao);
        }
        else if(vrx_value<1200){
            temperatura -= 5;            
            atualizaDisplay(temperatura, umidade, nivelRacao);
        }
        else if(vry_value>2800){
            umidade += 5;                      
            atualizaDisplay(temperatura, umidade, nivelRacao);
        }
        else if(vry_value<1200){
            umidade -= 5;            
            atualizaDisplay(temperatura, umidade, nivelRacao);              
        }        
        else if (button_A_value) { // Verifica se o botão A foi pressionado
            nivelRacao -= 10;
            if(nivelRacao < 0){
                nivelRacao = 0; // Define um limite mínimo para a ração em 0kg.
            }
            atualizaDisplay(temperatura, umidade, nivelRacao);                             
        }
        else if (button_B_value) { // Verifica se o botão B foi pressionado
            nivelRacao = 100;
            atualizaDisplay(temperatura, umidade, nivelRacao);                                      
        }

        if(umidade > 100){
            umidade = 100; // Define um limite máximo para a umidade em 100%
            atualizaDisplay(temperatura, umidade, nivelRacao);
        } 
        else if(umidade < 0){
            umidade = 0; // Define um limite mínimo para a umidade em 0%
            atualizaDisplay(temperatura, umidade, nivelRacao);
        }    
       
        if(temperatura > 35){                        
            printf("Irrigando...\n");
            temperatura -=0.3;                    
            atualizaDisplay(temperatura, umidade, nivelRacao);
        }

        if(umidade < 50){                        
            printf("Irrigando...\n");
            umidade +=0.3;         
            atualizaDisplay(temperatura, umidade, nivelRacao);
        }

        if(nivelRacao >= 40){                        
            gpio_put(LED_PIN_A, 1); // LED verde aceso            
            gpio_put(LED_PIN_B, 0);
        }
        else if (nivelRacao <= 20){
            gpio_put(LED_PIN_A, 0);
            gpio_put(LED_PIN_B, 1); // LED vermelho aceso
        }
        else {
            //Forma a cor amarela no led.
            gpio_put(LED_PIN_A, 1); // LED verde aceso
            gpio_put(LED_PIN_B, 1); // LED vermelho aceso 
        }    
            
       sleep_ms(250);
    }
    return 0;
}

