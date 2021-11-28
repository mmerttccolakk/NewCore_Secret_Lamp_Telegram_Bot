#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager


//default parameters
char bot_token[120]="xxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";//telegram token
char chat_id[20]="xxxxxxxxx"; //authorized user id
char wifi_portal_timout[4]="60"; //!wifi >portal uptime after reconnect to wifi 
char countdown_system[2]="0";//[1]-true or [0]-false  countdown to automatic shutdown for lamp
char countdown_system_timout[4]="300"; //[seconds] countdown system timout for automatic shutdown for lamp

//flag for saving data
bool shouldSaveConfig = false;

void setup() {

  Serial.begin(115200);
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_bot_token("bottoken", "Telegram Bot Token", bot_token, 120);
  WiFiManagerParameter custom_chat_id("chatid", "Telegram UserID", chat_id, 20);
  WiFiManagerParameter custom_text("<p>for advanced settings</p>");
  WiFiManagerParameter custom_wifi_portal_timout("wifiportaltimeout", "Wifi Portal Timeout", wifi_portal_timout, 4);
  WiFiManagerParameter custom_countdown_system_timout("countdown_system_timout", "[sn] countdown system lamp timout for automatic shutdown", countdown_system_timout, 4);
  WiFiManagerParameter custom_countdown_system("countdown_system", "[1 or 0]  countdown system (on or off) for lamp", countdown_system, 2);


  WiFiManager wifiManager;
  //wifiManager.setSaveConfigCallback(saveConfigCallback);
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

}

void loop() {
  // put your main code here, to run repeatedly:

}

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

