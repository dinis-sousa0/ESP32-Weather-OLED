#include "DHT.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <HTTPClient.h>

//---------Screen---------
//Replace with proper constructor for specific screen
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);//->construtor ssd1315 para full buffer

//----------------Som--------------
//https://projecthub.arduino.cc/aslanyen/charge-3000-15f0a0
int Note1=233;  //3Bb   233Hz
int Note2=294; //4D    294Hz
int Note3=330; //4E    330Hz
int Note4=349; //4F    349Hz
int Note5=392; //4G    392Hz
int Note6=440;  //4A    440Hz
int Note7=466; //4Bb   466Hz
int Note8=523; //5C    523Hz
int Note9=587; //5D    587Hz
int Note10=622; //5Eb  622Hz
int Note11=698;  //5F   698Hz
#define SOM 18

void somInicio(){
   int n=450; //rate at  which song progresses
  tone(SOM,  392, n/3);
  delay(n/3);
  delay(70);
  
  tone(SOM, 523, n/3);
  delay(n/3);
  delay(70);
  
  tone(SOM, 659, n/3);
  delay(n/3);
  delay(70);
  
  tone(SOM,  784, n*3/4);
  delay(n*3/4);
  delay(70);
  
  tone(SOM, 659, n/4);
  delay(n/4);
  delay(70);
  
  tone(SOM, 784, n*2);
  delay(n*2);
  delay(70);

  }


//-------------t&h-----------
#define DHTPIN 19     // Digital pin connected to the DHT sensor
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);
unsigned long timerDelay_in = 5000;

//---------button----------
#define LED 2
#define BUTTON_PIN 23 // GPIO23 pin connected to button
int lastState = HIGH; // the previous state from the input pin
int currentState;     // the current reading from the input pin
bool outdoor = true;
bool mudou = true;
// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated
// constants won't change:
const long interval = 1000;  // interval at which to blink (milliseconds)


//---------------WiFi-------------
const char* ssid = "BLANK"; //Fill with your 2.4ghz wifi ssid
const char* password = "BLANK";//and its respective password

//---------------Weather-------------
#define SUN	0
#define CLEARNIGHT 1
#define SUN_CLOUD  2
#define CLOUD 3
#define RAIN 4
#define THUNDER 5
#define SNOW 6
#define MIST 7

uint8_t getCase(const char *icon){
  Serial.print("Icon depois-> ");
  Serial.println(icon);
  if (strcmp(icon, "01d") == 0) return SUN;
  if (strcmp(icon, "01n") == 0) return CLEARNIGHT;
  if (strcmp(icon, "02d") == 0 || strcmp(icon, "02n") == 0) return SUN_CLOUD;
  if (strcmp(icon, "03d") == 0 || strcmp(icon, "03n") == 0 || strcmp(icon, "04d") == 0 || strcmp(icon, "04n") == 0) return CLOUD;
  if (strcmp(icon, "09d") == 0 || strcmp(icon, "09n") == 0 || strcmp(icon, "10d") == 0 || strcmp(icon, "10n") == 0) return RAIN;
  if (strcmp(icon, "11d") == 0 || strcmp(icon, "11n") == 0) return THUNDER;
  if (strcmp(icon, "13d") == 0 || strcmp(icon, "13n") == 0) return SNOW;
  if (strcmp(icon, "50d") == 0 || strcmp(icon, "50n") == 0) return MIST;
  return CLOUD; // Default
}


void drawWeatherSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
  switch(symbol)
  {
    case SUN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 69);	
      break;
    case CLEARNIGHT:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 66);
      break;
    case SUN_CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 65);	
      break;
    case CLOUD:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 64);	
      break;
    case RAIN:
      u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);
      u8g2.drawGlyph(x, y, 67);	
      break;
    case THUNDER:
      u8g2.setFont(u8g2_font_open_iconic_embedded_6x_t);
      u8g2.drawGlyph(x, y, 67);
      break;
    case SNOW:
      u8g2.setFont(u8g2_font_open_iconic_other_6x_t);
      u8g2.drawGlyph(x, y, 67);
      break;
    case MIST:
      u8g2.setFont(u8g2_font_open_iconic_text_6x_t);
      u8g2.drawGlyph(x, y, 66);
      break;
  }
}

