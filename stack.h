typedef struct Node{
    byte orig, ant;
    struct Node *next;
}node;

typedef struct Rec{
    byte id, origem, anterior, destino;
    char msg[20];
    struct Rec *next;
}rec_node;
// id do Algoritmo


node *list,*seta,*aux1,*aux2;
rec_node  *recebidos, *setaRec;

node *lastOne(node *list){
    node *cur = list;
    node *aux;

    while(cur->next != NULL){
        aux = cur;
        cur = cur->next;
    }
    return aux;
}
rec_node *lastRecOne(rec_node *recebidos){
    rec_node *cur = recebidos;
    rec_node *aux;

    while(cur->next != NULL){
        aux = cur;
        cur = cur->next;
    }
    return aux;
}
void addRecList(byte id, int origem, byte anterior, byte destino, char msg[20]){

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
    // Serial.println("msg Add");
}
void addList( byte origem, byte anterior, node *list ){
    node *cur=list;
    node *newN=(node *) malloc(sizeof(node)); // cria novo nó.
    node *aux= list->next;

    newN->orig = origem;
    newN->ant = anterior;
    newN->next = aux; 
    list->next = newN;
    
    return; // retorna lista atualizada.
}
void addFila( byte origem, byte anterior, int ip_this_node,node *list){
    int i;
    //verificar se o pacote não entrará em loop
    //Verifica se tem loop.

    // gateway não enfileira nada, apenas as msgs que recebe.
    if(ip_this_node == 0){
        addList(origem, anterior, list);
        return;
    }
    // dados para replicar
    //verifica se essa msg já passou aqui
    // add a fila de espera.
    addList(origem, anterior, list);
    // responder ao anterior.
    //sendMsg(id_resposta,origem,ip_this_node,anterior,msg);
    // guardar para função de envio.
    /*
        //mensagem recebida deve ser encaminhada
        
    */
    return;
}
void delList (node *cur){
    int i;
    if(cur == NULL) {// caso não seja encontrado, a respota da função anterior será null, nesse caso não é necessário deletar nada.
        return;
    }
    //printf("%s\n",cur->title);
    node *file_before = cur;  // caso seja encontrado o elemento, um ponteiro auxiliar o salva enquando o elemento anterior a ele é atualizado para
    node *file_search_for = cur->next; // apontar para o elemento posterior ao elemento salvo.
    cur = file_search_for->next;//cursor vai para fila após a procurada.
    file_before->next = cur;//file anterior à search_for aponta para a música após ela. 
    free(file_search_for);
}
void delRecList (Rec *cur){
    int i;
    //portENTER_CRITICAL_ISR(&mux);
    if(cur == NULL) // caso não seja encontrado, a respota da função anterior será null, nesse caso não é necessário deletar nada.
        return;
    //printf("%s\n",cur->title);
    rec_node *file_before = cur;  // caso seja encontrado o elemento, um ponteiro auxiliar o salva enquando o elemento anterior a ele é atualizado para
    rec_node *file_search_for = cur->next; // apontar para o elemento posteior ao elemento salvo.
    cur=file_search_for->next;//cur vai para file após a procurada.
    file_before->next = cur;//file anterior à search_for aponta para a música após ela. 
    free(file_search_for);
    //doing = "msg deletada";
    //portEXIT_CRITICAL_ISR(&mux);
}
