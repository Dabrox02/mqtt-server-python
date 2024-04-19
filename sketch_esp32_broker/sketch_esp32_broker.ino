#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>
///////////////////////////////////////////////////////////////

const int DHT_PIN = 19;
DHTesp dht;
const char* ssid = "xxxxxxxxxx"; /// wifi ssid
const char* password = "xxxxxxxxxx";
const char* mqtt_server = "broker.emqx.io";// server url

/////////////////////////////////////////////////////////////////

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

////////////////////////////////////////////////////////////////////

const int LED_GREEN_PIN = 2; // Change according to the connection of your ESP32
const int LED_YELLOW_PIN = 4; // Change according to the connection of your ESP32
const int LED_RED_PIN = 5; // Change according to the connection of your ESP32

/////////////////////////////////////////////////////////////////////

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
//////////////////////////////////////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}
////////////////////////////////////////////////////////////////////

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("uts-iot/mqtt-jaider/temp", "Welcome ESP32 to Temperature");
      client.subscribe("uts-iot/mqtt-jaider/temp");
      client.publish("uts-iot/mqtt-jaider/hum", "Welcome ESP32 to Humidity");
      client.subscribe("uts-iot/mqtt-jaider/hum");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

/////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(2, OUTPUT);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
}
///////////////////////////////////////////////////////////////////

void loop() {
  if (!client.connected()) {
  reconnect();
  }

  ///////////////////////////
  client.loop();
  unsigned long now = millis();
  
  if (now - lastMsg > 4000) { //perintah publish data
    lastMsg = now;
    TempAndHumidity data = dht.getTempAndHumidity();
    String temp = String(data.temperature, 2);
    client.publish("uts-iot/mqtt-jaider/temp", ("Temperatura: " + temp).c_str()); // publish temp topic /ThinkIOT/temp
    String hum = String(data.humidity, 1); 
    client.publish("uts-iot/mqtt-jaider/hum", ("Humedad: " + hum).c_str());
    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);

    // Get the current status of the LEDs
    int status_green = digitalRead(LED_GREEN_PIN);
    int status_red = digitalRead(LED_RED_PIN);
    int status_yellow = digitalRead(LED_YELLOW_PIN);

    //Control green led
    if (data.temperature > 0) {
      digitalWrite(LED_GREEN_PIN, HIGH); // Encender LED verde si la temperatura es mayor a 0
    } else {
      digitalWrite(LED_GREEN_PIN, LOW); // Apagar LED amarillo si la temperatura es normal
    }

    // Control the LEDs according to the status
    if (data.temperature > 24) {
      digitalWrite(LED_YELLOW_PIN, HIGH); // Encender LED amarillo si la temperatura es alta
    } else {
      digitalWrite(LED_YELLOW_PIN, LOW); // Apagar LED amarillo si la temperatura es normal
    }

    //control red led(possibility of non-functioning-erase)
    if (data.temperature == 0) {
      digitalWrite(LED_RED_PIN, HIGH); // Encender LED rojo si la temperatura es igual a 0
      delay(1000); // Esperar 1 segundo
      digitalWrite(LED_RED_PIN, LOW); // Apagar LED rojo después de 1 segundo
    } else {
      digitalWrite(LED_RED_PIN, LOW); // Asegurarse de que el LED rojo esté apagado si la temperatura es diferente de 0
    }
  }
}
