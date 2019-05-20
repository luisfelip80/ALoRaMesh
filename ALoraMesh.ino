#include "header.h"

void trataRecebidos(){

    //task1_usando = true;
    setaRec = lastRecOne(recebidos);
    byte id = setaRec->next->id;
    int origem = setaRec->next->origem;
    byte anterior = setaRec->next->anterior;
    byte destino =setaRec->next->destino;
    String msg = String(setaRec->next->msg);
    int i,h=0;
    delRecList(setaRec);
    //task1_usando = false;
    switch (id) {

        case id_new_node: //alguém pediu para ser resgistrado.
            doing ="pedido novo no.";
            //requisição de um novo nó na rede
            //montagem do pacote de dados com a resposta à requisição
            sendMsg(id_reply_node,ip_this_node,ip_this_node,origem,msg);
            custo = Heltec.LoRa.packetRssi() * -1;
            verificaNos(origem, anterior , custo);
            break;
        case id_reply_node: // responderam ao meu pedido de novo nó.
            doing ="respondeu requisicao";
            //nó respondeu à requisição de informação sobre o seu custo
            // custo
            custo = Heltec.LoRa.packetRssi() * -1;
            //Serial.println(custo);

            //inclusão do nó na tabela
            verificaNos(origem, anterior , custo);
            break;
        case id_connection_lost: // perda de sinal
            doing = "no caiu";
            //verificando em qual linha está este nó
            for(i = 0; i<linhas; i++){
                if(origem == tabela[i][1]){
                    tabela [i] [0] = 0;
                    tabela [i] [1] = -1;
                    tabela [i] [2] = max;
                    nr_vizinhos--;
                }
            }
            break;
        case id_repetidor_loop: // nó que eu pedi para repetir respondende falando que já reétiu essa mensagem para outro nó.
            doing = "erro loop";
            tentativas_reenvio = 0;
            for(i = 0; i < linhas; i++){
                // Serial.print("|");    
                //verificando em qual linha está o repetidor para marca-lo
                if(anterior == tabela[i][1] && tabela [i][0] != 2){
                    tabela [i] [0] = 1;
                }
            }
            espera = true;
            lastTime = millis();
            esperaTime = random(20000) + proximoEnvio;
            doing = "enviando msg";
            sendMessage(list);
            break;
        case id_repetidor_isolado: //nó que fiz o pedido para repetir responde falando que não pode repetir.
            doing = "rpt isolado";
            tentativas_reenvio = 0;
            espera = true;
            esperaTime = random(20000) + proximoEnvio;
            for(i = 0; i < linhas; i++){
                //verificando em qual linha está o repetidor para marca-lo
                if(anterior == tabela[i][1]){
                    tabela [i] [0] = 2;
                    // 2 -> repetidor isolado não deve mais se requisitado.
                }
            }
            break;
        case id_resposta: //outro nó respondeu meu pedido de repetir com um OK!
            doing = "repetidor respondeu";
            tentativas_reenvio=0;
            espera = true;
            esperaTime = random(20000) + proximoEnvio;
            seta = lastOne(list);
            vetorFila[seta->next->orig] = false;
            delList(seta);
            tam_fila--;
            break;
        case ip_this_node: //pedido para repetir msg
            doing = "pedido para repetir";
            verificaNos(origem, anterior , custo);
            //verifica se já está na fila.
            // no futuro verificar risco de incêndio, se alto substituir o valor na fila e dar prioridade.
            // verifica se pacote está na fila.
            // se eu for o gateway isso resolve loop. 'nota mental'
            if(vetorFila[origem] == true && ip_this_node != ip_gateway){
                doing = "ja ta na fila";
                sendMsg(id_ja_ta_aqui_a_msg,origem,anterior,anterior,msg);
                break;
            }
            //verifica se já passou aqui.
            // caso seja da mesma origem mas venha de um outro nó anterior, essa mensagem vai entrar em loop.
            for(i = 0 ; i < 10 ; i++){
                if(buffer[i][0] == origem && buffer[i][1] != anterior){
                    i = 11;// chave de controle
                }
            }
            // se chegar a 10 é porque não passou aqui.
            if(i==10  || ip_this_node == ip_gateway){
                doing= "nova msg.";
                if(ip_this_node == ip_gateway){
                    Serial.print("Nova msg, node: " + String(origem) + " msg: ");
                    Serial.println(msg);
                }
                else{
                    addFila(origem,anterior, ip_this_node, list);
                    tam_fila++;
                    if(origem +1 > nVetor){
                        nVetor = origem+1;
                        vetorFila = (bool*)realloc(vetorFila, sizeof(bool)*nVetor);
                        vetorFila[origem] = true;
                    }
                    else{
                        vetorFila[origem] = true;
                    }
                    if(buffer_conter == 10){
                        buffer_conter = 0;
                    }
                    buffer[buffer_conter] [0] = origem;
                    buffer[buffer_conter] [1] = anterior;
                    buffer_conter++;
                }
                sendMsg(id_resposta,ip_this_node,ip_this_node,anterior,msg);
            }
            else{
                doing = "loop detec.";
                sendMsg(id_repetidor_loop,origem,ip_this_node,anterior,msg);
            }
            break;
        case id_ja_ta_aqui_a_msg: // repetidor responde que já tem a msg.
            doing = "rpt ja tem msg";
            // repetidor já tem a mensagem com ele.
            //move msg para final da fila.
            tentativas_reenvio=0;
            esperaTime = random(20000) + proximoEnvio;  
            seta = lastOne(list);
            aux1 = seta->next;
            aux2 = list->next;
            if(aux1->next==NULL && aux2->next==NULL){
                break;
            }
            seta->next = NULL;
            list->next = aux1;
            aux1->next = aux2;
            break;
        case id_segunda_chance: // nó que eu isolei está pedindo uma nova chance pois ele fez uma nova conexão.

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
        case id_i_wake_up: // nó que acabou de acordar pede para ser registrado.
            doing ="pedido novo no.";
            //requisição de um novo nó na rede
            //montagem do pacote de dados com a resposta à requisição
            sendMsg(id_reply_node,ip_this_node,ip_this_node,origem,msg);
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
            custo = Heltec.LoRa.packetRssi() * -1;
            verificaNos(origem, anterior , custo);
            break;
        default:
        break;
    }
}
void onReceive(int packetSize) {

    int i=0,h=0,marc=0;
      // clear the display 
    if (packetSize == 0) return;          // if there's no packet, return
    IRS_use = true;
    byte id         = LoRa.read();          // destino address
    int  origem     = LoRa.read();            // origem address
    byte anterior   = LoRa.read();
    byte destino    = LoRa.read();
    byte Length     = LoRa.read();    // incoming msg length
    char msg[tam_msg] = "";
    
    // pega mensagem
    while (LoRa.available()) {            // can't use readString() in callback
        msg[i] = (char)LoRa.read();    // add bytes one by one
        i++;
    }
    //verifica se numero de caracteres bate com o numero de caracteres recebido na mensagem
    if (Length != i) {  // check length for error
        Serial.println("Erro na msg [leght no math]");
        IRS_use = false;
        return;                         // skip rest of function
    }
    // if the destino isn't this device or ip_broadcast,
    if (destino != ip_this_node && destino != ip_broadcast) {
        Serial.println("Nao eh pra mim, erro [wrong address].");
        doing ="Nao eh pra mim";
        IRS_use = false;
        return;
    }
    Serial.println("Msg recebida. " + String(id) + String(msg));
    doing ="msg recebida.";
    addRecList(id, origem, anterior, destino, msg);
    IRS_use = false;
    LoRa.receive();
    return;
}
void estateMachine(){
    int i;
    String msg = "dados";   
    switch (estado) {
        case 0:
            
            //onReceive(LoRa.parsePacket());
            if(list->next == NULL){
                timeDraw = tela(timeDraw, ip_repetidor, ip_gateway, ip_this_node, nr_vizinhos, tam_fila, linhas, tabela, doing, radio_lora, wifi, bluetooth,0,0); // atualiza tela
            }
            else{
                timeDraw = tela(timeDraw, ip_repetidor, ip_gateway, ip_this_node, nr_vizinhos, tam_fila, linhas, tabela, doing, radio_lora, wifi, bluetooth, list->next->orig, list->next->ant);
            }
            if(recebidos->next != NULL){ // se tiver msg recebida, tratar
                radio_lora = true;
                trataRecebidos();
            }
            else{
                radio_lora = false;
            }
          break;
        case 1:// enviar msg da fila se houver.
            espera = true;
            lastTime = millis();
            esperaTime = random(20000) + proximoEnvio;
            doing = "enviando msg";
            sendMessage(list);
            break;
        case 2:// contagem de tentativas
            tentativas_reenvio = 0;
            if(ip_repetidor == ip_gateway){
                break;
            }
            for(i = 0; i<linhas; i++){
                if(ip_repetidor == tabela[i][1]){
                    tabela [i] [0] = 0;
                    tabela [i] [1] = -1;
                    tabela [i] [2] = max;
                    nr_vizinhos --;
                }
            }
            //doing = "rpt rmv.";
            sendMsg(id_connection_lost,ip_repetidor,ip_this_node,ip_broadcast,msg);
            doing = "novo rpt def.";
            break;
        case 3: // 
            //ler sensores
            if(vetorFila[ip_this_node] == false){       // verificar depois se o risco de incêndio 
                addFila(ip_this_node,ip_this_node, ip_this_node, list);
                tam_fila++;
                if(ip_this_node +1 > nVetor){
                    nVetor = ip_this_node+1;
                    vetorFila = (bool*)realloc(vetorFila, sizeof(bool)*nVetor);
                    vetorFila[ip_this_node] = true;
                    Serial.println("opa");
                }
                else{
                    Serial.println("rola");
                    vetorFila[ip_this_node] = true;
                }
                doing = "msg add fila.";
            }
            else{
                doing = "msg ja reg.";
            }
            lastCheck = millis();
            break;
        case 4:// atualizando rede.
            
            // atualiza rede.
            doing= "atualizando rede.";
            lastSendTime = millis();
            for(int i =0 ;i<5 ;i++){
                sendMsg(id_new_node,ip_this_node,ip_this_node,ip_broadcast,msg);
            }
            break;
        default:
            break;
    }
}
void setup() {
    String msg = "dados";
    int i,j,l=0;
    // WIFI Kit series V1 not support Vext control
    // Library sets automatically baud rate to 115200 in next line
    Heltec.begin(true /*DisplayEnable Enable*/, true /*Heltec.LoRa Disable*/, true /*Serial Enable*/, true /*PABOOST Enable*/, BAND /*long BAND*/);   
    LoRa.onReceive(onReceive);
    //LoRa.receive();
    //LoRa.onReceive(onReceive);
    
    Heltec.display->clear();
    Heltec.display->drawXbm(24, 0, ballooncat_width, ballooncat_height, ballooncat_bits);
    Heltec.display->display();
    delay(1500);
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 10, versao);
    Heltec.display->drawXbm(24, 0, curupira_width, curupira_height, curupira_bits);
    Heltec.display->display();
    delay(2000);
     
    Heltec.display->drawLine(49+l, 61, 49+l, 63);
    Heltec.display->display();
    lastSendTime = millis();
    //attachInterrupt(digitalPinToInterrupt(26), msgIRT, RISING);
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
        buffer[i][0] = -1;
        buffer[i][1] = -1;
    }
    for(i = 0 ;i < 5; i++){
        l++;
        Heltec.display->drawLine(49+l, 61, 49+l, 63);
        Heltec.display->display();
        sendMsg(id_i_wake_up,ip_this_node,ip_this_node,ip_broadcast,msg);
    }
    // verificando os nós vizinho
    doing= "atualizando rede.";
    l++;
    Heltec.display->drawLine(49+l, 61, 49+l, 63);
    Heltec.display->display();
    lastCheck = millis();
    lastTime = millis();
    recebidos = (rec_node*)malloc(sizeof(rec_node));
    recebidos->next = NULL;
    list = (node*)malloc(sizeof(node));
    list->orig = 0;
    list->next = NULL;
    timeDraw = millis();   
}
void loop() {

    if(estado == 0){
        
        if(millis()-lastCheck > 10000){
         estado = 3;
        }
        else if(ip_this_node == ip_gateway){
            if( nr_vizinhos < 15 && millis() - lastSendTime > 3600000){
                estado = 4;
            }
            else{
                estado = 0;
            }
        }
        else if(list->next != NULL ){
            if(espera == false){
                estado = 1;
            }
            else if(millis() - lastTime > esperaTime){
                espera = false;
                lastTime = millis();
                estado = 0;
            }
            else{
                estado = 0;
            }
        }
        else {
            estado = 0;
        }
        estateMachine();
    }
    else if(estado == 1){
        
        if(tentativas_reenvio > 5){
            estado = 2;
        }
        else {
            estado = 0;
        }
        estateMachine();
    }
    else if(estado == 2){
        estado = 0;
        estateMachine();
    }
    else if(estado == 3){
        if( nr_vizinhos < 15 && millis() - lastSendTime > 60000){
            estado = 4;
        }
        else{
            estado = 0;
        }
        estateMachine();
    }
    else{
        estado = 0;
        estateMachine();
    }
}

