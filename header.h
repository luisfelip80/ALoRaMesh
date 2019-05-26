#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "heltec.h"
#include "Arduino.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcd.h"
#include "stack.h"

//TaskHandle_t Task1;

String versao = "0.0.6";

//IP do gateway
#define ip_gateway 0x00

//IP do ip_broadcast
#define ip_broadcast 0xFF

// IP do dispositivo
#define ip_this_node 0x00


#define tam_msg 20

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define linhas 15
#define colunas 3
#define max 9999999

#define ID_NEW_NODE             1
#define ID_CALLBACK             2
#define ID_LOST_CONNECTION      3
#define ID_REPETIDOR_ISOLADO    4
#define ID_REPETIDOR_LOOP       5
#define ID_RESPOSTA             6
#define ID_JA_TA_AQUI_A_MSG     7
#define ID_SEGUNDA_CHANCE       8
#define ID_I_WAKE_UP            9
#define ID_REPLY_CALL           10
#define ID_TALK                 11
#define ID_WAIT                 12
#define ID_REPLY_ASK            13

#define proximoEnvio			18000 


// tabela de vizinhos
int tabela [linhas][colunas];
byte repetidores[linhas] [2];
byte nr_repetidores = 0;
byte buffer[10][2];

bool t;
byte vez = 0;
//variáveis do tipo inteiro
int nr_vizinhos = 0;
byte nr_vizinhos_on = 0;
byte estado =0;
int custo;
int contador;
int contador_linhas;
int sinal;
int id;
int tentativas_reenvio;
int no_repetidor;
bool isol = false;
bool task2_usando = false;
bool task1_usando = false;
byte estate = 0;
bool wifi = false;
bool bluetooth = false;
bool radio_lora = false;
bool repetidor_bool = false;
bool freesend=false;
byte confir[15][3];
bool espera = false;
bool tenhoViz = false;
volatile bool IRS_use = false;

int  buffer_conter=0;
bool *vetorFila;
int nVetor = 15, recCont = 0;
String doing;
//variáveis do tipo string

String package;
byte ip_anterior;
byte ip_vizinho;
int ip_repetidor = -1;
byte ip_origem;
byte ip_filho;

bool my_turn_to_talk = false;

byte tam_fila = 0;

//Flags de envio e recebimento de pacotes
bool sendBlock = false;
bool ack_recebido = true;

byte gId;
int gOrigem;
byte gAnterior;
byte gDestino;
String gMsg;

int ant_repetidor;

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
long lastTime=0;
int interval = 2000;          // interval between sends
long lastCheck = 0;
long esperaTime = 20000;
long timeDraw ;
long HEAR_TIME;
long TIME_FOR_I_TALK = -180000;

