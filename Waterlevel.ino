// Library no Internet
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <WiFiClientSecure.h>

// Define pinout
#define parkir1 D0
#define parkir2 D1
#define masuk D2

// Start servo setup
Servo servo;

// MQTT Apps
const char* wifiSSID = "No Internet";
const char* wifiPass = "B1711UJI";

const char* brokerUser = "fadlyrizqy"; //ISI /praktisimengajar2023:kelompokB1
const char* brokerPass = "123456789"; //ISI 1234567890
const char* brokerHost = "0a1f1972a79646b6aa2499c7ed716268.s2.eu.hivemq.cloud"; //ISI rmq2.pptik.id 
const char* brokerID = ""; //ISI clientID

const char* topic = "deteksi"; //Deteksi jarak sensor dalam cm

//variabel untuk kirim pesan mqtt
unsigned long previousMillis = 0;
const long interval = 5000; // 5 detik loop
String msgStr = "";      // MQTT message buffer

int pos = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi()
{
  delay(10);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiPass);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() 
{
  // Loop until reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    //attempt to connect
    if (client.connect(brokerID, brokerUser, brokerPass)) {
      Serial.println("connected");

      // // SUBSCRIBER UNTUK ALARM NYALA/MATI
      // client.subscribe("bunyi");
      // Serial.println("Topic Subscribed");

    }
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      //wait for 5s before retrying
      delay(5000);
    }
  }
}

//subscribe call back
void callback(char*topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String data = "";
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    data += (char)payload[i];
  }
  Serial.println();
  Serial.print("Message size :");
  Serial.println(length);
  Serial.println();
  Serial.println("-----------------------");
  Serial.println(data);

// if(data=="ON"){
//   Serial.println("relay");
//   digitalWrite(relay, LOW);
// }
// else{
//   digitalWrite(relay, HIGH);
// }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  // set servo pada posisi 0
  servo.attach(2); //servo D4
  servo.write(0);

  // Koneksi NodeMCU ke WiFi
  setup_wifi();

  // koneksi TCP ke broker MQTT
  client.setServer(brokerHost, 8883);
  client.setCallback(callback); //defining function which will be called when message is received.
}

void loop() {
    if (!client.connected()) 
    {
        reconnect();
    }
    client.loop();

  unsigned long currentMillis = millis(); //read current time

  if (currentMillis - previousMillis >= interval) { //if current time - last time > 5 sec
    previousMillis = currentMillis;                  

    // Sensor baca jarak, 1 kondisi kosong, 0 kondisi ada objek
    slot1 = digitalRead(parkir1); // IR Sensor output pin connected to D0  
    slot2 = digitalRead(parkir2); // IR to D1
    pintu = digitalRead(palang); // IR to D2 (sensor pintu)
    
    // Slot parkir 1
    if(slot1 == 1 )  
    {  
      Serial.println("Slot 1 Kosong");  
    }  
    else  
    {  
      Serial.println("Slot 1 Terisi");  
    }  

    // slot parkir 2
    if(slot2 == 1 )  
    {  
      Serial.println("Slot 2 Kosong");  
    }  
    else  
    {  
      Serial.println("Slot 2 Terisi");  
    } 

    // pintu masuk 
    if(pintu == 1 )  
    {  
      delay(15);
      servo.write(pos);
      Serial.println("Palang nutup");  
      delay(15);
    }  
    else  
    {  
      Serial.println("Palang buka");  
      for (pos = 0; pos <= 180; pos += 1) {
        servo.write(pos);
        delay(15);
      }
      for (pos = 180; pos >= 0; pos -= 1) {
        servo.write(pos);
        delay(15);
      }
    } 

    // Print Jarak
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println("cm");

    // Print data untuk publish
    msgStr = String(distance) +","+String(alarm);
    byte arrSize = msgStr.length() + 1;
    char msg[arrSize];

    Serial.print("PUBLISH DATA:");
    Serial.println(msgStr);
    msgStr.toCharArray(msg, arrSize);
    client.publish(topic, msg);
    msgStr = "";
    delay(50);
  }
}
