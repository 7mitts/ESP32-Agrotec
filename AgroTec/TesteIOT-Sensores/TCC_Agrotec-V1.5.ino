#include <DFRobot_DHT11.h> //biblioteca do DHT11
#include <WiFi.h> //biblioteca para o wifi
#include <PubSubClient.h> //biblioteca para MQTT
#include <ESP32Servo.h> //biblioteca para o Servo motor

/*DEFINIÇÕES BÁSICAS PARA NÃO ESQUECER(quem ignorar vai tomar muqueta):
Sensor DHT11 - Sensor de umidade e temperatura que lê a temperatura e umidade,
simples e objetivo pra cambada

Servo ou Servo Motor - motor com braço, se move com base nos dados do sensor DHT11;

mqtt - meio de comunicação/ dispositivo que recebe ou manda informações
nosso caso é o ESP32 com o sensor DHT11(envia) e o servo(recebe);

mqtt broker - intermedio das informações, onde as informações são distribuidas
nosso caso o Node Red, lê as informações do DHT11 e devolve para o ESP32 que move o Servo

uint32 - valor entre 0 e 4.294.967.295;



*/

//DEFINIÇÕES
#define DHT11_PIN 13 //define o pino 13 como sensor DHT11
#define SERVO_PIN 12 //define o pino 12 como servo motor
#define REPORTING_PERIOD_MS 2000 //tempo entre ler os dados do DHT11 e enviar para o servidor

DFRobot_DHT11 DHT; //objeto para se referir ao DHT11
Servo ServoMotor; //objeto para se referir ao Servo Motor

//CONEXÃO WIFI
const char* ssid = "Samsung Galaxy A03"; //define o nome da rede que o esp vai procurar
const char* password = "Michel!@2025"; //define a senha a ser usada para conectar na rede

WiFiCllient Agrotec //Nome do esp para conectar no wifi
PubSubClient Client(Agrotec) //criação do objeto cliente MQTT que vai se comunicar com o Broker

const char* mqtt_server = "18.233.6.18"; //endereço do MQTT Broker, nosso caso o Node Red
const uint16_t mqtt_port = "1883"; //meio de comunicação com o servidor
const char* mqtt_topic = "/esp32"; //topico MQTT lido no Broker
const char* mqtt_name = "Agrotec"; //nome do esp quando for conectar no Broker
const char* mqtt_user = "Michel"; //autenticação do MQTT broker (Node Red)
const char* mqtt_password = "Agrotec@1152025"; //autenticação do MQTT broker (Node Red fellas)

uint32_t tsLastReport = 0; //marca o tempo desde o ultimo envio de dados para o Broker
String toprint; //guarda os ultimos dados enviados

// ========== AQUI COMEÇA O SETUP PRA EU NÂO ESQUECER ==========

void setup() {
  Serial.begin(115200); //Inicia a comunicação serial
  setup_wifi(); //começa a comunicação wifi

  client.setServer(mqtt_server, mqtt_port); //configura a conexão entre o ESP e o MQTT Broker
  client.setCallback(callback); //trata as mensagens recebidas na volta do Broker

  servoMotor.attach(SERVO_PIN); //define o pino 12 com o objeto do Servo
  servoMotor.write(0); //define que o Braço do servo começa em 0°
}

// ========== AQUI COMEÇA O LOOP PRINCIPAL PRA EU NÂO ESQUECER ==========

void loop() {
  if (!client.connected()) { // Verifica se o cliente MQTT está conectado
    reconnect(); // Se não estiver conectado, tenta reconectar
  }

  client.loop(); // Executa a comunicação com o servidor MQTT

  uint32_t now = millis(); // Armazena o tempo atual (em milissegundos)
  if (now - tsLastReport > REPORTING_PERIOD_MS) { // Verifica se o intervalo para o envio de dados foi atingido
    tsLastReport = now; // Atualiza o timestamp do último envio

    DHT.read(DHT11_PIN); // Lê os dados de temperatura e umidade do sensor DHT11

    // Exibe os dados no monitor serial
    Serial.print("Temp: "); //escreve Temp: no serial, é bem óbvio
    Serial.print(DHT.temperature); //exibe a temperatura em sí
    Serial.print(" °C | Umid: "); //escreve Umid: e adiciona o °C a temperatura
    Serial.print(DHT.humidity); //exibe a umidade em sí
    Serial.println(" %"); //exibe o simbolo de porcento para representar a umidade

    // Envia a temperatura para o tópico "/temp" no MQTT
    toprint = String(DHT.temperature);
    client.publish("/temp", toprint.c_str());

    // Envia a umidade para o tópico "/umid" no MQTT
    toprint = String(DHT.humidity);
    client.publish("/umid", toprint.c_str());

    // Controle automático do servomotor baseado na temperatura
    if (DHT.temperature <= 60) { // Se a temperatura for menor ou igual a 60°C
      servoMotor.write(180); // Move o servomotor para a posição 180 graus
    } else { // Caso contrário, move para a posição 90 graus
      servoMotor.write(90);
    }
  }
}

