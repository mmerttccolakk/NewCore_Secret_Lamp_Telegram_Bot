//for telegram bot
#include <esp82_eeprom.h> //https://github.com/mmerttccolakk/ARDUINO-ESP8266-EEPROM-LIBRARY
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>//https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

const unsigned long BOT_MTBS = 200; // mean time between scan messages

//default parameters
char bot_token[120]="";//telegram token
char chat_id[20]="xxxxxxxxx"; //authorized user id
char wifi_portal_timout[4]="60"; //!wifi >portal uptime after reconnect to wifi 
char countdown_system_timout[4]="300"; //[seconds] countdown system timout for automatic shutdown for lamp
char countdown_system[2]="0";//[1]-true or [0]-false  countdown to automatic shutdown for lamp

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

  //reading from eprom
  eprom_read(0).toCharArray(bot_token,120); 
  eprom_read(125).toCharArray(chat_id,20); 
  eprom_read(150).toCharArray(wifi_portal_timout,4);
  eprom_read(160).toCharArray(countdown_system_timout,4);
  eprom_read(170).toCharArray(countdown_system,4);

  bot.updateToken(String(bot_token));

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
  if (millis() - bot_lasttime > BOT_MTBS)
{
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
  }

}
void handleNewMessages(int numNewMessages){
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String answer;
  for (int i = 0; i < numNewMessages; i++){
    telegramMessage &msg = bot.messages[i];
    Serial.println("Received " + msg.text);
    if (msg.text == "/help")
      answer = "So you need _help_, uh? me too! use /start or /status";
    else if (msg.text == "/start")
      answer = "Welcome my new friend! You are the first *" + msg.from_name + "* I've ever met";
    else if (msg.text == "/status")
      answer = "All is good here, thanks for asking!";
    else
      answer = "Say what?";

    bot.sendMessage(msg.chat_id, answer, "Markdown");
  }
}
void bot_setup(){
  const String commands = F("["
                            "{\"command\":\"start\",  \"description\":\"Information about working\"},"
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
