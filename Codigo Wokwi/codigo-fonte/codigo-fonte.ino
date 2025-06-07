#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Definições dos pinos
#define PINO_DHT 15
#define TIPO_DHT DHT22
#define LED_ALERTA 2

DHT dht(PINO_DHT, TIPO_DHT);

// Dados da rede Wi-Fi (Wokwi usa Wi-Fi simulado)
const char* ssid = "Wokwi-GUEST";
const char* senha = "";

// Dados do servidor MQTT público
const char* servidorMQTT = "broker.hivemq.com";
const int portaMQTT = 1883;

WiFiClient espClient;
PubSubClient cliente(espClient);

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(LED_ALERTA, OUTPUT);

  // Conectar ao Wi-Fi
  WiFi.begin(ssid, senha);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado com sucesso!");

  cliente.setServer(servidorMQTT, portaMQTT);
}

void loop() {
  if (!cliente.connected()) {
    reconectar();
  }
  cliente.loop();

  // Leitura dos sensores simulados no Wokwi (potenciômetros nos pinos 34 e 35)
  int umidadeSolo = analogRead(34); // sensor de umidade do solo simulado
  int nivelAgua = analogRead(35);   // sensor de nível de água simulado

  // Leitura DHT22 (simulado no Wokwi)
  float umidadeAr = dht.readHumidity();
  float temperatura = dht.readTemperature();

  // Acende LED se os níveis estiverem críticos
  if (umidadeSolo > 700 && nivelAgua > 700) {
    digitalWrite(LED_ALERTA, HIGH);
  } else {
    digitalWrite(LED_ALERTA, LOW);
  }

  // Monta mensagem JSON
  String mensagem = "{\"umidadeSolo\":" + String(umidadeSolo) +
                    ",\"nivelAgua\":" + String(nivelAgua) +
                    ",\"temperatura\":" + String(temperatura, 1) +
                    ",\"umidadeAr\":" + String(umidadeAr, 1) + "}";

  // Publica no tópico MQTT
  cliente.publish("chuva/monitoramento", mensagem.c_str());

  Serial.println(mensagem);

  delay(5000);
}

void reconectar() {
  while (!cliente.connected()) {
    Serial.print("Tentando conectar MQTT...");
    if (cliente.connect("ESP32ChuvaAlerta")) {
      Serial.println("Conectado ao servidor MQTT");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}
