#include <DFRobot_DHT11.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ====== DEFINIÇÕES ======
#define DHT11_PIN 13
#define SERVO_PIN 12
#define REPORTING_PERIOD_MS 2000

DFRobot_DHT11 DHT;
Servo servoMotor;

const char* ssid = "Samsung Galaxy A03";
const char* password = "Michel!@2025";

WiFiClient Agrotec;
PubSubClient client(Agrotec);

const char* mqtt_server = "18.233.6.18";
const uint16_t mqtt_port = 1883;
const char* mqtt_topic = "/esp";
const char* mqtt_name = "Agrotec";
const char* mqtt_user = "Michel";
const char* mqtt_password = "Agrotec@1152025";

uint32_t tsLastReport = 0;
String toprint;

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0); // posição inicial
}

// ====== LOOP PRINCIPAL ======
void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  uint32_t now = millis();
  if (now - tsLastReport > REPORTING_PERIOD_MS) {
    tsLastReport = now;

    DHT.read(DHT11_PIN);

    Serial.print("Temp: ");
    Serial.print(DHT.temperature);
    Serial.print(" °C | Umid: ");
    Serial.print(DHT.humidity);
    Serial.println(" %");

    // Envia temperatura
    toprint = String(DHT.temperature);
    client.publish("/temp", toprint.c_str());

    // Envia umidade
    toprint = String(DHT.humidity);
    client.publish("/umid", toprint.c_str());

    // Controle automático do servo
    if (DHT.temperature <= 60) {
      servoMotor.write(180);
    } else {
      servoMotor.write(90);
    }
  }
}

// ====== CONEXÃO WI-FI ======
void setup_wifi() {
  Serial.print("Conectando ao Wi-Fi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

// ====== RECONEXÃO MQTT ======
void reconnect() {
  while (!client.connected()) {
    Serial.println("Tentando conexão com MQTT...");

    if (client.connect(mqtt_name, mqtt_user, mqtt_password)) {
      Serial.println("MQTT conectado!");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Falhou. Código: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

// ====== CALLBACK MQTT ======
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensagem recebida no tópico ");
  Serial.print(topic);
  Serial.print(": ");

  String messageTemp;
  for (unsigned int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);

  if (String(topic) == mqtt_topic) {
    if (messageTemp == "REBOOT") {
      Serial.println("Reiniciando ESP...");
      ESP.restart();
    } else if (messageTemp.startsWith("SERVO:")) {
      int angulo = messageTemp.substring(6).toInt();
      Serial.print("Movendo o servo para: ");
      Serial.println(angulo);
      servoMotor.write(angulo);
    }
  }
}
