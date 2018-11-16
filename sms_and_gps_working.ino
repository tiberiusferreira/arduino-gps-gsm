/*
GPRS Connect TCP

This sketch is used to test seeeduino GPRS_Shield's send SMS func.
to make it work, you should insert SIM card to Seeeduino GPRS 
and replace the phoneNumber,enjoy it!

create on 2015/05/14, version: 1.0
by lawliet.zou(lawliet.zou@gmail.com)
*/
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>

#include <TimeLib.h>
#include <TinyGPS++.h>


#define PIN_TX    10
#define PIN_RX    11
#define BAUDRATE  9600
#define PHONE_NUMBER "04119981964310"

#define BUTTON_PIN  8


//Pinos utilizados para conexao do modulo GY-NEO6MV2
static const int RXPin = 5, TXPin = 6;



//Objeto TinyGPS++
TinyGPSPlus gps;

//Conexao serial do modulo GPS
SoftwareSerial Serial_GPS(RXPin, TXPin);

const int UTC_offset = -3;

GPRS gprs(PIN_TX,PIN_RX,BAUDRATE);//RX,TX,BaudRate





void reinit_gsm(){
    while(true){
      int error_count = 0;  
      int max_error = 20; 
      gprs.listen();
      while(!gprs.init() && error_count < max_error) {
          Serial.println("Initialization failed, retrying in 1 sec");
          delay(1000);
          error_count = error_count+1;
      }  
    
      while(!gprs.isNetworkRegistered() && error_count < max_error)
      {
        Serial.println("Network has not registered yet! Retrying in 5 s");
        delay(5000);
        error_count = error_count+1;
      }
    
      if (error_count < max_error){
        Serial.println("gprs initialize done! Waiting 40s for network to stabilize");
        delay(40000);
        print_signal_str();
        Serial.println("Initialization Done");
        return; // return
      }else{
        Serial.println("Too many errors, rebooting GSM");
        reboot();
      }
    }
}


void print_signal_str(){
  gprs.listen();
  int signal_strength = 0;
  gprs.getSignalStrength(&signal_strength);
  Serial.print("Signal at ");
  Serial.print(signal_strength);
  Serial.println(" !");
}
void reboot(){
  digitalWrite(9, LOW);
  delay(1000);
  digitalWrite(9, HIGH);
}

void setup() {
  pinMode(9, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(9, HIGH);
  Serial.begin(9600); 
  Serial_GPS.begin(9600);

  reinit_gsm();

  Serial.println(TinyGPSPlus::libraryVersion());
}

void loop() {
    Serial_GPS.listen();
    
    if(digitalRead(BUTTON_PIN)==HIGH){
      Serial.println("Pressed!");
      print_signal_str();
      Serial_GPS.listen();
      while(true){
        if (Serial_GPS.available() > 0 && gps.encode(Serial_GPS.read()))
            if (gps.location.isValid())
                {
                  String text_to_send = String("Teste do Botao do Panico. Estou aqui: http://www.google.com/maps/place/");
                  String latitude = String(gps.location.lat(),7);
                  String longitude = String(gps.location.lng(),7);
                  String text_to_send_lat = String(text_to_send+latitude+","+longitude);
                  Serial.println(text_to_send_lat); //latitude
                  gprs.listen();
                  print_signal_str();
                  int number_err = 0;
                  while(true){
                    char str_array[text_to_send_lat.length()];
                    text_to_send_lat.toCharArray(str_array,text_to_send_lat.length());
                    if(gprs.sendSMS(PHONE_NUMBER, str_array)) //define phone number and text
                    {
                      Serial.print("Send SMS Succeed!\r\n");
                      break;
                    }
                    else {
                      Serial.print("Send SMS failed!\r\n");
                      delay(5000);
                      print_signal_str();
                      delay(5000);
                      number_err = number_err + 1;
                      if (number_err > 10){
                        Serial.print("Too many errors rebooting GSM!\r\n");
                        reinit_gsm();
                        number_err = 0;
                      }
                    }
                  }
                  Serial_GPS.listen();
                  break;
                }
                else
                {
                  Serial.println(F("INVALID GPS DATA"));
                  delay(1000);
                }
      }
    }

}


