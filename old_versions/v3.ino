#include "esp82_eeprom.h" //https://github.com/mmerttccolakk/ARDUINO-ESP8266-EEPROM-LIBRARY
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager


//default parameters
char bot_token[120]="xxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";//telegram token
char chat_id[20]="xxxxxxxxx"; //authorized user id
char wifi_portal_timout[4]="60"; //!wifi >portal uptime after reconnect to wifi 
char countdown_system_timout[4]="300"; //[seconds] countdown system timout for automatic shutdown for lamp
char countdown_system[2]="0";//[1]-true or [0]-false  countdown to automatic shutdown for lamp


//flag for saving data
bool shouldSaveConfig = false;

void setup() {


  setup_eeprom(512);//eprom begin
  Serial.begin(115200);//serial begin

  //reading from eprom
  eprom_read(100).toCharArray(bot_token,120); 
  eprom_read(125).toCharArray(chat_id,20); 
  eprom_read(150).toCharArray(wifi_portal_timout,4);
  eprom_read(160).toCharArray(countdown_system_timout,4);
  eprom_read(170).toCharArray(countdown_system,4);

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

  //wifiManager.setConfigPortalTimeout(WifiPortalTimeout);
  wifiManager.setDebugOutput(true);
    if(!wifiManager.autoConnect("Cyber_JET", "12345678")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      ESP.restart();
      delay(5000);
    } 
  Serial.println("connected..");


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
}

void loop() {
  // put your main code here, to run repeatedly:

}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}