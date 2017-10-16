---
title: 12 - Mundo analógico \\ Código
subtitlepage-note: Código
author: Rafael Corsi - rafael.corsi@insper.edu.br
date: Abril - 2017
---

![Roland Kirk](figs/Roland-Kirk.png){width=70%}


# Introdução

O código exemplo *12 - ADC* configura e demonstra a utilização do conversor analógico digital (AFEC)
do microcontrolador SAME-70, esse periférico pode ser utilizado para transformar valores analógicos
do mundo externo ao processador em valores digitais que podem ser processados pelo uC.

Em sua grande maioria os sensores elétricos são transdutores, ou seja, convertem convertem grandezas diversas
em tensão (sensor de luz, sensor de temperatura, gás, ...). Para podermos processar essas grandezas é necessário
primeiramente digitalizarmos esses valores.

Periféricos utilizados :
 - Power Management Controller (PMC)
 - Analog Front-End Controller (AFEC)
 - Universal Synchronous Asynchronous Receiver Transceiver (USART)

## Código Visão geral

```C

/************************************************************************/
/* Includes                                                             */
/************************************************************************/
...

/************************************************************************/
/* Defines                                                              */
/************************************************************************/
...

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

/**
 * \brief Configure UART console.
 * BaudRate : 115200
 * 8 bits
 * 1 stop bit
 * sem paridade
 */
static void configure_console(void)
{
  ...
}

/** 
 * converte valor lido do ADC para temperatura em graus celsius
 * input : ADC reg value
 * output: Temperature in celsius
 */
static int32_t convert_adc_to_temp(int32_t ADC_value)
{
  ...
}
/************************************************************************/
/* Call backs / Handler                                                 */
/************************************************************************/

/**
 * \brief AFEC interrupt callback function.
 */
static void AFEC_Temp_callback(void)
{
  ...
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/
int main(void)
{

  /* Initialize the SAM system. */
  sysclk_init();
  ioport_init();
  board_init();
  
  /* inicializa delay */
  delay_init(sysclk_get_cpu_hz());
  
  /* inicializa console (printf) */
  configure_console();
  
  /* Output example information. */
  puts(STRING_HEADER);
  
  /************************************* 
   * Ativa e configura AFEC
   *************************************/  
  ...
  ...
  
  while (1) {

    if(is_conversion_done == true) {
      ...
      ...
      afec_start_software_conversion(AFEC0);
      delay_s(1);
    }
  }
}

```

###  configure\_console(void)

A função configure\_console() inicializa o periférico **USART** para controlar os pinos (PA21, PB4) do microcontrolador,
esses pinos estão conectados no EDBG (gravador embarcado no kit) que possibilitam abrimos um terminal no computador
e recebermos/enviarmos dados via uma comunicação serial, similar como é feito no arduino.

A função configura também o **stdio** para utilizar esse periférico como saída e entrada das funções **printf**, **getc**, .... .

![edbg comunicação UART - XPLD](figs/edbg.png){width=100%}

Para acessar os printfs do microcontrolador é necessário abrir um terminal no windows, na pasta do repositório : **Softwares Extras/** possui um programa chamado **PUTTY**, ele deve ser configurado como a seguir :

![edbg comunicação UART - XPLD](figs/putty.png){width=60%}


Onde a COM port deve ser identificada em : "Gerenciador de dispositivos" :

![COM port Windows](figs/com.png){width=60%}

### static int32\_t convert\_adc\_to\_temp(int32\_t ADC\_value){}

Essa é uma função que converte o valor lido (ADC\_value) pelo conversor analógico digital e converte para temperatura
em graus célsius. Sua implementação são duas simples transformações :

1. Regra de três para converter o valor em bits para Volts
2. Equacionamento linear para transformar Volts em graus Celsius

## static void AFEC_Temp_callback(void){}

É a função que é chamada quando a conversão analógica digital é concluida. Nessa função fazemos a leitura do valor da
conversão analógica e salvamos o seu valor em uma variável global.

```C
g_ul_value = afec_channel_get_value(AFEC0, AFEC_CHANNEL_TEMP_SENSOR);
```

CallBacks são interrupções já tratadas por software, no caso do AFEC não tratamos diretamente o HANDLER mas sim callbacks que são
configuradas para cada canal do AFEC0:

```C
main(){
  ...

  /* configura call back */
  afec_set_callback(AFEC0, AFEC_INTERRUPT_EOC_11,	AFEC_Temp_callback, 1);
  ...
}
```

![CallBack](figs/handler.png){width=60%}


Na função chamada no main, fazemos a associação do canal 11 que o sensor de temperatura está conectado a função de callback **AFEC\_Temp\_callback** nesse caso não trabalhamos com o Handler direto, mas sim com chamadas de funções internas a ele. O Handler do AFEC é gerenciado pelas funções da Atmel ASF.

### AFEC

O Analog Front-End Controller (AFEC) nesse exemplo é configurado para operar lendo o sensor de temperatura interno do microcontrolador,
que está conectado na entrada 11 do MUX. 

# Desafio

## Programação

1. Analise o código exemplo e implemente a leitura de temperatura a cada 1 segundo
    - utilize o printf para exebir o valor no terminal
2. Utilize um TimerCounter para dar a taxa de amostragem correta (1 segundo)
3. Implemente para cada leitura um TimeStamp de quando ela foi executada como a seguir :

 ```
  20h10 - 17-4-2017 : Temperatura Interna 28C
 ```
4. Organize o código, crie uma função para a inicialização do AFEC 
 