void drawWeather(double degree,const char *desc_tempo, const char * icon, const char*local,double vento)
{
  Serial.print("Icon antes-> ");
  Serial.println(icon);
  uint8_t symbol = getCase(icon);
  drawWeatherSymbol(76, 50, symbol);
  u8g2.setFont(u8g2_font_inr16_mf);
  u8g2.setCursor(4, 18);
  char deground[6];
  dtostrf(degree, 2, 0, deground);;  // arredonda 
  u8g2.print(deground);
  u8g2.print("°C");
  u8g2.drawHLine(4,20,56);
  u8g2.drawHLine(4,21,56);
  u8g2.setFont(u8g2_font_helvB08_tf);
  u8g2.setCursor(2,34);
  char venround[6];
  sprintf(venround, "%.1f", vento);  // 1 casa decimal
  u8g2.print(venround);
  u8g2.print(" km/h");
  u8g2.setCursor(2,48);
  char descCopy[30]; //espaco para copiar a string (a original nao e mutavel (read only))
  strncpy(descCopy, desc_tempo, sizeof(descCopy) - 1);
  descCopy[sizeof(descCopy) - 1] = '\0'; //garante fim correto
  descCopy[0] = toupper(descCopy[0]); //agora pode modificar
  u8g2.print(descCopy);
  u8g2.setCursor(2,62);
  u8g2.print(local);
}

void draw(double degree,const char* desc_tempo, const char * icon, const char *local, double vento){
    u8g2.firstPage();
    do {
      drawWeather(degree, desc_tempo, icon,local,vento);
    } while ( u8g2.nextPage() );
}


String API_OWM ="BLANK"; //fill with your openweathermap api key
String localidade = "BLANK"; //fill desired city
String pais= "BLANK"; //and its respective country code, eg US UK etc
String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + localidade + "," + pais + "&APPID=" + API_OWM + "&lang="+ pais + "&units=metric";


unsigned long lastTime = 0;
unsigned long timerDelay_out = 30000;

String jsonBuffer;

String httpGETRequest(const char* serverName) {
  digitalWrite(LED, HIGH);
  WiFiClient client;
  HTTPClient http;
  const int retries_max = 5;
  const int retry_delay = 500;
  String payload = "{}";
  
  for (int attempt = 1; attempt < retries_max + 1; attempt++){
    Serial.print("Tentativa ");
    Serial.print(attempt);
    Serial.println("...");
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverName);
    
    // Send HTTP POST request
    int httpResponseCode = http.GET();
     
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      payload = http.getString();
      break;
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      if (attempt < retries_max) {
        Serial.println("Retrying...");
        delay(retry_delay);} else {
          Serial.println("Error getting API info...");
        }
    }
  }
    // Free resources
  http.end();

  digitalWrite(LED, LOW);
  return payload;
}

//------------------------------------start------------------------------------------------

void setup(void) {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(SOM, OUTPUT);

  //u8g2-SSD1315
  u8g2.begin();
  u8g2.enableUTF8Print();		// enable UTF8 support for the Arduino print() function
  u8g2.setFont(u8g2_font_helvB08_tf);//u8g2_font_ncenB08_tr);
  u8g2.setFontDirection(0);
  Serial.print(u8g2.getDisplayWidth());
  Serial.print("x");
  Serial.println(u8g2.getDisplayHeight());

  digitalWrite(LED, HIGH);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  u8g2.firstPage();
  do {
    u8g2.setCursor(32, 32);
    u8g2.print("Connecting...");
  } while (u8g2.nextPage());
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  digitalWrite(LED, LOW);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.print(ssid);
  Serial.print(" with IP Address: ");
  Serial.println(WiFi.localIP());
  u8g2.firstPage();
  do {
    u8g2.setCursor(32, 22);
    u8g2.print("Connected to");
    u8g2.setCursor(32, 36);
    u8g2.print(ssid);
  } while (u8g2.nextPage());
  
  //print timers
  Serial.print("Outdoor timer set to ");
  Serial.print(timerDelay_out/1000);
  Serial.println(" seconds");
  Serial.print("Indoor timer set to ");
  Serial.print(timerDelay_in/1000);
  Serial.println(" seconds");

  // initialize the pushbutton pin as an pull-up input
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  //dht11
  Serial.println(F("DHT11 initialized"));
  dht.begin();
  somInicio();
}

