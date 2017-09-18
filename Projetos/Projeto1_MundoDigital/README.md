# Alarme
[Trello](https://trello.com/b/8pkMvgrf/computa%C3%A7%C3%A3o-embarcada-projeto-1)
## Descrição

Um sensor de presença ativará uma contagem de tempo de 10 segundos. Caso a contagem chegue à 0, um buzzer atuará como um alarme. Para que o alarme seja desarmado é necessário que o usuário coloque uma senha. Um LED será utilizado para feedback ao usuário.

## Diagrama de Blocos

![Diagrama de Blocos](https://raw.githubusercontent.com/gustavoefeiche/ComputacaoEmbarcada2017-2/master/Projetos/Projeto1_MundoDigital/Diagram.png)

## Descrição do Projeto
Utilizando o embarcado SAME70 da Atmel, um TimerCounter será utilizado para causar uma interrupção no sistema a cada 250ms, em que será realizada uma checagem do sensor de presença, validando um movimento no local. Se o sensor detectar presença, ativará uma contagem de 10 segundos, utilizando o mesmo TimerCounter. Caso a contagem chegue aos 10 segundos, um buzzer será ativado, atuando como alarme.

### Periféricos
#### Sensor de Presença:
Possui três pinos, VCC, OUT e GND. VCC e GND são utilizados para conexão do sensor à energia, enquanto o pino OUT causa uma saída digital de nível alto se uma presença for detectada. O sensor possui ainda dois parafusos que podem ser utilizados para diminuir ou aumentar a distância verificada (de 3 a 7 metros) e diminuir ou aumentar o delay entre cada leitura (de 5 a 200 segundos). Caso nenhuma presença seja detectada, a saída será digital de nível baixo.

#### Buzzer:
Com funcionamento bastante simples, emite um som se conectado à energia. Neste sistema, o buzzer está conectado a um transistor que, por sua vez, está conectado à um pino da placa SAME70 configurado como saída. Caso a contagem chegue a 10 segundos, o pino será configurado como nível alto, permitindo a passagem de corrente pelo transistor e ativando o alarme.

#### Botões:
O sistema possui três botões, conectados a energia de um lado e a três pinos diferentes da placa, todos configurados como entrada e com pull-up ativado. Com a ativação do pull-up, o pino está sempre em nível alto para que, quando o botão for apertado, aterre o circuito, causando uma interrupção no sistema. Esta interrupção, configurada em nível de software, detectará qual botão foi apertado (qual está em nível baixo). O periférico do microprocessador, NVIC, é responsável pelo controle das interrupções.

#### LED:
O LED servirá para feedback ao usuário, acendendo se o alarme estiver ativo e apagando se o alarme estiver desativado. Para tanto, deve-se colocar nível alto em um pino configurado como saída para ligar o LED, e baixo para desligar.

## Lista de Materiais
  - 3x Push Button
  - LED verde
  - Sensor de Presença
  - Buzzer
