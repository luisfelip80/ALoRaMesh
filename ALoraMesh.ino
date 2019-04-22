#include "heltec.h"
#include "Arduino.h"
#include <stdio.h>
#include <stdlib.h>
#include "bluetooth_img.h"
#include "wifi_img.h"
#include "radio_img.h"
#include "bateria_img.h"
#include "curupira.h"
#include "ballooncat.h"

//IP do gateway
#define ip_gateway 0x00

//IP do ip_broadcast
#define ip_broadcast 0xFF

// IP do dispositivo
#define ip_this_node 0x00

#define BAND    915E6  //you can set band here directly,e.g. 868E6,915E6
#define linhas 15
#define colunas 3
#define max 9999999

typedef struct Node{
    byte orig, ant;
    struct Node *next;
}node;

// id do Algoritmo
#define id_new_node             1
#define id_reply_node           2
#define id_connection_lost      3
#define id_repetidor_isolado    4
#define id_repetidor_loop       5
#define id_resposta             6
#define id_ja_ta_aqui_a_msg     7
#define id_segunda_chance       8



// tabela de vizinhos
int tabela [linhas][colunas];

//variáveis do tipo inteiro
int nr_vizinhos = 0;
int custo;
int contador;
int contador_linhas;
int sinal;
int id;
int tentativas_reenvio;
int no_repetidor;

bool wifi = false;
bool bluetooth = false;
bool radio_lora = false;
bool freesend=false;
byte confir[15][3];
bool espera = false;
bool isol = false;
byte buffer[10];
int  buffer_conter=0;
bool *vetorFila;
int nVetor = 15;
String doing;
//variáveis do tipo string

String package;
byte ip_anterior;
byte ip_vizinho;
int ip_repetidor = -1;
byte ip_origem;
byte ip_filho;

//Flags de envio e recebimento de pacotes
bool sendBlock = false;
bool ack_recebido = true;

String outgoing;              // outgoing message
byte msgCount = 0;            // count of outgoing messages
long lastSendTime = 0;        // last send time
long lastTime=0;
int interval = 2000;          // interval between sends
long lastCheck = 0;
long esperaTime = 20000;
long timeDraw ;

node *list,*seta,*aux1,*aux2;

