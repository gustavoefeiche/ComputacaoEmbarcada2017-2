# Projeto Final

#### O projeto final consistia na continuação do projeto3, que era criar um sistema embarcado que se comunicasse com um servidor e atuasse remotamente. Neste projeto, a placa realiza requisições REST continuamente (polling) para um servidor Flask rodando em uma instância da Amazon Web Services. Com isso, seria possível acender ou apagar alguns LEDs, simulando uma automação residencial.

## Diagrama de Blocos e Explicação
[](https://i.imgur.com/slFAhzG.png)

Acima está o diagrama de blocos do circuito. Seu funcionamento é bastante simples: um TC (TimerCounter) causa uma interrupção no firmware a cada 1 segundo; após está interrupção, uma variável global é "setada", ativando uma máquina de estados. Esta máquina realiza requisições para o servidor (54.175.192.2:5000) continuamente, tentando resgatar o estado de cada LED. Por meio de uma interface Web, o usuário consegue mudar o estado de cada LED, atuando remotamente no sistema. Ao mesmo tempo, um LDR está conectado ao circuito detectando a luminosidade do ambiente. Em software, um valor analógico é produzido pelo LDR, que é convertido, sabendo que 4095 = 3.3V. Um limite máximo de 3500 (mais claro) é utilizado para que o LED apague.
A leitura do LDR é realizada pelo periférico AFEC (Analog Front-End Controller), capaz de ler diversos canais analógicos.

## Utilização
Para utilizar o sistema, o usuário deve configurar no arquivo ```main.h``` o SSID (nome) da rede Wi-Fi que deseja se conectar e a senha desta rede. O IP colocado já está online, porém fica a critério do usuário escolher outro endereço para o servidor caso possua o seu próprio. É necessário também escolher a porta que deseja conectar.
Em seguida, basta ligar a placa ao computador e compilar o projeto. O sistema começará a funcionar imediatamente.
