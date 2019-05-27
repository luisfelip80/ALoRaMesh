#include "header.h"

/*
ver o que é isso 
15:52:46.997 -> Msg recebida. origem: 2 id: 13 msg dados 6
15:52:46.997 -> Nova msg, node: 2 msg: dados 6
15:52:46.997 -> mandei :6
15:53:12.640 -> Nao eh pra mim, erro [wrong address].
15:53:13.111 -> Nao eh pra mim, erro [wrong address].

*/
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

        case ID_NEW_NODE: //alguém pediu para ser resgistrado.
            doing ="pedido novo no.";
            //requisição de um novo nó na rede
            //montagem do pacote de dados com a resposta à requisição
            custo = Heltec.LoRa.packetRssi() * -1;
            verificaNos(origem, anterior , custo);
            break;
        /*case ID_CALLBACK: // responderam ao meu pedido de novo nó.
            doing ="respondeu requisicao";
            //nó respondeu à requisição de informação sobre o seu custo
            // custo
            custo = Heltec.LoRa.packetRssi() * -1;
            //inclusão do nó na tabela
            verificaNos(origem, anterior , custo);
            break;*/
        case ID_TALK:
        
            my_turn_to_talk = true;
            TIME_FOR_I_TALK = millis();
            break;
        case ID_WAIT:
        
            my_turn_to_talk = false;
            TIME_FOR_I_TALK = millis();
            break; 
        case ID_REPLY_CALL: // alguém me pediu para registrar ele como possível requisidor de repetições (eu vou repetir o sinal dele).
            
            if(ip_this_node != ip_gateway){
                t = testaVizinhos(origem, anterior);
                if(t == false){ // eu estou isolado
                    sendMsg(ID_REPETIDOR_ISOLADO, origem ,ip_this_node, anterior, msg);
                    doing = "ped. reply negado";
                    break;
                }
            }
            for(i=0;i<15;i++){
                if(repetidores[i] [0] == 0){
                    repetidores[i] [0] = origem;
                    nr_repetidores++;
                    i = 15;
                }
            }
            repetidor_bool = true;
            doing = "ped. reply ok";
            break;
        case ID_LOST_CONNECTION: // perda de sinal
            doing = "no caiu";
            //verificando em qual linha está este nó
            for(i = 0; i<linhas; i++){
                if(origem == repetidores[i][0]){
                    repetidores[i] [0] = 0;
                    nr_repetidores --;
                }
                if(origem == tabela[i][1]){
                    tabela [i] [0] = 0;
                    tabela [i] [1] = -1;
                    tabela [i] [2] = max;
                    nr_vizinhos--;
                }
            }
            break;
        case ID_REPETIDOR_LOOP: // nó que eu pedi para repetir respondende falando que já reétiu essa mensagem para outro nó.
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
            break;
        case ID_REPETIDOR_ISOLADO: //nó que fiz o pedido para repetir responde falando que não pode repetir.
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
        case ID_RESPOSTA: //outro nó respondeu meu pedido de repetir com um OK!
            doing = "repetidor respondeu";
            tentativas_reenvio=0;
            espera = true;
            esperaTime = random(20000) + proximoEnvio;
            seta = lastOne(list);
            vetorFila[seta->next->orig] = false;
            delList(seta);
            tam_fila--;
            break;
        case ID_REPLY_ASK: //pedido para repetir msg
            if(anterior != repetidores[vez][0]){
                doing ="fora de hora";
                sendMsg(ID_WAIT, ip_this_node, ip_this_node, anterior, msg);
                break;
            }
            doing = "pedido para repetir";
            verificaNos(origem, anterior , custo);
            //verifica se já está na fila.
            // no futuro verificar risco de incêndio, se alto substituir o valor na fila e dar prioridade.
            // verifica se pacote está na fila.
            // se eu for o gateway isso resolve loop. 'nota mental'
            if(vetorFila[origem] == true && ip_this_node != ip_gateway){
                doing = "ja ta na fila";
                sendMsg(ID_JA_TA_AQUI_A_MSG,origem,anterior,anterior,msg);
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
                sendMsg(ID_RESPOSTA,ip_this_node,ip_this_node,anterior,msg);
            }
            else{
                doing = "loop detec.";
                sendMsg(ID_REPETIDOR_LOOP,origem,ip_this_node,anterior,msg);
                if(origem == repetidores[i][0]){
                    repetidores[i][0] = 0;
                    nr_repetidores --;
                }
            }
            break;
        case ID_JA_TA_AQUI_A_MSG: // repetidor responde que já tem a msg.
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
        case ID_SEGUNDA_CHANCE: // nó que eu isolei está pedindo uma nova chance pois ele fez uma nova conexão.

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
        case ID_I_WAKE_UP: // nó que acabou de acordar pede para ser registrado.
            doing ="pedido novo no.";
            //requisição de um novo nó na rede
            //montagem do pacote de dados com a resposta à requisição
            sendMsg(ID_CALLBACK,ip_this_node,ip_this_node,origem,msg);
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
    Serial.println("Msg recebida. origem: " +String(origem)+ " id: "+ String(id)+" msg " + String(msg));
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
                timeDraw = tela(timeDraw, ip_repetidor, ip_gateway, ip_this_node, nr_vizinhos, tam_fila, linhas, tabela, doing, radio_lora, wifi, bluetooth,repetidor_bool,0,0); // atualiza tela
            }
            else{
                timeDraw = tela(timeDraw, ip_repetidor, ip_gateway, ip_this_node, nr_vizinhos, tam_fila, linhas, tabela, doing, radio_lora, wifi, bluetooth, repetidor_bool, list->next->orig, list->next->ant);
            }

            if(recebidos->next != NULL){ // se tiver msg recebida, tratar
                radio_lora = true;
                trataRecebidos();
            }

            if(nr_repetidores > 0){
                if(millis() - HEAR_TIME >= 60000){ // escolher quem fala, quem eu vou ouvir.
                                    
                    for(i=0; i<15 && repetidores[i][1] == 0 ;i++); // procura marcação [i][1] == 1, era quem eu estava ouvindo.
                
                    if(i<15){//achei de quem era a vez.    
                        sendMsg(ID_WAIT, ip_this_node, ip_this_node, repetidores[i][0] , msg);
                        repetidores[i][1] = 0;
                        i++;
                        if(i==15){
                            i=0;
                        }
                        while(repetidores[i][0] == 0){ // vou dar uma volta circular no vetor de repetidores para ver se tem outro requisitor.
                            i++;
                            if(i==15){
                                i=0;
                            }
                        }
                        sendMsg(ID_TALK, ip_this_node, ip_this_node, repetidores[i][0] , msg);
                        vez = i;
                    }
                    else{// Eu não estava ouvindo ninguém, vamos achar alguém, se houver.
                        for(i=0; i<15 && repetidores[i][0] == 0 ;i++); //Deve haver pois nr_repetidores tem que ser > 0 para entrar aqui  
                        if(i<15){    
                            repetidores[i][1] = 1; // marca que estou ouvindo ele.
                            sendMsg(ID_TALK, ip_this_node, ip_this_node, repetidores[i][0] , msg);        
                            vez = i;
                        }
                    }
                    HEAR_TIME = millis();
                }
            }
            else{
                repetidor_bool = false;
            }
            if(nr_vizinhos == 0 || ip_repetidor == -1){
                radio_lora = false;
            }
          break;
        case 1:// enviar msg da fila se houver.
            if(ip_this_node != ip_gateway){
                if(my_turn_to_talk){
                    
                    TIME_FOR_I_TALK = millis();
                    espera = true;
                    lastTime = millis();
                    esperaTime = random(20000) + proximoEnvio;
                    doing = "enviando msg";
                    sendMessage(list);
                }
                else if (millis() - TIME_FOR_I_TALK >= 180000){
                    seta = lastOne(list);
                    menorCusto(seta->next->orig, seta->next->ant);
                    if(ant_repetidor == ip_repetidor && ip_repetidor != -1){
                        tentativas_reenvio++;
                        sendMsg(ID_REPLY_CALL, ip_this_node, ip_this_node, ip_repetidor, msg);
                    }
                    else if(ip_repetidor == -1){
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
                    }
                    TIME_FOR_I_TALK = millis();
                }
            }
            break;
        case 2:// contagem de tentativas
            tentativas_reenvio = 0;
            if(ip_repetidor == ip_gateway){
                break;
            }
            for(i = 0; i<linhas; i++){
                if(ip_repetidor == tabela[i][1] && tabela[i][1] != -1){
                    tabela [i] [0] = 0;
                    tabela [i] [1] = -1;
                    tabela [i] [2] = max;
                    nr_vizinhos--;
                    i=linhas;
                }
            }
            //doing = "rpt rmv.";
            sendMsg(ID_LOST_CONNECTION, ip_repetidor,ip_this_node,ip_broadcast,msg);
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
                }
                else{
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
            for(int i =0 ;i<2 ;i++){
                sendMsg(ID_NEW_NODE,ip_this_node,ip_this_node,ip_broadcast,msg);
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
        repetidores[i] [0] = 0;
        repetidores[i] [1] = 0;
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
        sendMsg(ID_I_WAKE_UP,ip_this_node,ip_this_node,ip_broadcast,msg);
    }
    // verificando os nós vizinho
    doing= "atualizando rede.";
    l++;
    Heltec.display->drawLine(49+l, 61, 49+l, 63);
    Heltec.display->display();
    lastCheck = millis();
    lastTime = millis();
    TIME_FOR_I_TALK = millis();
    HEAR_TIME = millis();
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
            if( nr_vizinhos < 15 && millis() - lastSendTime > 900000){
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
        
        if(tentativas_reenvio >= 5){
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
        if( nr_vizinhos < 15 && millis() - lastSendTime > 600000){
            estado = 4;
        }
        else if(nr_vizinhos == 0 && millis() - lastSendTime > 60000){
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