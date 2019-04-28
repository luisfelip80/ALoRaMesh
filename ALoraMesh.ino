#include "ALoraMesh.h"

void sendMsg(byte id, int origem, byte anterior, byte destino, String msg){
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
node *lastOne(){
    node *cur = list;
    node *aux;

    while(cur->next != NULL){
        aux = cur;
        cur = cur->next;
    }
    return aux;
}
rec_node *lastRecOne(){
    rec_node *cur = recebidos;
    rec_node *aux;

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
            Heltec.display->drawString(0,30, "   last rec Orig "+ String(list->next->orig)+" ate "+String(list->next->ant));
        }
        else{
            Heltec.display->drawString(0,30, "   Fila vazia");
        }
    }
    else{
        Heltec.display->drawString(0,30, "   no rep   "+ repetidor);
    }
    Heltec.display->drawString(0,40, "   nº viz  "+String(nr_vizinhos)+"  tª fila "+String(tam_fila));
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
void menorCusto(int origem,int anterior){
    int i ;
    String msg = "dados";
    doing = "definir repetidor";
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
}
void addRecList(byte id, int origem, byte anterior, byte destino, char msg[tam_msg]){

    rec_node *cur=recebidos;
    rec_node *newN=(rec_node *) malloc(sizeof(rec_node)); // cria novo nó.
    rec_node *aux= recebidos->next;
    newN->id = id;
    newN->origem = origem;
    newN->anterior = anterior;
    newN->destino = destino;
    strcpy(newN->msg,msg);
    newN->next = aux; 
    recebidos->next = newN;
    Serial.println("msg Add");
}
void addList( byte origem, byte anterior){
    node *cur=list;
    node *newN=(node *) malloc(sizeof(node)); // cria novo nó.
    node *aux= list->next;

    newN->orig = origem;
    newN->ant = anterior;
    newN->next = aux; 
    list->next = newN;
    tam_fila++;
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
    tam_fila--;
    doing = "msg deletada";
}
void delRecList (Rec *cur){
    int i;
    portENTER_CRITICAL_ISR(&mux);
    if(cur == NULL) // caso não seja encontrado, a respota da função anterior será null, nesse caso não é necessário deletar nada.
        return;
    //printf("%s\n",cur->title);
    rec_node *file_before = cur;  // caso seja encontrado o elemento, um ponteiro auxiliar o salva enquando o elemento anterior a ele é atualizado para
    rec_node *file_search_for = cur->next; // apontar para o elemento posteior ao elemento salvo.
    cur=file_search_for->next;//cur vai para file após a procurada.
    file_before->next = cur;//file anterior à search_for aponta para a música após ela. 
    free(file_search_for);
    doing = "msg deletada";
    portEXIT_CRITICAL_ISR(&mux);
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
void addFila( byte origem, byte anterior){
    String msg = "dados";
    int i;
    bool marc = false;
    //verificar se o pacote não entrará em loop
    //Verifica se tem loop.

    // gateway não enfileira nada, apenas as msgs que recebe.
    if(ip_gateway == ip_this_node){
        doing = "Gateway responde..";
        addList(origem, anterior);
        sendMsg(id_resposta,origem,ip_this_node,anterior,msg);
        return;
    }
    marc = testaVizinhos(origem, anterior);
    // não tem nós alem da origem e anterior. Não posso ser usado como repetidor por esse nó anterior.
    if(!marc){
        isol = true;
        doing = "Repetidor isolado.";
        sendMsg(id_repetidor_isolado,origem,ip_this_node,anterior,msg);
        return;
    }
    // tem mais nós além do repetidor e anterior.
    // proximo teste é seguro.
    else{
        // dados para replicar
        //verifica se essa msg já passou aqui
        // add a fila de espera.
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
                sendMsg(id_segunda_chance,ip_this_node,ip_this_node,ip_broadcast,msg);
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
void trataRecebidos(){

    //task1_usando = true;
    setaRec = lastRecOne();
    byte id = setaRec->next->id;
    int origem = setaRec->next->origem;
    byte anterior = setaRec->next->anterior;
    byte destino =setaRec->next->destino;
    String msg = String(setaRec->next->msg);
    int i,h=0;
    delRecList(setaRec);
    //task1_usando = false;
    switch (id) {

        case id_new_node:
            doing ="pedido novo no.";
            //requisição de um novo nó na rede
            //montagem do pacote de dados com a resposta à requisição
            sendMsg(id_reply_node,ip_this_node,ip_this_node,origem,msg);
            custo = Heltec.LoRa.packetRssi() * -1;
            verificaNos(origem, anterior , custo);
            break;
        case id_reply_node:
            doing ="respondeu requisicao";
            //nó respondeu à requisição de informação sobre o seu custo
            // custo
            custo = Heltec.LoRa.packetRssi() * -1;
            Serial.println(custo);

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
        case id_repetidor_loop:
            doing = "erro loop";
            tentativas_reenvio = 0;
            esperaTime=0;
            for(i = 0; i < linhas; i++){
                Serial.print("|");    
                //verificando em qual linha está o repetidor para marca-lo
                if(anterior == tabela[i][1] && tabela [i][0] != 2){
                    tabela [i] [0] = 1;
                }
            }
            break;
        case id_repetidor_isolado:
            doing = "rpt isolado";
            tentativas_reenvio = 0;
            esperaTime=0;
            for(i = 0; i < linhas; i++){
                //verificando em qual linha está o repetidor para marca-lo
                if(anterior == tabela[i][1]){
                    tabela [i] [0] = 2;
                    // 2 -> repetidor isolado não deve mais se requisitado.
                }
            }
            break;
        case id_resposta:
            doing = "repetidor respondeu";
            tentativas_reenvio=0;
            esperaTime = random(2000) + 50000;    
            seta = lastOne();
            vetorFila[seta->next->orig] = false;
            delList(seta);
            break;
        case ip_this_node:
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
            for(i = 0 ; i < 10 ; i++){
                if(buffer[i] == origem){
                    i = 11;// chave de controle
                }
            }
            // se chegar a 10 é porque não passou aqui.
            if(i==10  || ip_this_node == ip_gateway){
                doing= "nova msg.";
                addFila(origem,anterior);
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
            esperaTime = 60000;  
            seta = lastOne();
            aux1 = seta->next;
            aux2 = list->next;
            if(aux1->next==NULL && aux2->next==NULL){
                break;
            }
            seta->next = NULL;
            list->next = aux1;
            aux1->next = aux2;
            break;
        case id_segunda_chance:

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
        break;
    }
}
void task1(void *parameter){
  
    while(1){
        TIMERG0.wdt_wprotect=TIMG_WDT_WKEY_VALUE;
        TIMERG0.wdt_feed=1;
        TIMERG0.wdt_wprotect=0;
        if(LoRa.parsePacket()!=0){
            task2_usando = true;
            byte id         = LoRa.read();          // destino address
            int  origem     = LoRa.read();            // origem address
            byte anterior   = LoRa.read();
            byte destino    = LoRa.read();
            byte Length = LoRa.read();    // incoming msg length
            char msg[tam_msg] = "";

            // pega mensagem
             int i=0;
            while (LoRa.available()) {            // can't use readString() in callback
                msg[i] += (char)LoRa.read();    // add bytes one by one
                i++;
            }
            Serial.println("Msg recebida.");
            doing ="msg recebida.";
            //verifica se numero de caracteres bate com o numero de caracteres recebido na mensagem
            if (Length != i) {  // check length for error
                Serial.println("Erro na msg [leght no math]");
                task2_usando = false;
                return;                         // skip rest of function
            }
            // if the destino isn't this device or ip_broadcast,
            if (destino != ip_this_node && destino != ip_broadcast) {
                Serial.println("Nao eh pra mim, erro [wrong address].");
                doing ="Nao eh pra mim";
                task2_usando = false;
                return;
            }
            addRecList(id, origem, anterior, destino, msg);
            task2_usando = false;
            Serial.println("oi!!");
        }
        delay(1);
    }
}
void IRAM_ATTR msgIRT() {
    portENTER_CRITICAL_ISR(&mux);
        onReceive(LoRa.parsePacket());
    portEXIT_CRITICAL_ISR(&mux);
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
    Serial.println("Msg recebida.");
    doing ="msg recebida.";
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
    Serial.println("msg ok");
    addRecList(id, origem, anterior, destino, msg);
    IRS_use = false;
    LoRa.receive();
    return;
}
void sendMessage() {
    int i,marc=0;
    seta = lastOne();
    String msg = "dados"; 
    // tem mais nós além do repetidor e anterior.
    // proximo teste é seguro.
    menorCusto(seta->next->orig,seta->next->ant);
    if(ip_repetidor == -1 ){
        doing = "Repetidor isolado";
        if(ip_this_node != seta->next->ant){
            isol = true;
            for(i=0; i < 2; i++){
                sendMsg(id_repetidor_isolado,seta->next->orig,ip_this_node,seta->next->ant,msg);
            }
        }
        seta = lastOne();
        vetorFila[seta->next->orig] = false;
        delList(seta);
        return;
    }
    tentativas_reenvio ++;
    sendMsg(ip_repetidor,seta->next->orig,ip_this_node,ip_repetidor,msg);
}
void tela(){
    Heltec.display->clear();
    // draw the current demo method
    //Serial.println("task1 core " +String(xPortGetCoreID()));
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
}
void estateMachine(){
    int i;
    String msg = "dados";   
    switch (estado) {
        case 0:
            
            //onReceive(LoRa.parsePacket());
            tela(); // atualiza tela
            if(recebidos->next != NULL){ // se tiver msg recebida, tratar
                radio_lora = true;
                trataRecebidos();
            }
            else{
                radio_lora = false;
            }
          break;
        case 1:// enviar msg da fila se houver.
            
            seta = lastOne();
            menorCusto(seta->next->orig, seta-> next->ant);
            espera = true;
            lastTime = millis();
            esperaTime = random(2000) + 20000;
            doing = "enviando msg";
            sendMessage();
            break;
        case 2:// contagem de tentativas
            tentativas_reenvio = 0;
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
            doing= "Preparando Msg.";
            if(vetorFila[ip_this_node] == false){       // verificar depois se o risco de incêndio 
                addList(ip_this_node, ip_this_node);    //aumentou, caso sim alterar valor na fila e aumentar prioridade.
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
    LoRa.receive();
    //LoRa.onReceive(onReceive);
    
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
        buffer[i] = -1;
    }
    for(i = 0 ;i < 5; i++){
        l++;
        Heltec.display->drawLine(49+l, 61, 49+l, 63);
        Heltec.display->display();
        sendMsg(id_new_node,ip_this_node,ip_this_node,ip_broadcast,msg);
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
        
        if(millis()-lastCheck > 30000){
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

