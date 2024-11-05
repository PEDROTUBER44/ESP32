#include <Keypad.h>

// Configuração do teclado matricial 3x4
const byte ROWS = 4;      // Número de linhas do teclado
const byte COLS = 3;      // Número de colunas do teclado
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {13, 12, 11, 10};    // Pinos das linhas
byte colPins[COLS] = {9, 8, 2};           // Pinos das colunas

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Configuração dos pinos
const int ledWhitePin = 6;       // LED branco (acionado por A0)
const int speakerPin = 5;        // Speaker (acionado por A1)
const int ledRedPin = 4;         // LED vermelho (sempre ativo, pisca no reset)
const int ledGreenPin = 3;       // LED verde (confirmação)

// Definição das opções de entrega
String deliveryOptions[10] = {
  "Outro tipo", "Amazon", "CEDEX", "Correios", "Mercado Livre",
  "iFood", "Uber Entrega", "DHL", "Casas Bahia", "Shein"
};

// Variáveis de controle
String selectedOption = "";     // Armazena a opção de correio selecionada
unsigned long ledTimer = 0;     // Temporizador para o LED verde
unsigned long whiteLedTimer = 0; // Temporizador para o LED branco
unsigned long speakerTimer = 0; // Temporizador para o alto-falante
bool isConfirmationActive = false; // Indica se a confirmação está ativa
bool isWhiteLedOn = false;       // Indica se o LED branco está ligado
bool isSpeakerOn = false;        // Indica se o alto-falante está ligado

void setup() {
  pinMode(ledWhitePin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  pinMode(ledRedPin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  
  // Configura pinos A0 e A1 como entradas
  pinMode(A0, INPUT);           // Entrada para o LED branco
  pinMode(A1, INPUT);           // Entrada para o speaker
  
  digitalWrite(ledRedPin, HIGH);   // LED vermelho ligado no início
  Serial.begin(2000000);               // Inicia comunicação serial com o ESP32
}

void loop() {
  // Desabilita a função do teclado enquanto o LED verde está ativo
  if (!isConfirmationActive) {
    // Verifica o teclado somente se não está em confirmação
    char key = keypad.getKey();

    if (key) {
      if (key >= '0' && key <= '9') {
        selectedOption = String(key);   // Armazena a seleção do usuário
      } else if (key == '*') {
        resetOption();                  // Ação de correção (reset)
      } else if (key == '#') {
        confirmOption();                // Ação de confirmação
      }
    }
  }

  // Verifica a entrada A0 para acionar o LED branco
  if (digitalRead(A0) == HIGH && !isWhiteLedOn) {
    digitalWrite(ledWhitePin, HIGH);
    whiteLedTimer = millis();
    isWhiteLedOn = true;
  }
  
  // Desliga o LED branco após 15 segundos
  if (isWhiteLedOn && (millis() - whiteLedTimer >= 15000)) {
    digitalWrite(ledWhitePin, LOW);
    isWhiteLedOn = false;
  }

  // Verifica a entrada A1 para acionar o alto-falante
  if (digitalRead(A1) == HIGH && !isSpeakerOn) {
    tone(speakerPin, 1000);      // Emite som de 1kHz
    speakerTimer = millis();
    isSpeakerOn = true;
  }
  
  // Desliga o alto-falante após 15 segundos
  if (isSpeakerOn && (millis() - speakerTimer >= 15000)) {
    noTone(speakerPin);
    isSpeakerOn = false;
  }
  
  // Desliga o LED verde e religa o vermelho após a confirmação
  if (isConfirmationActive && (millis() - ledTimer >= 15000)) {
    digitalWrite(ledGreenPin, LOW);
    digitalWrite(ledRedPin, HIGH);
    isConfirmationActive = false;
  }
}

// Função para corrigir ou resetar a última ação
void resetOption() {
  selectedOption = "";               // Reseta a opção selecionada
  for (int i = 0; i < 3; i++) {      // LED vermelho pisca 3 vezes
    digitalWrite(ledRedPin, LOW);
    delay(200);
    digitalWrite(ledRedPin, HIGH);
    delay(200);
  }
  Serial.println("Opção resetada.");
}

// Função para confirmar a ação
void confirmOption() {
  if (selectedOption != "") {
    int optionIndex = selectedOption.toInt();  // Converte a seleção para número
    if (optionIndex >= 0 && optionIndex <= 9) {
      // Envia a opção de entrega correspondente ao ESP32
      Serial.print("Opção confirmada: ");
      Serial.println(deliveryOptions[optionIndex]); // Envia o nome da opção ao monitor serial

      // Adiciona uma mensagem de comentário ao monitor serial
      Serial.print("Enviando para ESP32: ");
      Serial.println(deliveryOptions[optionIndex]); // Envia a informação para o ESP32

    } else {
      Serial.println("Opção inválida.");
    }
    selectedOption = "";              // Reseta a seleção
    digitalWrite(ledGreenPin, HIGH);  // Liga o LED verde
    digitalWrite(ledRedPin, LOW);     // Desliga o LED vermelho
    ledTimer = millis();
    isConfirmationActive = true;
  } else {
    Serial.println("Nenhuma opção selecionada para confirmar.");
  }
}
