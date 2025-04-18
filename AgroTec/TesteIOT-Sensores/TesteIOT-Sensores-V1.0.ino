#include <ESP32Servo.h>
#include <DHT.h>
#include <WiFi.h>

//wifi
const int ledGreen = 4;
const int ledBlue = 2;
const int ledRed = 27;
const char* ssid = "Samsung Galaxy A03";
const char* password = "Michel!@2025";

//DHT22
#define DHTPIN 23
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); //obj dht na classe DHT - não esquecer

//define pro servo
#define SERVO_PIN 22
Servo servoMotor;

void setup() {
  Serial.begin(115200);

//wifi
  pinMode(ledGreen, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledRed, OUTPUT);

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED) {  //se der erro é isso aqui q aparece
    if (millis() - startAttemptTime >= 10000) {
      Serial.println("Falha ao conectar");
      digitalWrite(ledRed, HIGH);
      digitalWrite(ledGreen, LOW);
      digitalWrite(ledBlue, LOW);
      return;
    }
    delay(500);
  }

  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());  //ESP32 ID
  digitalWrite(ledBlue, HIGH);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledRed, LOW);

//sensor
  dht.begin();

//servo
  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);
}

void loop() {

  float umid = dht.readHumidity();
  float temp = dht.readTemperature();

  if (isnan(umid)){
      Serial.println("Deu ruim no DHT");
      delay(2000);
      return;
  }else{
    Serial.println("Umidade: ");
    Serial.println(umid);

    Serial.println("Temperatura: ");
    Serial.println(temp);

      if(umid<40){
        servoMotor.write(90);
      }else{
        servoMotor.write(0);
      }
  }
  delay(20000);
}
