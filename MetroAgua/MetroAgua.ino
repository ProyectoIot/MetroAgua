#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif
 


//To use send Email for Gmail to port 465 (SSL), less secure app option should be enabled. https://myaccount.google.com/lesssecureapps?pli=1

#include <Arduino.h>
#include <ESP_Mail_Client.h>


#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
//#include <ArduinoJson.h>
#include <ArduinoJson.h>

// Servidor NTP
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

// Sensor de caudal YF-S201
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
float caudalMax=0;
float caudalMin=0;
long t0 = 0;

// Replace with your network credentials
const char* ssid = "TP-LINK_F90A14";
const char* password = "11135999";

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for Outlook or smtp.mail.yahoo.com
 * For yahoo mail, log in to your yahoo mail in web browser and generate app password by go to
 * https://login.yahoo.com/account/security/app-passwords/add/confirm?src=noSrc
 * and use the app password as password with your yahoo mail account to login.
 * The google app password signin is also available https://support.google.com/mail/answer/185833?hl=en
*/
#define SMTP_HOST "smtp.gmail.com"

/** The smtp port e.g. 
 * 25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
*/
#define SMTP_PORT 465

/* The log in credentials */
#define AUTHOR_EMAIL "proy.iot2021@gmail.com"
#define AUTHOR_PASSWORD "Iot.2021"

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);




// Initialize Telegram BOT
#define BOTtoken "1985473034:AAE-dAJ42hyDpjGNy3FuYlJ37_BH6nBOYHw"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "-560789110"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Checks for new messages every 2 second.
int botRequestDelay = 2000;
unsigned long lastTimeBotRan;

// valores de los correos

String codigo="";
String tituloCorreo="Resumen del Sistema AguaIot";
String direccionDestino="proy.iot2021@gmail.com";
String nickDestino="Admin";
String resultadoCorreo="";

float caudalAlarma=20.0;
float volumenAlarma=1000;
int conf=0;


String chat_id="";

unsigned long epochTime;
String formattedTime;
int currentHour;
int currentMinute;
int currentSecond;
String weekDay;
int monthDay;
int currentMonth;
String currentMonthName;
String currentYear;
String currentDate;

String caudalMax_Date;
String caudalMax_Time;


void enviarMensaje()
{

  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  
  ESP_Mail_Session session ;

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "mydomain.net";

/* Declare the message class */
  SMTP_Message message;

  
  

  /* Set the message headers */
  message.sender.name = "Sistema Agua-IOT";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = tituloCorreo.c_str();
  message.addRecipient(nickDestino.c_str(), direccionDestino.c_str());
int valor = 10;

    //String codigo="";
    
   message.html.content = codigo.c_str();
  //message.html.content = "<p>This is the "<span style=\"color:#ff0000;\">html text</span> message.</p><p>The message was sent via ESP device.</p>"; 
  

  /** The html text message character set e.g.
   * us-ascii
   * utf-8
   * utf-7
   * The default value is utf-8
  */
  message.html.charSet = "us-ascii";

  /** The content transfer encoding e.g.
   * enc_7bit or "7bit" (not encoded)
   * enc_qp or "quoted-printable" (encoded)
   * enc_base64 or "base64" (encoded)
   * enc_binary or "binary" (not encoded)
   * enc_8bit or "8bit" (not encoded)
   * The default value is "7bit"
  */
  message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /** The message priority
   * esp_mail_smtp_priority_high or 1
   * esp_mail_smtp_priority_normal or 3
   * esp_mail_smtp_priority_low or 5
   * The default value is esp_mail_smtp_priority_low
  */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  /** The Delivery Status Notifications e.g.
   * esp_mail_smtp_notify_never
   * esp_mail_smtp_notify_success
   * esp_mail_smtp_notify_failure
   * esp_mail_smtp_notify_delay
   * The default value is esp_mail_smtp_notify_never
  */
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  message.addHeader("Message-ID: <proy.iot2021@gmail.com>");
  

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}