// Para configurar seções críticas (interrupções de ativação e interrupções de desativação não disponíveis)
// usado para desabilitar e interromper interrupções
//portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
void sendMsg(byte id, int origem, byte anterior, byte destino, String msg){
   Serial.println("mandei :" + String(id));
    while(IRS_use);
    //String msg = String(pacote);
    //portENTER_CRITICAL_ISR(&mux);  //início da seção crítica
    LoRa.beginPacket();                   // start packet
    LoRa.write(id);              // ID de identificação
    LoRa.write(origem);                // Destino
    LoRa.write(anterior);                       // ip_this_node
    LoRa.write(destino);
    LoRa.write(msg.length());
    LoRa.print(msg);                 // add payload
    LoRa.endPacket();
    LoRa.receive();
    //portEXIT_CRITICAL_ISR(&mux);
}
void menorCusto(int origem,int anterior){
    int i ;
    String msg = "dados";
    doing = "definir repetidor";

    ant_repetidor = ip_repetidor;
    
    for(i = 0; i < linhas; i++){
        //verificando em qual linha está o repetidor para marca-lo
        if((tabela [i] [1] == origem && tabela [i] [0] != 2) || (tabela [i] [1] == anterior && tabela [i] [0] != 2)){
            tabela[i][0] = 1;
        }
        if(tabela[i][0] != 2 && tabela[i][0] != 1){
            no_repetidor = tabela[i][2];
            ip_repetidor = tabela[i][1];
        }
    }
    for(i = 0; i < linhas; i++){
        //verificando o outro nó de menor custo
        if(ip_gateway == tabela[i][1]){
            no_repetidor = tabela [i] [2];
            ip_repetidor = tabela [i] [1];
            i = linhas;
        }
        else if(no_repetidor > tabela[i][2] && tabela[i][0] != 1 && tabela[i][0] != 2){
            no_repetidor = tabela [i] [2];
            ip_repetidor = tabela [i] [1];
        }
    }
    for(i = 0; i < linhas; i++){
        //verificando em qual linha está o repetidor marcado para reseta-lo.
        if(tabela [i] [0] == 1){
            tabela [i] [0] = 0;
        }
    }
    if(ip_repetidor != ant_repetidor && ip_repetidor != -1 && ip_this_node != ip_gateway){
        tentativas_reenvio++;
        sendMsg( ID_REPLY_CALL , ip_this_node , ip_this_node , ip_repetidor ,msg);
    }
}
void verificaNos(int origem,int anterior, int custo){
    bool  h = false;
    int i;
    String msg = "dados";
    if(nr_vizinhos < 15){
        for(i = 0; i< linhas; i ++){
            //verificando se é um nó novo na rede
            if(origem == tabela[i][1]){
                //não é novo
                h=true;
                if(custo != tabela [i] [2]){
                    tabela [i] [2] = custo;
                }
                i=linhas;
            }
        }
        if(!h){
            doing = "Novo no.";
            if(isol){
                sendMsg(ID_SEGUNDA_CHANCE,ip_this_node,ip_this_node,ip_broadcast,msg);
                isol = false;
            }
            for(i = 0; i< linhas; i ++){
                if(tabela[i][1] == -1){
                    tabela [i] [1] = origem;
                    tabela [i] [2] = custo;
                    i=linhas;
                    nr_vizinhos++;
                }
            }
            menorCusto(origem,anterior);
        }
        else{
            doing= "no ja reg.";
        }   
    }
}
void sendMessage(node *list){
    int i,marc=0;
    seta = lastOne(list);
    String msg = "dados"; 
    // tem mais nós além do repetidor e anterior.
    // proximo teste é seguro.
    menorCusto(seta->next->orig,seta->next->ant);
    if(ip_repetidor == -1 ){
        doing = "Repetidor isolado";
        if(ip_this_node != seta->next->ant){
            isol = true;
            for(i=0; i < 2; i++){
                sendMsg(ID_REPETIDOR_ISOLADO, seta->next->orig, ip_this_node, seta->next->ant, msg);
            }
        }        
        seta = lastOne(list);
        vetorFila[seta->next->orig] = false;
      	delList(seta);
      	tam_fila--;
        return;
    }
    tentativas_reenvio ++;
    contador++;
    if(seta->next->orig == ip_this_node){
        sendMsg(ID_REPLY_ASK,seta->next->orig,ip_this_node,ip_repetidor,msg + " " +String(contador));
    }
    else{
        sendMsg(ID_REPLY_ASK,seta->next->orig,ip_this_node,ip_repetidor,msg);
    }
    doing = "msg enviada";
    return;
}
bool testaVizinhos(int origem, byte anterior){
    bool marc = false;
    int i;
    for( i = 0; i < linhas; i++){
        //verificando em qual linha está os nós de loop para marca.
        if((tabela [i] [1] == origem && tabela [i] [0] != 2) || (tabela [i] [1] == anterior && tabela [i] [0] != 2)){
            tabela [i] [0] = 1;
        }

    }
    for(i = 0; i < linhas; i++){
        //verificando se há nos além do anterior, origem e isolados.
        if(tabela[i][0] != 1 && tabela[i][0] != 2 && tabela[i][1] != -1){
            marc = true;
        }
    }
    for(i = 0; i < linhas; i++){
        //verificando em qual linha está o repetidor para desmarca.
        if(tabela[i][0] == 1){
            tabela [i] [0] = 0;
        }
    }
    return marc;
}
