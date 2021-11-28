//for telegram bot
#include <esp82_eeprom.h> //https://github.com/mmerttccolakk/ARDUINO-ESP8266-EEPROM-LIBRARY
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>//https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

//pin information 
#define PIN_RELAY D5
const unsigned long BOT_MTBS = 1000; // mean time between scan messages

//default parameters
char bot_token[120]="xxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";//telegram token
char chat_id[20]="xxxxxxxxx"; //authorized user id
char wifi_portal_timout[4]="60"; //!wifi >portal uptime after reconnect to wifi 
char countdown_system_timout[4]="300"; //[seconds] countdown system timout for automatic shutdown for lamp
char countdown_system[4]="1";//[1]-true or [0]-false  countdown to automatic shutdown for lamp

//countdown_system
bool auto_off = true;
// last time messages scan has been done
unsigned long bot_lasttime; 
//flag for saving data
bool shouldSaveConfig = false;

  //create telegram parameters
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
  WiFiClientSecure secured_client;
  UniversalTelegramBot bot(String(bot_token), secured_client);
  
void setup() {

  setup_eeprom(512);//eprom begin
  Serial.begin(115200);//serial begin

  //relay pin output
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);//for information led

  //enable at startup
  digitalWrite(PIN_RELAY, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);

  //reading from eprom
  eprom_read(0).toCharArray(bot_token,120); 
  eprom_read(125).toCharArray(chat_id,20); 
  eprom_read(150).toCharArray(wifi_portal_timout,4);
  eprom_read(160).toCharArray(countdown_system_timout,4);
  eprom_read(170).toCharArray(countdown_system,4);

  bot.updateToken(String(bot_token));
  auto_off=String(bot_token).toInt();

  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_bot_token("bottoken", "Telegram Bot Token", bot_token, 120);
  WiFiManagerParameter custom_chat_id("chatid", "Telegram UserID", chat_id, 20);
  WiFiManagerParameter custom_text("<p>for advanced settings</p>");
  WiFiManagerParameter custom_wifi_portal_timout("wifiportaltimeout", "Wifi Portal Timeout", wifi_portal_timout, 4);
  WiFiManagerParameter custom_countdown_system_timout("countdown_system_timout", "[sn] countdown system lamp timout for automatic shutdown", countdown_system_timout, 4);
  WiFiManagerParameter custom_countdown_system("countdown_system", "[1 or 0]  countdown system (on or off) for lamp", countdown_system, 2);


  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_bot_token);
  wifiManager.addParameter(&custom_chat_id);
  wifiManager.addParameter(&custom_text);
  wifiManager.addParameter(&custom_wifi_portal_timout);
  wifiManager.addParameter(&custom_countdown_system_timout);
  wifiManager.addParameter(&custom_countdown_system);
  wifiManager.setConfigPortalTimeout(String(wifi_portal_timout).toInt());
  wifiManager.setDebugOutput(true);

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org


    if(!wifiManager.autoConnect("Cyber_JET", "12345678")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    } 
  Serial.println("connected..");

  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600){
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);

  //read updated parameters
  strcpy(bot_token, custom_bot_token.getValue());
  strcpy(chat_id, custom_chat_id.getValue());
  strcpy(wifi_portal_timout, custom_wifi_portal_timout.getValue());
  strcpy(countdown_system_timout, custom_countdown_system_timout.getValue());
  strcpy(countdown_system, custom_countdown_system.getValue());

   //save the custom parameters to eeprom
  if (shouldSaveConfig) {
    eprom_write(0,String(bot_token));//begin adress and write string
    eprom_write(125,String(chat_id));
    eprom_write(150,String(wifi_portal_timout));
    eprom_write(160,String(countdown_system_timout));
    eprom_write(170,String(countdown_system));
    Serial.println("saving config");
    shouldSaveConfig = false;
  }
  bot_setup();
}

void loop() {
  if (millis() - bot_lasttime > BOT_MTBS){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }

  if(bot_lasttime>=(String(countdown_system_timout).toInt()*1000) and auto_off==true){

    Serial.println("");
    auto_off = false;
    String answer = "Hide OK- Invisible // The countdown system is complete."; 
    Serial.println(answer);
    bot.sendMessage(String(chat_id), answer, "");
    digitalWrite(PIN_RELAY, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
void handleNewMessages(int numNewMessages){
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String answer;
  for (int i = 0; i < numNewMessages; i++){
    telegramMessage &msg = bot.messages[i];//information from message 
    
    //account authorization check user id from message
    if (String(chat_id) != String(msg.chat_id)){

      bot.sendMessage(String(msg.chat_id), "You are not an authorized user", "");//the user is notified as unauthorized
      bot.sendMessage(String(chat_id), "Unauthorized user detected user id:" + String(msg.chat_id)+" name:"+String(msg.from_name), "");//unauthorized user is reported to administrator
      
      //serial debug
      Serial.println("");
      Serial.print("Unauthorized user detected user id:");
      Serial.print(String(msg.chat_id));
      Serial.print(" name:");
      Serial.println(String(msg.from_name));

    }else{
    
    //serial debug
    Serial.println("Received " + msg.text);

    //answers to telegrams
    if (msg.text == "/info"){
        if(digitalRead(PIN_RELAY)==LOW){
          answer = "Hide OK- Invisible"; 
        }else if(auto_off==true){
          answer = "Show OK- Visible but shutdown remaining seconds " + String(String(countdown_system_timout).toInt()-(millis()/1000)); 
        }else{
          answer = "Show OK- Visible"; 
        }
    }else if(msg.text == "/hide"){
      answer = "Hide OK- Invisible"; 
      digitalWrite(PIN_RELAY, LOW);
      digitalWrite(LED_BUILTIN, LOW);
    }else if (msg.text == "/show"){
      answer = "Show OK- Visible"; 
      digitalWrite(PIN_RELAY, HIGH);
      digitalWrite(LED_BUILTIN, HIGH);
    }else if(msg.text == "/cdown_on"){
      answer = "Countdown system timout is ACTIVE"; 
      auto_off = true;
      char countdown_system[4]="1";
      eprom_write(170,String(countdown_system));
    }else if (msg.text == "/cdown_off"){
      answer = "Countdown system timout is DEACTIVE"; 
      auto_off = false;
      char countdown_system[4]="0";
      eprom_write(170,String(countdown_system));
    }else{
      answer = "I do not understand. What do you say ? *" + msg.from_name + "*";
    }
    bot.sendMessage(msg.chat_id, answer, "Markdown");
    Serial.println(answer);
    }
  }
}
void bot_setup(){
  const String commands = F("["
                            "{\"command\":\"info\",  \"description\":\"Information about working\"},"
                            "{\"command\":\"hide\", \"description\":\"Turn off the lamp\"},"
                            "{\"command\":\"show\",\"description\":\"Turn on the lamp\"}," 
                            "{\"command\":\"cdown_on\",  \"description\":\"Turn on the countdown system\"},"
                            "{\"command\":\"cdown_off\", \"description\":\"Turn off the countdown system\"}"// no comma on last command
                            "]");
  bot.setMyCommands(commands);
  bot.sendMessage(String(chat_id), String("Online! My ip is "+WiFi.localIP().toString()),"Markdown");
}
//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
