#include <WiFi.h>
#include <WebServer.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Configurações do WiFi
const char* ssid = "VIVOFIBRA-F611";
const char* password = "sL5LsvrGcg";

// Configurações do Firebase
#define API_KEY "AIzaSyBZkRfkPaOJAUwbdJHjecqrXj2NEJIxLxY"
#define DATABASE_URL "https://esp32-bece3-default-rtdb.firebaseio.com/"


const int ledPin = 21; // Define a porta serial

// Objetos do Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

WebServer server(80);

// Variável para controlar o estado de conexão com o Firebase
bool signupOK = false;

// Página HTML
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <title>Controle ESP32</title>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            margin-top: 50px;
        }
        .button {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 15px 32px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 16px;
            margin: 4px 2px;
            cursor: pointer;
            border-radius: 4px;
        }
        .button.off {
            background-color: #f44336;
        }
    </style>
</head>
<body>
    <h1>Controle do LED</h1>
    <button id="ledButton" class="button" onclick="toggleLED()">LIGAR LED</button>
    <script>
        var ledStatus = false;

        function toggleLED() {
            var button = document.getElementById('ledButton');
            ledStatus = !ledStatus;

            fetch('/toggle')
                .then(response => response.text())
                .then(state => {
                    if (ledStatus) {
                        button.innerHTML = 'DESLIGAR LED';
                        button.classList.add('off');
                    } else {
                        button.innerHTML = 'LIGAR LED';
                        button.classList.remove('off');
                    }
                })
                .catch(error => console.error('Erro:', error));
        }

        // Verificar estado inicial
        fetch('/getState')
            .then(response => response.text())
            .then(state => {
                ledStatus = state === 'on';
                if (ledStatus) {
                    button.innerHTML = 'DESLIGAR LED';
                    button.classList.add('off');
                }
            });
    </script>
</body>
</html>
)rawliteral";

void handleRoot() {
    server.send(200, "text/html", index_html);
}

void handleToggle() {
    static bool ledState = false;
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    
    // Atualiza o estado no Firebase
    if (Firebase.RTDB.setBool(&fbdo, "led/state", ledState)){
        Serial.println("Estado atualizado no Firebase");
        // Envia sinal pela porta serial 21
        if (ledState) {
            Serial.write("LED LIGADO\n"); // Enviar comando para ligar LED
        } else {
            Serial.write("LED DESLIGADO\n"); // Enviar comando para desligar LED
        }
    }
    else {
        Serial.println("Falha ao atualizar Firebase");
        Serial.println("RAZÃO: " + fbdo.errorReason());
    }
    
    server.send(200, "text/plain", ledState ? "on" : "off");
}

void handleGetState() {
    bool ledState;
    if (Firebase.RTDB.getBool(&fbdo, "led/state", &ledState)) {
        server.send(200, "text/plain", ledState ? "on" : "off");
    } else {
        server.send(500, "text/plain", "erro");
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("\nIniciando...");

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

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

        // Configuração do servidor web
        server.on("/", HTTP_GET, handleRoot);
        server.on("/toggle", HTTP_GET, handleToggle);
        server.on("/getState", HTTP_GET, handleGetState);
        server.begin();
        Serial.println("Servidor web iniciado");
    } else {
        Serial.println("Falha ao conectar ao WiFi");
    }
}

void loop() {
    if (WiFi.status() == WL_CONNECTED) {
        server.handleClient();
        
        // Verifica mudanças no Firebase
        if (Firebase.ready() && signupOK) {
            if (Firebase.RTDB.getBool(&fbdo, "led/state")) {
                bool firebaseState = fbdo.boolData();
                digitalWrite(ledPin, firebaseState);
            }
        }
    }
}