void loop(void) {
  currentState = digitalRead(BUTTON_PIN);

  if(lastState == LOW && currentState == HIGH){
    Serial.println("Botão pressionado");
    outdoor = !outdoor;
    mudou = true;
    //Serial.println(outdoor);
    delay(25); //debounce
  }
  
  // save the last state
  lastState = currentState;

  if(outdoor==true){
    // Send an HTTP GET request
    if (((millis() - lastTime) > timerDelay_out) || (mudou==true)) {
      if (mudou==true){
        mudou = !mudou;
        //Serial.println(mudou);
      }
      // Check WiFi connection status
      if(WiFi.status()== WL_CONNECTED){
        
        jsonBuffer = httpGETRequest(serverPath.c_str());
        Serial.println(jsonBuffer);
        JSONVar myObject = JSON.parse(jsonBuffer);
    
        // JSON.typeof(jsonVar) can be used to get the type of the var
        //Serial.println(JSON.typeof(myObject));
        if (JSON.typeof(myObject) == "object") {
          Serial.print("JSON object = ");
          Serial.println(myObject);
          Serial.print("Temperature: ");
          Serial.println(myObject["main"]["temp"]);
          Serial.print("Pressure: ");
          Serial.println(myObject["main"]["pressure"]);
          Serial.print("Humidity: ");
          Serial.println(myObject["main"]["humidity"]);
          Serial.print("Wind Speed: ");
          Serial.println(myObject["wind"]["speed"]);

          //print no ecra
          draw(myObject["main"]["temp"],myObject["weather"][0]["description"],myObject["weather"][0]["icon"],myObject["name"],myObject["wind"]["speed"]);
          Serial.println("Done printing on screen");
        } else {
          Serial.println("Parsing input failed!"); //erro a obter info do site
        }

      }
      else {
        Serial.println("WiFi Disconnected");
      }
      lastTime = millis();
    }
  }

  if(outdoor==false){
    if (((millis() - lastTime) > timerDelay_in) || (mudou==true)) {
      if (mudou==true){
        mudou = !mudou;
        //Serial.println(mudou);
      }
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      digitalWrite(LED, HIGH);
      float h = dht.readHumidity();//humdity (percentage)
      float t = dht.readTemperature();//temperature celsius
      //float f = dht.readTemperature(true);//farhenteitneit

      if (isnan(h) || isnan(t)) {
        Serial.println(F("Error reading DHT!"));
        return;
      }

      float hic = dht.computeHeatIndex(t, h, false);//heat index celsius(isFahreheit = false)
      digitalWrite(LED, LOW);
      //float hif = dht.computeHeatIndex(f, h); //farthneint

      Serial.print(F("Humidity: "));
      Serial.print(h);
      Serial.println(F("%"));
      Serial.print(F("Temperature: "));
      Serial.print(t);
      Serial.println(F("°C "));
      Serial.print(F("Feels like "));
      Serial.print(hic);
      Serial.println(F("°C "));
      //Serial.print(f);
      //Serial.print(F("°F  Heat index: "));
      //Serial.print(hif);
      //Serial.println(F("°F"));

      u8g2.firstPage();
      do {
        u8g2.setCursor(0, 15);
        u8g2.print("Temp: ");
        u8g2.print(t);
        u8g2.print(" ºC");
        u8g2.setCursor(0, 35);
        u8g2.print("Feels Like: ");
        u8g2.print(hic);
        u8g2.print(" ºC");
        u8g2.setCursor(0, 55);
        u8g2.print("Hum: ");
        u8g2.print(h);
        u8g2.print(" %");
      } while ( u8g2.nextPage() );

      lastTime = millis();
    }
  }

}