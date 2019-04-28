#include "soc/timer_group_struct.h"
#include "soc/timer_group_reg.h"
#include "heltec.h"
#include "Arduino.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bluetooth_img.h"
#include "wifi_img.h"
#include "radio_img.h"
#include "bateria_img.h"
#include "curupira.h"
#include "ballooncat.h"

TaskHandle_t Task1;

//IP do gateway
#define ip_gateway 0x00

//IP do ip_broadcast
#define ip_broadcast 0xFF

// IP do dispositivo
#define ip_this_node 0x0a


#define tam_msg 20

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define linhas 15
#define colunas 3
#define max 9999999

#define id_new_node             1
#define id_reply_node           2
#define id_connection_lost      3
#define id_repetidor_isolado    4
#define id_repetidor_loop       5
#define id_resposta             6
#define id_ja_ta_aqui_a_msg     7
#define id_segunda_chance       8
#define id_i_wake_up            9

typedef struct Node{
    byte orig, ant;
    struct Node *next;
}node;

typedef struct Rec{
    byte id, origem, anterior, destino;
    char msg[tam_msg];
    struct Rec *next;
}rec_node;
// id do Algoritmo



// tabela de vizinhos
int tabela [linhas][colunas];

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
bool freesend=false;
byte confir[15][3];
bool espera = false;
bool tenhoViz = false;
volatile bool IRS_use = false;

byte buffer[10];
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

byte tam_fila = 0;

//Flags de envio e recebimento de pacotes
bool sendBlock = false;
bool ack_recebido = true;

byte gId;
int gOrigem;
byte gAnterior;
byte gDestino;
String gMsg;

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
long lastTime=0;
int interval = 2000;          // interval between sends
long lastCheck = 0;
long esperaTime = 20000;
long timeDraw ;

node *list,*seta,*aux1,*aux2;
rec_node  *recebidos, *setaRec;

// Para configurar seções críticas (interrupções de ativação e interrupções de desativação não disponíveis)
// usado para desabilitar e interromper interrupções
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;



