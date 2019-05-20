#include "imagens/bluetooth_img.h"
#include "imagens/wifi_img.h"
#include "imagens/radio_img.h"
#include "imagens/bateria_img.h"
#include "imagens/curupira.h"
#include "imagens/ballooncat.h"


void showNos(int linhas, int tabela[15][3], String doing){
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
void drawFontFaceDemo(int ip_repetidor,byte ip_gateway, byte ip_this_node, int nr_vizinhos, int tam_fila, String doing, int orig, int ant) {
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
        Heltec.display->drawString(0,30, "   last rec Orig "+ String(orig)+" ate "+String(ant));
    }
    else{
        Heltec.display->drawString(0,30, "   no rep   "+ repetidor);
    }
    Heltec.display->drawString(0,40, "   nº viz  "+String(nr_vizinhos)+"  tª fila "+String(tam_fila));
    Heltec.display->drawString(0,54, ">"+doing);
}
void drawImage(int bateria, bool radio_lora, bool wifi, bool bluetooth) {
   
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
int tela(int timeDraw,int ip_repetidor,byte ip_gateway, byte ip_this_node, int nr_vizinhos, int tam_fila, int linhas, int tabela[15][3], String doing,bool radio_lora, bool wifi, bool bluetooth, int orig, int ant){
    Heltec.display->clear();
    // draw the current demo method
    //Serial.println("task1 core " +String(xPortGetCoreID()));
    if(millis() - timeDraw < 10000){
        drawFontFaceDemo( ip_repetidor, ip_gateway, ip_this_node, nr_vizinhos, tam_fila, doing, orig, ant);
        drawImage(0, radio_lora, wifi, bluetooth);
    }
    else if(millis() - timeDraw > 10000){
        showNos( linhas, tabela, doing);
    }
    if(millis() - timeDraw > 20000){
        timeDraw = millis();
    }
    Heltec.display->display();
    return timeDraw;
}

