

const int sensorPin = D5; // d5
const int measureInterval = 1000;
volatile int pulseConter;

// YF-S201
const float factorK = 7.5;

// FS300A
//const float factorK = 5.5;

// FS400A
//const float factorK = 3.5;

float volumen = 0;
float flow_Lmin =0;
long t0 = 0;

#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
//#include <ArduinoJson.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "TP-LINK_F90A14";
const char* password = "11135999";

// Initialize Telegram BOT
#define BOTtoken "1985473034:AAE-dAJ42hyDpjGNy3FuYlJ37_BH6nBOYHw"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "-560789110"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 1 second.
int botRequestDelay = 5000;
unsigned long lastTimeBotRan;

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    Serial.println(chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/caudal") {
     
      
      bot.sendMessage(chat_id,"El caudal es "+ String(flow_Lmin, 3)+ " L/min", "");
      Serial.println ("Caudal");
    }
    
    if (text == "/consumo") {
   

      
      bot.sendMessage(chat_id, "El volumen es "+ String(volumen,3)+ " litros ", "");
      Serial.println ("Consumo");
    }

     if (text == "/start")
    {
      String welcome = "Welcome to Universal Arduino Telegram Bot library, " + from_name + ".\n";
      welcome += "This is Flash Led Bot example.\n\n";
      welcome += "/caudal : Lmin\n";
      welcome += "/consumo : Litros\n";
      
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    
//    if (text == "/state") {
//      if (digitalRead(ledPin)){
//        bot.sendMessage(chat_id, "LED is ON", "");
//      }
//      else{
//        bot.sendMessage(chat_id, "LED is OFF", "");
//      }
//    }
  }
}

 ICACHE_RAM_ATTR void ISRCountPulse()
{
  pulseConter++;
}

float GetFrequency()
{
  pulseConter = 0;

  interrupts();
  delay(measureInterval);
  noInterrupts();

  return (float)pulseConter * 1000 / measureInterval;
}

void SumVolume(float dV)
{
  volumen += dV / 60 * (millis() - t0) / 1000.0;
  t0 = millis();
}

void setup()
{

  Serial.begin(9600);
 #ifdef ESP8266
    client.setInsecure();
  #endif

WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot Started", "");

  
  
 attachInterrupt(digitalPinToInterrupt(sensorPin), ISRCountPulse, RISING);
  t0 = millis();
}

void loop()
{

  // obtener frecuencia en Hz

  
 
  
//
//  Serial.print(" Caudal: ");
//  Serial.print(flow_Lmin, 3);
//  Serial.print(" (L/min)\tConsumo:");
//  Serial.print(volumen, 1);
//  Serial.println(" (L)");

  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  
    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
}
   
 float frequency = GetFrequency();

  // calcular caudal L/min
  flow_Lmin = frequency / factorK;
  SumVolume(flow_Lmin);
}