// void task1(void *parameter){
  
//     while(1){
//         TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
//         TIMERG0.wdt_feed=1;
//         TIMERG0.wdt_wprotect=0;
//         if(LoRa.parsePacket()!=0){
//             task2_usando = true;
//             byte id         = LoRa.read();          // destino address
//             int  origem     = LoRa.read();            // origem address
//             byte anterior   = LoRa.read();
//             byte destino    = LoRa.read();
//             byte Length = LoRa.read();    // incoming msg length
//             char msg[tam_msg] = "";

//             // pega mensagem
//              int i=0;
//             while (LoRa.available()) {            // can't use readString() in callback
//                 msg[i] += (char)LoRa.read();    // add bytes one by one
//                 i++;
//             }
//             Serial.println("Msg recebida.");
//             doing ="msg recebida.";
//             //verifica se numero de caracteres bate com o numero de caracteres recebido na mensagem
//             if (Length != i) {  // check length for error
//                 Serial.println("Erro na msg [leght no math]");
//                 task2_usando = false;
//                 return;                         // skip rest of function
//             }
//             // if the destino isn't this device or ip_broadcast,
//             if (destino != ip_this_node && destino != ip_broadcast) {
//                 Serial.println("Nao eh pra mim, erro [wrong address].");
//                 doing ="Nao eh pra mim";
//                 task2_usando = false;
//                 return;
//             }
//             addRecList(id, origem, anterior, destino, msg);
//             task2_usando = false;
//             Serial.println("oi!!");
//         }
//         delay(1);
//     }
// }
// void IRAM_ATTR msgIRT() {
//     portENTER_CRITICAL_ISR(&mux);
//         onReceive(LoRa.parsePacket());
//     portEXIT_CRITICAL_ISR(&mux);
// }