// ========== AQUI COMEÇA O SETUP WIFI PRA EU NÂO ESQUECER ==========

void setup_wifi() {
  Serial.print("Conectando ao Wi-Fi: "); //exibe "Conectando ao Wifi no serial"
  Serial.println(ssid); // Exibe o nome da rede Wi-Fi no monitor serial
  WiFi.begin(ssid, password); // Inicia a conexão Wi-Fi com o SSID e senha fornecidos

  while (WiFi.status() != WL_CONNECTED) { // Enquanto não estiver conectado ao Wi-Fi
    delay(500); // Aguarda 500ms
    Serial.print("."); // Exibe um ponto no monitor serial
  }

  Serial.println("\nWiFi conectado!"); // Informa que a conexão foi bem-sucedida
  Serial.print("Endereço IP: "); //Exibe a frase "Endereço IP" a fds n vou mais botar oq significa printl
  Serial.println(WiFi.localIP()); // Exibe o endereço IP atribuído ao dispositivo
}

// ========== AQUI COMEÇA SISTEMA DE RECONEXÂO DO MQTT PRA CASO CAIA PRA EU NÂO ESQUECER ==========

void reconnect() {
  while (!client.connected()) { // Enquanto não estiver conectado ao servidor MQTT
    Serial.println("Tentando conexão com MQTT..."); // Informa que está tentando conectar

    if (client.connect(mqtt_name, mqtt_user, mqtt_password)) { // Tenta se conectar ao servidor MQTT com nome, usuário e senha
      Serial.println("MQTT conectado!"); // Informa que a conexão foi bem-sucedida
      client.subscribe(mqtt_topic); // Se conectar com sucesso, se inscreve no tópico
    } else { // Se a conexão falhar
      Serial.print("Falhou. Código: ");
      Serial.print(client.state()); // Exibe o código de erro da falha de conexão
      Serial.println(" Tentando novamente em 5 segundos..."); // Informa que tentará novamente em 5 segundos
      delay(5000); // Atraso de 5 segundos antes de tentar novamente
    }
  }
}

// ========== AQUI COMEÇA CALLBACK, aqui vai os bgl PRA CASO CAIA PRA EU NÂO ESQUECER ==========

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensagem recebida no tópico "); 
  Serial.print(topic); // Exibe o tópico em que a mensagem foi recebida
  Serial.print(": ");

  String messageTemp; // Cria uma string temporária para armazenar a mensagem recebida
  for (unsigned int i = 0; i < length; i++) { // Itera sobre os bytes da mensagem
    messageTemp += (char)message[i]; // Converte os bytes para caracteres e os adiciona à string
  }
  Serial.println(messageTemp); // Exibe a mensagem recebida no monitor serial

  if (String(topic) == mqtt_topic) { // Verifica se a mensagem foi recebida no tópico correto
    if (messageTemp == "REBOOT") { // Se a mensagem for "REBOOT"
      Serial.println("Reiniciando ESP..."); // Informa que o ESP será reiniciado
      ESP.restart(); // Reinicia o ESP32
    } else if (messageTemp.startsWith("SERVO:")) { // Se a mensagem começar com "SERVO:"
      int angulo = messageTemp.substring(6).toInt(); // Extrai o valor do ângulo da mensagem
      Serial.print("Movendo o servo para: ");
      Serial.println(angulo); // Exibe o ângulo no monitor serial
      servoMotor.write(angulo); // Move o servomotor para o ângulo especificado
    }
  }
}
