#include <Arduino.h>
#include <DHTesp.h>
#include <SSD1306Wire.h> //display library
#include <ESP8266Wifi.h>

//define transmission pins for display
#define SDA D2
#define SCL D1
#define D_TIME 500

//Instantiate an object from class DHTesp
DHTesp myDht;
float temp, hum;

//Instantiate object for Oled
SSD1306Wire Oled(0x3c, SDA, SCL); //(I2C address of the device, )

// Instantiate an object of the class WiFiServer, listening at port 80
WiFiServer server(80);


//Soft AP setup variables
String mySSID = "learnAutomation", myPASS = "ThisIsPassword123";
IPAddress localIP(192,168,4,2);
IPAddress Gateway(0,0,0,0);
IPAddress SubnetMask(255,255,255,0);
int Chn = 13;

long prevTime;

void setup() {
  Serial.begin(115200);
  //init DHT22 sensor
  myDht.setup(D7, DHTesp::DHT22);

  //init display
  Oled.init();
  Oled.clear();
  Oled.setFont(ArialMT_Plain_16);
  Oled.flipScreenVertically();
  Oled.setTextAlignment(TEXT_ALIGN_LEFT);
  //Oled.drawString(0, 0, "Wonderful");
  //Oled.display();

  //Configure Soft AP
  if (WiFi.softAPConfig(localIP, Gateway, SubnetMask)) Serial.println("Config ok!");
  if (WiFi.softAP(mySSID, myPASS)) {
    Serial.println("AP has been started");
    Serial.println(WiFi.softAPIP().toString());
  }
  else Serial.println("AP failed to start!");

  //after the AP has been configured, init Svr
  server.begin();

}

void loop() {
  //millis used to avoid delay 
  if (prevTime + D_TIME <= millis()){
    //get the data
    //the ADC in DHT22 is about 2.5KHz so we have to delay or wait
    temp = myDht.getTemperature();
    hum = myDht.getHumidity();

    Oled.clear();
    if (temp >= 22) Oled.drawString(0, 0, "It's Hot!  "+(String)temp+" C");
    else if (temp < 22 && temp > 18) Oled.drawString(0, 0, "It's OK!  "+(String)temp+" C");
    else Oled.drawString(0, 0, "Freezing  "+(String)temp+" C");

    Oled.drawString(0, 24, "R.H. is  " + (String)hum+"%");
    Oled.drawString(48, 48, "Thanks!");
    //Oled.drawString(4, 8, "T = "+ (String)temp+" C");
    //Oled.drawString(4, 24, "H = "+ (String)hum +"%");
    Oled.display();
    prevTime = millis();
  }
  //Serial.printf("%s\t%.1f C\t%.1f%%\n", myDht.getStatusString(), temp, hum);
  //delay(5000);

  //Instantiate client object and scan for requests. 
  WiFiClient client = server.available();
  if (client != NULL){
    //if true, print client's IP address
    Serial.println("New Client! IP = "+client.remoteIP().toString());
    //the lines end with \r or \n
    //request ends with empty line \n only
    String currentLine = "", httpReq = "";
    do {
      currentLine = client.readStringUntil('\n');
      Serial.println(currentLine);
      httpReq += currentLine;
    } while (currentLine.length() > 1);

    if (httpReq.substring(0, 3) == "GET") {
      int i1 = httpReq.indexOf("GET ") + 4;
      int i2 = httpReq.indexOf("HTTP/") - 1;
      String res = httpReq.substring(i1, i2);

      if (res.indexOf("index.html") > -1){
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type: text/html");
        client.println("Connection: close");
        client.println();

        client.println("<!DOCTYPE html><html>");
        client.println("<head><meta name=\"DHT22 values\">");
        client.println("</head>"); 

        client.println("<body><h1 style=\"font-size:60px; color=rgb(52, 162, 255)\">Temperature = " + (String)temp + " C <br> Humidity = "+(String)hum + " %</h1>");
        
        client.println("</body>");
      }
      else {
        client.println("HTTP/1.1 Not index.html");
        client.println();
      }
      
    }
    else {
      client.println("HTTP/1.1 No GET request!");
      client.println();
    }
  } 
} 