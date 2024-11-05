#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

const char* ssid = "VIVOFIBRA-F611";
const char* password = "sL5LsvrGcg";
const int outputLock = 19;    // Pin for lock control
const int outputLed = 22;     // Pin for LED
const int outputSirene = 18;  // Pin for speaker
bool signupOK = false;

// Variables for siren timing control
unsigned long sireneStartTime = 0;
const unsigned long sireneDuration = 5000; // Siren duration in milliseconds (5 seconds)
bool sireneActive = false;

#define API_KEY "AIzaSyBZkRfkPaOJAUwbdJHjecqrXj2NEJIxLxY"
#define DATABASE_URL "https://esp32-bece3-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void toggleFECHADURA(bool fechaduraState) {
    digitalWrite(outputLed, fechaduraState);   // Control LED
    digitalWrite(outputLock, fechaduraState);  // Control actual lock - Agora corresponde diretamente ao estado
    
    if (Firebase.RTDB.setBool(&fbdo, "fechadura/state", fechaduraState)){
        Serial.println("Estado atualizado no Firebase");
        
        if (fechaduraState) {
            Serial.write("PORTA SERIAL LIGADA\n");
        } else {
            Serial.write("PORTA SERIAL DESLIGADA\n");
        }
    }
    else {
        Serial.println("Falha ao atualizar Firebase");
        Serial.println("RAZÃO: " + fbdo.errorReason());
    }
}

void toggleSPEAKER(bool speakerState) {
    if (speakerState && !sireneActive) {
        sireneActive = true;
        sireneStartTime = millis();
        Serial.println("Sirene ativada");
    }
    
    if (Firebase.RTDB.setBool(&fbdo, "speaker/state", speakerState)){
        Serial.println("Estado do speaker atualizado no Firebase");
    }
    else {
        Serial.println("Falha ao atualizar estado do speaker no Firebase");
        Serial.println("RAZÃO: " + fbdo.errorReason());
    }
}

void handleSirene() {
    if (sireneActive) {
        tone(outputSirene, 100);
        delay(500);
        tone(outputSirene, 500);
        delay(500);
        
        if (millis() - sireneStartTime >= sireneDuration) {
            noTone(outputSirene);
            sireneActive = false;
            
            if (Firebase.RTDB.setBool(&fbdo, "speaker/state", false)) {
                Serial.println("Sirene desativada e Firebase atualizado");
            } else {
                Serial.println("Falha ao atualizar estado da sirene no Firebase");
            }
        }
    }
}

void setup() {
    Serial.begin(2000000);
    Serial.println("\nIniciando...");

    pinMode(outputLock, OUTPUT);  // Initialize lock pin
    pinMode(outputLed, OUTPUT);   // Initialize LED pin
    pinMode(outputSirene, OUTPUT); // Initialize speaker pin
    
    digitalWrite(outputLock, LOW);  // Ensure lock starts closed
    digitalWrite(outputLed, LOW);   // Ensure LED starts off
    noTone(outputSirene);          // Ensure speaker starts silent

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
        
        // Inicializa o estado da fechadura verificando o Firebase
        if (Firebase.RTDB.getBool(&fbdo, "fechadura/state")) {
            bool initialState = fbdo.boolData();
            digitalWrite(outputLed, initialState);
            digitalWrite(outputLock, initialState);
        }
    } else {
        Serial.println("Falha ao conectar ao WiFi");
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (Firebase.ready() && signupOK) {
            // Check lock state
            if (Firebase.RTDB.getBool(&fbdo, "fechadura/state")) {
              bool fechaduraState = fbdo.boolData();
              digitalWrite(outputLed, fechaduraState);
              digitalWrite(outputLock, fechaduraState);  // Atualiza diretamente o estado da fechadura
            }
            
            // Check speaker state
            if (Firebase.RTDB.getBool(&fbdo, "speaker/state")) {
                bool speakerState = fbdo.boolData();
                toggleSPEAKER(speakerState);
            }
            
            // Handle siren operation
            handleSirene();
        }
    }
}