void sendMsg(byte id, int origem, byte anterior, byte destino, String msg){
    LoRa.beginPacket();                   // start packet
    LoRa.write(id);              // ID de identificação
    LoRa.write(origem);                // Destino
    LoRa.write(anterior);                       // ip_this_node
    LoRa.write(destino);
    LoRa.write(msg.length());
    LoRa.print(msg);                 // add payload
    LoRa.endPacket();
}
node *lastOne(){
    node *cur = list;
    node *aux;

    while(cur->next != NULL){
        aux = cur;
        cur = cur->next;
    }
    return aux;
}
void drawFontFaceDemo() {
    // Font Demo1
    String repetidor = String(ip_repetidor);
    if(ip_repetidor == ip_gateway){
      repetidor = String('G');
    }
    else if(ip_repetidor == -1){
      repetidor = String('-'); 
    }
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_16);
    if(ip_this_node!=ip_gateway){
        Heltec.display->drawString(0, 10, "  IP      "+String(ip_this_node));
    }
    else{
        Heltec.display->drawString(0, 10, "       Gateway");
    }
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    if(ip_this_node == ip_gateway){
        if(list->next != NULL){
            Heltec.display->drawString(0,30, "   last rec Orig: "+ String(list->next->orig)+" ate: "+String(list->next->ant));
        }
        else{
            Heltec.display->drawString(0,30, "   Fila vazia");
        }
    }
    else{
        Heltec.display->drawString(0,30, "   no rep:   "+ repetidor);
    }
    Heltec.display->drawString(0,40, "   nº viz:    "+String(nr_vizinhos));
    Heltec.display->drawString(0,54, ">"+doing);
}
void drawImage(int bateria) {
   
    for(int i =0 ; i< 6;i++){
          Heltec.display->drawLine(117+i, 3 + bateria,117+i, 13);
    }
    Heltec.display->drawXbm(112, 0, bateria_width, bateria_height, bateria_bits);
    if(radio_lora){
      Heltec.display->drawXbm(112, 16, radio_width, radio_height, radio_bits);
    } 
    if(wifi){
      Heltec.display->drawXbm(112, 32, wifi_width, wifi_height, wifi_bits);
    }
    if(bluetooth){
      Heltec.display->drawXbm(112, 48, bluetooth_width, bluetooth_height, bluetooth_bits);
    }
}
void showNos(){
    String vi[15];
    for( int i =0;i<linhas;i++){
        if(tabela [i][1] == 0){
            vi[i]=String('G');
        }
        else if(tabela[i][1] != -1){
            if(tabela[i][0] == 2){
                vi[i] = '('+String(tabela[i][1])+')';
            }
            else{
                vi[i] = String(tabela[i][1]);
            }
        }
        else{
            vi[i] = String(0);
        }
    }
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0,"  Vizinhos:");
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 15,"    "+vi[0]+"    " +vi[1]+"    "+vi[2]+"    "+vi[3]+"    "+vi[4]);
    Heltec.display->drawString(0, 25,"    "+vi[5]+"    " +vi[6]+"    "+vi[7]+"    "+vi[8]+"    "+vi[9]);
    Heltec.display->drawString(0, 35,"    "+vi[10]+"    " +vi[11]+"    "+vi[12]+"    "+vi[13]+"    "+vi[14]);
    Heltec.display->drawString(0,54, ">"+doing);
}
void menorCusto(){
    int i , origem, anterior;
    seta = lastOne();
    origem = seta->next->orig;
    anterior = seta-> next->ant;
    doing = "definir repetidor";
    for(i = 0; i < linhas; i++){
        Serial.print("|");
        //verificando em qual linha está o repetidor para marca-lo
        if((tabela [i] [1] == origem && tabela [i] [0] != 2) || (tabela [i] [1] == anterior && tabela [i] [0] != 2)){
            tabela[i][0] = 1;
        }
        if(tabela[i][0] != 2 && tabela[i][0] != 1){
            no_repetidor = tabela[i][2];
            ip_repetidor = tabela[i][1];
        }
    }
    Serial.println("no repetidor" +String(no_repetidor));
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
    Serial.println("repetidor: "+String(ip_repetidor));
    doing = "repetidor def.";
}
void addList( byte origem, byte anterior){
    node *cur=list;
    node *newN=(node *) malloc(sizeof(node)); // cria novo nó.
    node *aux= list->next;

    newN->orig = origem;
    newN->ant = anterior;
    newN->next = aux; 
    list->next = newN;
    return; // retorna lista atualizada.
}
void delList (node *cur){
    int i;
    if(cur == NULL) // caso não seja encontrado, a respota da função anterior será null, nesse caso não é necessário deletar nada.
        return;
    //printf("%s\n",cur->title);
    node *file_before = cur;  // caso seja encontrado o elemento, um ponteiro auxiliar o salva enquando o elemento anterior a ele é atualizado para
    node *file_search_for = cur->next; // apontar para o elemento posteior ao elemento salvo.
    cur=file_search_for->next;//cur vai para file após a procurada.
    file_before->next = cur;//file anterior à search_for aponta para a música após ela. 
    free(file_search_for);
    doing = "msg deletada";
}
void addFila( byte origem, byte anterior){
    Serial.println("verifica se há repetidor");
    String msg = "dados";
    doing = "verifica repetidores";
    int i,marc = 0;
    //verificar se o pacote não entrará em loop
    //Verifica se tem loop.
    for(i = 0; i < linhas; i++){
        //verificando em qual linha está os nós de loop para marca.
        if((tabela [i] [1] == origem && tabela [i] [0] != 2) || (tabela [i] [1] == anterior && tabela [i] [0] != 2)){
            tabela [i] [0] = 1;
        }
    }
    for(i = 0; i < linhas; i++){
        //verificando se há nos além do anterior, origem e isolados.
        if(tabela[i][0] != 1 && tabela[i][0] != 2 && tabela[i][1] != -1){
            marc = 1;
        }
    }
    for(i = 0; i < linhas; i++){
        //verificando em qual linha está o repetidor para desmarca.
        if(tabela[i][0] == 1){
            tabela [i] [0] = 0;
        }
    }
    if(marc == 0 && ip_gateway == ip_this_node){
        Serial.println("Gateway responde.");
        doing = "Gateway responde..";
        Serial.println("Adiciona a fila de espera.");
        doing = "msg add fila.";
        addList(origem, anterior);
        sendMsg(id_resposta,origem,ip_this_node,anterior,msg);
        return;
    }
    // não tem nós alem da origem e anterior. Não posso ser usado como repetidor por esse nó anterior.
    if(marc == 0){
        isol = true;
        Serial.println('<');
        Serial.println("Repetidor isolado.");
        doing = "Repetidor isolado.";
        Serial.println("Configurando retorno ao anterior com msg de erro [no way].");
        sendMsg(id_repetidor_isolado,origem,anterior,anterior,msg);
        Serial.println("Enviado.");
        return;
    }
    // tem mais nós além do repetidor e anterior.
    // proximo teste é seguro.
    else{
        Serial.println("< pronto. Temos repetidor.");
        // dados para replicar
        //verifica se essa msg já passou aqui
        // add a fila de espera.
        Serial.println("Adiciona a fila de espera.");
        doing = "msg add fila.";
        addList(origem, anterior);
        if(origem +1 > nVetor){
            nVetor = origem+1;
            vetorFila = (bool*)realloc(vetorFila, sizeof(bool)*nVetor);
            vetorFila[origem] = true;
        }
        else{
            vetorFila[origem] = true;
        }
        if(buffer_conter==10){
            buffer_conter = 0;
        }
        Serial.println("Adiciona a buffer de msg passadas aqui.");
        buffer[buffer_conter] = origem;
        buffer_conter++;
        // responder ao anterior.
        sendMsg(id_resposta,origem,ip_this_node,anterior,msg);
        // guardar para função de envio.
        /*
            //mensagem recebida deve ser encaminhada
            
        */
    }
}
void onReceive(int packetSize) {

    int i,h=0,marc=0;
      // clear the display 
    if (packetSize == 0) return;          // if there's no packet, return
    String msg = "dados";
    // read packet header bytes:
    byte id = LoRa.read();          // destino address
    int origem = LoRa.read();            // origem address
    byte anterior = LoRa.read();
    byte destino = LoRa.read();
    byte incomingLength = LoRa.read();    // incoming msg length
    String incoming = "";                 // payload of packet

    // pega mensagem
    while (LoRa.available()) {            // can't use readString() in callback
        incoming += (char)LoRa.read();    // add bytes one by one
    }
    Serial.println("Msg recebida.");
    doing ="msg recebida.";
    //verifica se numero de caracteres bate com o numero de caracteres recebido na mensagem
    if (incomingLength != incoming.length()) {  // check length for error
        Serial.println("Erro na msg [leght no math]");
        return;                           // skip rest of function
    }
    // if the destino isn't this device or ip_broadcast,
    if (destino != ip_this_node && destino != ip_broadcast) {
        Serial.println("Nao eh pra mim, erro [wrong address].");
        doing ="Nao eh pra mim";
        return;                           // skip rest of function
    }
    radio_lora = true;
    switch (id) {

        case id_new_node:
            Serial.println("Pedido de novo no.");
            doing ="pedido novo no.";
            //requisição de um novo nó na rede
            //montagem do pacote de dados com a resposta à requisição
            sendMsg(id_reply_node,ip_this_node,ip_this_node,origem,msg);

            //verificando se o nó já é um vizinho
            Serial.println("verificando nos");
            doing = "verificando nos";
            custo = Heltec.LoRa.packetRssi() * -1;
            h=0;
            if(nr_vizinhos < 15){
                for(i = 0; i< linhas; i ++){
                    //verificando se é um nó novo na rede
                    if(origem == tabela[i][1]){
                        //não é novo
                        h=1;
                        if(custo != tabela [i] [2]){
                            tabela [i] [2] = custo;
                        }
                        i=linhas;
                    }
                }
                if(h == 0){
                    Serial.println("Novo no.");
                    doing = "Novo no.";
                    if(isol){
                        sendMsg(id_segunda_chance,ip_this_node,ip_this_node,ip_broadcast,msg);
                    }
                    for(i = 0; i< linhas; i ++){
                        if(tabela[i][1] == -1){
                            tabela [i] [1] = origem;
                            tabela [i] [2] = custo;
                            i=linhas;
                            nr_vizinhos++;
                        }
                    }
                    Serial.println("Adicionado a tabela.");
                    doing = "add a tabela";
                }
                else{
                    Serial.println("no ja registrado.");
                    doing= "no ja reg.";
                }
            }
            //malandragem.
            //o correto deveria enviar a origem e o anterior para calcular novo repetidor, mas dentro desse case eu posso
            // fazer isso.
            break;
        case id_reply_node:
            Serial.println("No respondeu ao pedido de informacao.");
            doing ="respondeu requisicao";
            //nó respondeu à requisição de informação sobre o seu custo
            // custo
            custo = Heltec.LoRa.packetRssi() * -1;
                        Serial.println(custo);

            //inclusão do nó na tabela
            h=0;
            doing = "verificando nos";
            for(i = 0; i< linhas; i ++){
                //verificando se é um nó novo na rede
                if(origem == tabela[i][1]){
                    //não é novo
                    h=1;
                    if(custo != tabela [i] [2]){
                        tabela [i] [2] = custo;
                    }
                    i=linhas;
                }
            }
            if(h == 0){
                Serial.println("Novo no.");
                doing = "Novo no.";
                if(isol){
                    sendMsg(id_segunda_chance,ip_this_node,ip_this_node,ip_broadcast,msg);
                }
                for(i = 0; i< linhas; i ++){
                    if(tabela[i][1] == -1){
                        tabela [i] [1] = origem;
                        tabela [i] [2] = custo;
                        nr_vizinhos++;
                        i=linhas;
                        h=0;
                    }
                }
                Serial.println("Adicionado a tabela.");
                //malandragem.
            }          
            Serial.println("pronto.");
            Serial.print("Repetidor: no");
            Serial.println(ip_repetidor);
            Serial.println("SendBlock!!!");
            break;
        case id_connection_lost: // perda de sinal
            doing = "no caiu";
            //verificando em qual linha está este nó
            Serial.println("Perda de sinal identificada.");
            Serial.println("Removendo no da tabela.");
            for(i = 0; i<linhas; i++){
                if(origem == tabela[i][1]){
                    tabela [i] [0] = 0;
                    tabela [i] [1] = -1;
                    tabela [i] [2] = max;
                    nr_vizinhos--;
                }
            }
            break;
        case id_repetidor_loop:
            doing = "erro loop"; 
            Serial.println("Erro [loop way]");
            Serial.println(" Loop.");
            tentativas_reenvio = 0;
            esperaTime=0;
            Serial.print("Procurando novo repetidor.");
            while(ip_repetidor == anterior || ip_repetidor == origem){
                Serial.print(".");
                //criar um novo repetidor, pois o pacote entrará em loop 
                for(i = 0; i < linhas; i++){
                    //verificando em qual linha está o repetidor para marca-lo
                    if(ip_repetidor == tabela[i][1] && tabela[i][1]!=ip_gateway && tabela[i][0] != 2){
                        tabela [i] [0] = 1;
                        // 1 -> repetidor não deve ser requisitado para esse pacote.
                    }
                }
            }
            Serial.println("Pronto, novo repetidor definido.");
            doing= "novo no def.";
            Serial.print("Repetidor: ");
            Serial.println(ip_repetidor);
            break;
        case id_repetidor_isolado:
            Serial.println("Erro [no way]");
            Serial.println(" Repetidor isolado.");
            doing = "rpt isolado";
            tentativas_reenvio = 0;
            esperaTime=0;
            Serial.print("Procurando novo repetidor >");
            for(i = 0; i < linhas; i++){
                Serial.print("|");    
                //verificando em qual linha está o repetidor para marca-lo
                if(ip_repetidor == tabela[i][1]){
                    tabela [i] [0] = 2;
                    // 2 -> repetidor isolado não deve mais se requisitado.
                }
            }
            doing = "no removido";
            Serial.println('<');
            Serial.println("Pronto, novo repetidor definido.");
            doing ="novo rpt def.";
            Serial.print("Repetidor: ");
            Serial.println(ip_repetidor);
            break;
        case id_resposta:
            doing = "repetidor respondeu";
            Serial.println("Repetidor respondeu, msg recebida.");
            tentativas_reenvio=0;
            Serial.println("Alongar tempo para proximo envio");
            esperaTime = 60000;    
            seta = lastOne();
            vetorFila[seta->next->orig] = false;
            delList(seta);
            Serial.println("Msg removida da fila.");
            doing = "msg remov. fila";
            break;
        case ip_this_node:
            doing = "pedido para repetir";
            Serial.print("Pedido de requisicao do no: ");
            Serial.println(anterior);
            //verifica se já está na fila.
            // no futuro verificar risco de incêndio, se alto substituir o valor na fila e dar prioridade.
            // verifica se pacote está na fila.
            // se eu for o gateway isso resolve loop. 'nota mental'
            Serial.println("Verifica se msg ja esta na fila.");
            if(vetorFila[origem] == true && ip_this_node != ip_gateway){
                Serial.println("Esta na fila de espera");
                doing = "ja ta na fila";
                sendMsg(id_ja_ta_aqui_a_msg,origem,anterior,anterior,msg);
                break;
            }
            Serial.println("Nova msg.");
            //verifica se já passou aqui.
            Serial.println("Verificacao de loop, essa msg ja passou aqui?");
            Serial.println("Verificar buffer de origens.");
            for(i = 0 ; i < 10 ; i++){
                if(buffer[i] == origem){
                    i = 11;// chave de controle
                }
            }
            // se chegar a 10 é porque não passou aqui.
            if(i==10  || ip_this_node == ip_gateway){
                Serial.println("Nao.");
                doing= "nova msg.";
                addFila(origem,anterior);
            }
            else if(ip_this_node != ip_gateway){
                Serial.println("Sim.");
                doing = "loop detec.";
                Serial.println("Preparar msg de retorno ao anterior.");
                sendMsg(id_repetidor_loop,origem,anterior,anterior,msg);
                doing = "resp. envianda";
            }
            break;
        case id_ja_ta_aqui_a_msg: // repetidor responde que já tem a msg.
            doing = "rpt ja tem msg";
            // repetidor já tem a mensagem com ele.
            //move msg para final da fila.
            Serial.println("Msg ja esta no repetidor");
            esperaTime = 60000;  
            seta = lastOne();
            aux1 = seta->next;
            aux2 = list->next;
            if(aux1->next==NULL && aux2->next==NULL){
                Serial.println("break!");
                break;
            }
            seta->next = NULL;
            list->next = aux1;
            aux1->next = aux2;
            break;
        case id_segunda_chance:
            Serial.println("segunda chance");
            Serial.println(" Repetidor isolado.");

            for(i = 0; i < linhas; i++){
                //verificando em qual linha está o repetidor para desmarca-lo
                if(origem == tabela[i][1] && tabela[i][0] != 2){
                    i=linhas;
                }
                else if (origem == tabela[i][1] && tabela[i][0] == 2){
                   tabela[i][0] = 0;
                   i=linhas;
                }
            }
            doing = "no desmarcado";
            break;
        default: 
            Serial.println("ID invalido");    
    }
    // if message is for this device, or ip_broadcast, print details:
    //Serial.println("Received from: 0x" + String(origem, HEX));
    //Serial.println("Sent to: 0x" + String(destino, HEX));
    //Serial.println("Message ID: " + String(incomingMsgId));
    //Serial.println("Message length: " + String(incomingLength));
    //Serial.println("Message: " + incoming);
    //Serial.println("RSSI: " + String(LoRa.packetRssi()));
    //Serial.println("Snr: " + String(LoRa.packetSnr()));
    //Serial.println();
}
void setup() {
    String msg = "dados";
    int i,j,l=0;
    // WIFI Kit series V1 not support Vext control
    // Library sets automatically baud rate to 115200 in next line
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);
    
    Heltec.display->clear();
    Heltec.display->drawXbm(24, 0, ballooncat_width, ballooncat_height, ballooncat_bits);
    Heltec.display->display();
    delay(1500);
    Heltec.display->clear();
    Heltec.display->drawXbm(24, 0, curupira_width, curupira_height, curupira_bits);
    Heltec.display->display();
    delay(2000);
     
    Heltec.display->drawLine(49+l, 61, 49+l, 63);
    Heltec.display->display();
    lastSendTime = millis();
    Serial.println("Heltec.LoRa init succeeded.");
    vetorFila = (bool*)malloc(sizeof(bool)*15);
    for( i = 0; i< 15 ; i++){
        l++;
        Heltec.display->drawLine(49+l, 61, 49+l, 63);
        Heltec.display->display();
        tabela [i] [0] = 0;
        tabela [i] [1] = -1;
        tabela [i] [2] = max;
        vetorFila[i]=false;
    }
    buffer_conter=0;
    for(i=0;i<10;i++){
        l++;
        Heltec.display->drawLine(49+l, 61, 49+l, 63);
        Heltec.display->display();
        buffer[i] = -1;
    }
    
    // verificando os nós vizinhos
    Serial.println("Enviando Broadcast.");
    doing= "atualizando rede.";
    for(i = 0 ;i < 5; i++){
        l++;
        Heltec.display->drawLine(49+l, 61, 49+l, 63);
        Heltec.display->display();
        sendMsg(id_new_node,ip_this_node,ip_this_node,ip_broadcast,msg);
        onReceive(LoRa.parsePacket());
    }
    l++;
    Heltec.display->drawLine(49+l, 61, 49+l, 63);
    Heltec.display->display();
    lastCheck = millis();
    lastTime = millis();
    list = (node*)malloc(sizeof(node));
    list->orig = 0;
    list->next = NULL;
    Serial.println("Criada fila.");
    timeDraw = millis();
}
void loop() {
  int i;
    String msg = "dados"; 
    Heltec.display->clear();
    // draw the current demo method
    if(millis() - timeDraw < 10000){
        drawFontFaceDemo();
        drawImage(0);
    }
    else if(millis() - timeDraw > 10000){
        showNos();
    }
    if(millis() - timeDraw > 20000){
        timeDraw = millis();
    }
    // write the buffer to the display
    Heltec.display->display();
 
    if(millis()-lastCheck > 30000 && ip_this_node != ip_gateway){
        //ler sensores
        Serial.println("Lendo sensores.");
        doing= "lendo senores.";
        if(vetorFila[ip_this_node] == false){ // verificar depois se o risco de incêndio aumentou, caso sim alterar valor na fila e aumentar prioridade.
            addList(ip_this_node, ip_this_node);
            if(ip_this_node +1 > nVetor){
                nVetor = ip_this_node+1;
                vetorFila = (bool*)realloc(vetorFila, sizeof(bool)*nVetor);
                vetorFila[ip_this_node] = true;
            }
            else{
                vetorFila[ip_this_node] = true;
            }
            Serial.println("Msg adicionada a fila.");
            doing = "msg add fila.";
        }
        else{
            Serial.println("Msg ja esta adicionada a fila.");
            doing = "msg ja reg.";
        }
        /*     
        seta = list;   
        Serial.println("Lista:");
        while(seta->next != NULL){
            Serial.print("->");
            Serial.print(seta->orig);
            seta = seta->next;
        }
        Serial.println("fim");
        */
        lastCheck = millis();
        
    }
    if(millis() - lastSendTime > 60000){
        
        //reseta buffer
        // atualiza rede.
        Serial.println("Atualizando tabela, enviando broadcast.");
        doing= "atualizando rede.";
        lastSendTime = millis();
        for(int i =0 ;i<5 ;i++){
            sendMsg(id_new_node,ip_this_node,ip_this_node,ip_broadcast,msg);
        }
    }
    if(list->next != NULL && espera == false && ip_this_node != ip_gateway){
        menorCusto();
        espera = true;
        lastTime = millis();
        esperaTime = 20000;
        Serial.println("Enviando mensagem da fila.");
        doing = "enviando msg";
        sendMessage();
    }
    if(millis() - lastTime > esperaTime){
        espera = false;
        lastTime = millis();
    }

    if(tentativas_reenvio > 3 && ip_gateway != ip_repetidor){
        Serial.println("3 tres tentativas falharam.");
        doing = "3 tentativas fal.";
        tentativas_reenvio = 0;

        Serial.println("Procurando no repetidor na tabela.");
        for(i = 0; i<linhas; i++){
            if(ip_repetidor == tabela[i][1]){
                tabela [i] [0] = 0;
                tabela [i] [1] = -1;
                tabela [i] [2] = max;
                nr_vizinhos --;
                Serial.println("Removido.");
            }
        }
        doing = "rpt rmv.";
        sendMsg(id_connection_lost,ip_repetidor,ip_this_node,ip_broadcast,msg);
        Serial.println("Enviada msg para vizinhos informando perda de no.");
        doing = "novo rpt def.";
        Serial.println("Novo repetidor definido.");
    }
    //Serial.println(String(ip_repetidor));
    //Serial.println("    "+String(tabela[0][0])+"    " +String(tabela[1][0])+"    "+String(tabela[2][0])+"    "+String(tabela[3][0])+"    "+String(tabela[4][0])+"    "+String(tabela[5][0])+"    " +String(tabela[6][0])+"    "+String(tabela[7][0])+"    "+String(tabela[8][0])+"    "+String(tabela[9][0])+"    "+String(tabela[10][0])+"    " +String(tabela[11][0])+"    "+String(tabela[12][0])+"    "+String(tabela[13][0])+"    "+String(tabela[14][0]));

    onReceive(LoRa.parsePacket());
}
void sendMessage() {
    int i,marc=0;
    seta = lastOne();
    String msg = "dados"; 
    // tem mais nós além do repetidor e anterior.
    // proximo teste é seguro.
    if(ip_repetidor == -1 ){
        Serial.println("Repetidor isolado.");
        Serial.println("Configurando retorno ao anterior com msg de erro [no way].");
        if(ip_this_node != seta->next->ant){
            sendMsg(id_repetidor_isolado,seta->next->orig,seta->next->ant,seta->next->ant,msg);
            isol = true;
        }
        Serial.println("Enviado.");
        seta = lastOne();
        vetorFila[seta->next->orig] = false;
        delList(seta);
        Serial.println("Msg removida da fila.");
        return;
    }
    Serial.println("< pronto.");
    tentativas_reenvio ++;
    sendMsg(ip_repetidor,seta->next->orig,ip_this_node,ip_repetidor,msg);
    Serial.println("Enviado.");
}