void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    chat_id = String(bot.messages[i].chat_id);
    Serial.println(chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message

    //Serial.println(text);
    if (bot.messages[i].type==F("callback_query") && conf==0) { 
          String text1 = bot.messages[i].text;
    if (text1==F("Caudal")){
      bot.sendMessage(chat_id,"El caudal es de"+ String(flow_Lmin, 3)+ " L/min con un caudal maximo de "+ (String)(caudalMax)+" L/min ocurrido en "+caudalMax_Date+" "+caudalMax_Time+" .", "");
      }
      else if (text1==F("Consumo")){
       bot.sendMessage(chat_id, "El volumen es de  "+ String(volumen,3)+ " Litros. ", ""); 
        }
    }
    String from_name = bot.messages[i].from_name;
    String text = bot.messages[i].text;
    if (text == "/caudal" && conf==0) {
         
      bot.sendMessage(chat_id,"El caudal es de"+ String(flow_Lmin, 3)+ " L/min con un caudal maximo de "+ (String)(caudalMax)+" L/min ocurrido en "+caudalMax_Date+" "+caudalMax_Time+" .", "");
    }
    
    if (text == "/consumo" && conf==0) {
         
      bot.sendMessage(chat_id, "El volumen es "+ String(volumen,3)+ " litros ", "");
      Serial.println ("Consumo");
    }

    if (text == "/pantalla" && conf==0) {
  // usar botones en la pantalla , se le pueden poner cosas (pacotilla)
      String keyboardJson=F("[[ { \"text\": \"Caudal\",\"color\": \"green\", \"callback_data\" : \"Caudal\"}], [ { \"text\": \"Consumo\", \"callback_data\" : \"Consumo\"}]]");
      bot.sendMessageWithInlineKeyboard(chat_id,"Botones en pantalla", "", keyboardJson);
    }
if (text == "/teclado" && conf==0) {

String keyboardJson = "[[\"/caudal\", \"/consumo\"]]"; // usar botones con los nombres de las funciones
      bot.sendMessageWithReplyKeyboard(chat_id, "Botones en teclado", "", keyboardJson, true);
      Serial.println ("botones");
    }
    if (text == "/informeCorreo" && conf==0)
    {
      darHora();
      codigo="<p>Resumen de Sistema Agua-IOT </p>";
      codigo+="<p>El caudal es de  " + String (flow_Lmin,3)+" L/min con un caudal maximo de "+ (String)(caudalMax)+" L/min ocurrido en "+caudalMax_Date+" "+caudalMax_Time+" .</p>";
      codigo+="<p>El volumen es de "+String (volumen,3)+" Litros.</p>";
      codigo+="<p> Este mensaje es enviado con fecha "+currentDate + " a las "+ formattedTime+" via NodeMCU por el Sistema Agua-IOT.</p>"; 
      //tituloCorreo= "Datos de caudal y consumo de agua. ";
      //nickDestino= "Usuario_1";
      //direccionDestino="proy.iot2021@gmail.com";
      enviarMensaje();
      if (resultadoCorreo=="success"){
        bot.sendMessage(chat_id, "Correo enviado correctamente", "Markdown");
        }
        if (resultadoCorreo=="failed"){
        bot.sendMessage(chat_id, "Error en envio de correo", "Markdown");
        }
    }

    if (text == "/confAlarma" && conf==0)
    {
     bot.sendMessage(chat_id, "Cambio alarmas con C-valor y V-valor. Para salir del modo configuracion alarma escriba X", "Markdown"); 
     conf=1;
    }
    if (text.substring(0,3)== "/C-" &&  conf==1)
    {
       int pos =text.indexOf("-");
     //bot.sendMessage(chat_id, (String)(pos), "Markdown"); 
     bot.sendMessage(chat_id, "Alarma de caudal: "+text.substring(pos+1)+" L/min", "Markdown");
     String valor1= text.substring(pos+1);
     if (valor1=="off"){
      caudalAlarma=20;
      }
      else{
     caudalAlarma=valor1.toFloat();
      }
    }
    
    if (text.substring(0,3)== "/V-" && conf==1)
    {
       int pos =text.indexOf("-");
     //bot.sendMessage(chat_id, (String)(pos), "Markdown"); 
     bot.sendMessage(chat_id, "Alarma de volumen: "+text.substring(pos+1)+" Litros", "Markdown");
     String valor= text.substring(pos+1);
     
     if (valor=="off"){
      volumenAlarma=100;
     }
     else {
      volumenAlarma=valor.toFloat();
      }
    }
    if (text == "/X" && (conf==1 || conf==2))
     {
      if (conf==1)
      {
        conf=0;
      bot.sendMessage(chat_id, "Ha salido del modo configuracion de alarma", "Markdown"); 
      }
      else
      {
        conf=0;
      bot.sendMessage(chat_id, "Ha salido del modo configuracion de correo", "Markdown");
      }
      
     }
     if (text == "/alarmas" && conf==0)
     {
      
      bot.sendMessage(chat_id, "La alarma de caudal esta en "+(String)(caudalAlarma)+ " L/min. \n"+"La alarma de volumen esta en "+(String)(volumenAlarma)+ " Litros", "Markdown"); 
            
     }
      if (text == "/correo" && conf==0)
     {
      
      bot.sendMessage(chat_id, "Direccion de correo "+ direccionDestino, "Markdown"); 
      bot.sendMessage(chat_id, "Nick del correo "+nickDestino, "Markdown"); 
      bot.sendMessage(chat_id, "Titulo del correo "+ tituloCorreo, "Markdown"); 
      
     }

     if (text == "/confCorreo" && conf==0)
     {
      conf=2;
      bot.sendMessage(chat_id, "Ha entrado en el modo Configuracion correo. ", "Markdown"); 
      bot.sendMessage(chat_id, "Para cambiar el correo pongalo de la siguiente forma I-usuario@gmail.com.\n Para cambiar el nick ponga NI-. \n Para cambiar el titulo del correo ponga TI-. \n Para cambiar el Para salir del modo configuracion correo escriba X", "Markdown"); 
      
     }
     if (text.substring(0,3)== "/I-" && conf==2)
     {
      int pos1 =text.indexOf("-");
     String valor1= text.substring(pos1+1); 
     if (valor1.indexOf("@")>0)
     {
      direccionDestino=valor1;
     bot.sendMessage(chat_id, "Nuevo correo: "+valor1, "Markdown");
     }
     else 
     {
      bot.sendMessage(chat_id, "Correo incorrecto", "Markdown");
     }
     }
     if (text.substring(0,4)== "/NI-" && conf==2)
     {
      int pos2 =text.indexOf("-");
     nickDestino=text.substring(pos2+1);;
     bot.sendMessage(chat_id, "Nuevo nick del correo: "+nickDestino, "Markdown");
     }

     if (text.substring(0,4)== "/TI-" && conf==2)
     {
      int pos3 =text.indexOf("-");
     tituloCorreo=text.substring(pos3+1);;
     bot.sendMessage(chat_id, "Nuevo titulo del correo: "+tituloCorreo, "Markdown");
     }

     if (text == "/start" && conf==0)
    {
      String welcome = "Sistema de medicion de parametros de agua, " + from_name + ".\n";
      welcome += "\n";
      welcome += "/caudal : L/min\n";
      welcome += "/consumo : Litros\n";
      welcome += "/pantalla : Botones en pantalla\n";  
      welcome += "/teclado : Botones en teclado\n";
      welcome += "/informeCorreo : Enviar correo  con resumen\n";
      welcome += "/alarmas : Valores de las alarmas\n";
      welcome += "/correo : Correo a enviar informe\n";
      welcome += "/confAlarma : Configurar alarmas\n";
      welcome += "/confCorreo : Configurar correo";
      
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
    

  }
}
 void darHora()
 {
  timeClient.update();

  epochTime = timeClient.getEpochTime();
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  

  weekDay = weekDays[timeClient.getDay()];
  Serial.print("Week Day: ");
  Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  monthDay = ptm->tm_mday;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  currentMonthName = months[currentMonth-1];
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  currentYear = ptm->tm_year+1900;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

  Serial.println("");
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

 // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(-14400); // -4=-14400 horario de verano -5=18000 horario normal
  darHora(); 
}

void loop()
{


  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

 if (caudalAlarma<=flow_Lmin){
    darHora();
    bot.sendMessage(chat_id, currentDate+"  "+formattedTime+"\n"+"Alarma  de Caudal con valor de "+(String)(flow_Lmin)+" L/min", "");
    
    }
  if (volumenAlarma<=volumen){
    darHora();
    bot.sendMessage(chat_id, currentDate+"  "+formattedTime+"\n"+"Alarma  de Volumen con valor de "+(String)(volumen)+" Litros", "");
    
    }
  
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

  if (flow_Lmin>caudalMax) // Calculo del caudal max 
  {
    darHora();
    caudalMax_Date=currentDate;
    caudalMax_Time=formattedTime;
    caudalMax=flow_Lmin;
  }

}

void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failled: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);
      resultadoCorreo= result.completed ? "success" : "failed";

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}
