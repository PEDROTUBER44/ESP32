/*

Codigo do DOIT ESP32 DEVKIT V1.

O código consiste no seguinte: ele ficará verificando o Realtime Database do Firebase.
Caso ele esteja diferente do estado atual da fechadura, ele irá mudar para se igualar ao do Firebase.

*/


#include <WiFi.h> // Incluir Modulo de conexão ao WIFI
#include <Firebase_ESP_Client.h> // Incluir modulo de Client do Firebase
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

const char* ssid = "Casa"; // SSID da Internet
const char* password = "ramos#@1"; // Senha da Internet
const int outputPin = 21; // Define a porta serial
bool signupOK = false; // Variável para controlar o estado de conexão com o Firebase

#define API_KEY "AIzaSyBZkRfkPaOJAUwbdJHjecqrXj2NEJIxLxY" // API_KEY do firebase
#define DATABASE_URL "https://esp32-bece3-default-rtdb.firebaseio.com/" // URL do Database

// Objetos do Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void toggleFECHADURA(bool fechaduraState) {
    digitalWrite(outputPin, fechaduraState);
    
    // Atualiza o estado no Firebase
    if (Firebase.RTDB.setBool(&fbdo, "fechadura/state", fechaduraState)){
        Serial.println("Estado atualizado no Firebase");
        
        // Envia sinal pela porta serial setada na variavel outputPin
        if (fechaduraState) {
            Serial.write("PORTA SERIAL LIGADA\n"); // Enviar comando para a portaserial ligar
        } else {
            Serial.write("PORTA SERIAL DESLIGADA\n"); // Enviar comando para a portaserial desligar 
        }
    }
    else {
        Serial.println("Falha ao atualizar Firebase");
        Serial.println("RAZÃO: " + fbdo.errorReason());
    }
}

void setup() {
    Serial.begin(2000000); // 9600 apenas para debug
    Serial.println("\nIniciando...");

    pinMode(outputPin, OUTPUT);
    digitalWrite(outputPin, LOW);

    // Conexão WiFi
    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, password);

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 20) {
        delay(1000);
        Serial.print(".");
        attempt++;
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi conectado!");
        Serial.print("IP local: ");
        Serial.println(WiFi.localIP());

        // Configuração do Firebase
        config.api_key = API_KEY;
        config.database_url = DATABASE_URL;

        if (Firebase.signUp(&config, &auth, "", "")) {
            Serial.println("Signup no Firebase OK");
            signupOK = true;
        } else {
            Serial.printf("Falha no Signup %s\n", config.signer.signupError.message.c_str());
        }

        config.token_status_callback = tokenStatusCallback;
        Firebase.begin(&config, &auth);
        Firebase.reconnectWiFi(true);
    } else {
        Serial.println("Falha ao conectar ao WiFi");
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        // Verifica mudanças no Firebase
        if (Firebase.ready() && signupOK) {
            if (Firebase.RTDB.getBool(&fbdo, "fechadura/state")) {
                bool firebaseState = fbdo.boolData();
                digitalWrite(outputPin, firebaseState);
            }
        }
    }
}