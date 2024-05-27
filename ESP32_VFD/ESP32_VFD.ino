#include <ArduinoJson.h>
#include <FutabaVFD162S.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFi.h>
// #include <TFT_eSPI.h>
#include <Timezone.h>
#include <time.h>
#include <TimeLib.h>
//#include <ArduinoJSON.h>
#include <WiFi.h>
#include <HTTPClient.h>



#define TZ_OFFSET 0
#define NTP_INTERVAL 60000
#define NTP_SVR "0.us.pool.ntp.org"

String apik = "";
String latitude = "";
String longitude = "";

String jsonbuffer = "";

String weather = "Invalid";
float tmptr = -99;

char tmpchar[4];
char tmpcharn[5];
char tmpchars[3];

const char* ssid = "";
const char* passwd = "";
const String days[8] = {"", "Sun","Mon","Tue","Wed","Thu","Fri","Sat"};\
String output = "";
int tmphour = 0;
// US Eastern Time Zone (New York, Detroit)
TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -240};  // Eastern Daylight Time = UTC - 4 hours
TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -300};   // Eastern Standard Time = UTC - 5 hours
Timezone usET(usEDT, usEST);

WiFiUDP ntpudp;
NTPClient ntpc(ntpudp, NTP_SVR, TZ_OFFSET, NTP_INTERVAL);
FutabaVFD162S vfd(27, 26, 25);

byte openHeart[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};

byte duC[8] = {
  0b00011,
  0b00011,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

// byte duC[8] = {
//   0b10101,
//   0b01110,
//   0b10101,
//   0b01110,
//   0b10101,
//   0b01110,
//   0b10101,

// };

void getCurrWeather(){
  StaticJsonDocument<200> weatherinfo;
  // DeserializationError error = deserializeJson(weatherinfo, jsonbuffer.c_str());
  String url = "http://api.openweathermap.org/data/2.5/weather?lat=" + latitude + "&lon=" + longitude + "&appid=" + apik;
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url.c_str());
  int httpresp = http.GET();
  String pld;
  if (httpresp > 0){
    Serial.println("Got weather info!");
    // Serial.println(http.getString());
    pld = http.getString();
    jsonbuffer = pld.c_str();
    // Serial.println("Stuck below");
  }
  else{
    Serial.println("Failed to get weather");
    jsonbuffer = "";
  }
  http.end();
  if (jsonbuffer == ""){
    weather = "Invalid";
    tmptr = -99;
  }
  else {
      deserializeJson(weatherinfo, jsonbuffer);
//    if (JSON.typeof(weatherinfo) == "undefined"){
//      Serial.println("Failed to parse weather info");
//      weather = "Invalid";
//      tmptr = -99;
//    }
//    else{
      const char * tmpw = weatherinfo["weather"]["main"];
      weather = tmpw;
      tmptr = weatherinfo["main"]["temp"];
      tmptr -= 273.15;
      // tmptr = -50;
//    }
  }
  return;
};

void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  vfd.begin(16, 2);
  vfd.setBrightness(200);
  vfd.clear();
  vfd.setCursor(0,0);
  vfd.print("WiFi Connecting");
  WiFi.begin(ssid, passwd);
  while (WiFi.status() != WL_CONNECTED){
    delay(1000);
    vfd.setCursor(0,1);
    vfd.print(".");
  }
  vfd.clear();
  vfd.setCursor(0,0);
  vfd.println("WiFi Connected");
  delay(1000);
  vfd.clear();
  vfd.setCursor(0,0);
  vfd.print("IP Address:");
  vfd.setCursor(0,1);
  vfd.println(WiFi.localIP());
  delay(2000);
  ntpc.begin();
  vfd.clear();

  ntpc.update();
  setTime(usET.toLocal(ntpc.getEpochTime()));
  getCurrWeather();
  vfd.createChar(1, duC);
//  String output = "";
//  ntpc.update();
//  String time = ntpc.getFormattedTime();
//  int wkd = ntpc.getDay();
//  long milsec = ntpc.getEpochTime()-TZ_OFFSET;
//  yr = milsec/YEAR+1970;
//  mon = (milsec%YEAR)/MONTH+1;
//  dy = ((milsec%YEAR)%MONTH)/DAY+1;
//  output += yr;
//  output += '/';
//  output += mon/10;
//  output += mon%10;
//  output += '/';
//  output += dy/10;
//  output += dy%10;
//  output += "   ";
//  output += days[wkd];
//  output += "    ";
//  output += time;
//  vfd.setCursor(0,0);
//  vfd.print(output);
}

void loop() {
  // put your main code here, to run repeatedly:
  output = "";
  if (tmphour != hour()){
    ntpc.update();
    setTime(usET.toLocal(ntpc.getEpochTime()));
    getCurrWeather();
  }
  tmphour = hour();

  Serial.println(weather);
  Serial.println(tmptr);
  
  //Serial.println(hour());
  if(ntpc.getHours() < 8){
    vfd.setBrightness(50);
  }
  else{
    vfd.setBrightness(150);
  }
  output += year();
  output += '/';
  output += month()/10;
  output += month()%10;
  output += '/';
  output += day()/10;
  output += day()%10;
  output += "   ";
  output += days[weekday()];
//  output += "    ";
  output += hour()/10;
  output += hour()%10;
  output += ":";
  output += minute()/10;
  output += minute()%10;
  output += ":";
  output += second()/10;
  output += second()%10;

  if (tmptr >= 10){
    output += "  ";
    dtostrf(tmptr, 4, 1,tmpchar);
    output += tmpchar;
  }
  else if (tmptr >= 0){
    output += "   ";
    dtostrf(tmptr, 3, 1,tmpchars);
    output += tmpchars;
  }
  else if (tmptr > -10){
    output += "  ";
    dtostrf(tmptr, 4, 1,tmpchar);
    output += tmpchar;    
  }
  else{
    output += " ";
    dtostrf(tmptr, 5, 1,tmpcharn);
    output += tmpcharn;
  }
  output += (char)1;
  output += "C";

  vfd.setCursor(0,0);
  vfd.print(output);
  // vfd.setCursor(14, 1);
  // vfd.write((char)1);
  delay(1000);
}
