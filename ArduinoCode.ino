// Definindo o pino de controle da fechadura
const int pinFechadura = 7; // Pino onde o relé da fechadura está conectado
const int pinSinalESP = 8;  // Pino onde o Arduino recebe o sinal do ESP32

void setup() {
    // Inicializando os pinos
    pinMode(pinFechadura, OUTPUT); 
    pinMode(pinSinalESP, INPUT);
    // A fechadura começa em estado de travada
    digitalWrite(pinFechadura, LOW); 
}

void loop() {
    // Lê o sinal vindo do ESP32
    int sinal = digitalRead(pinSinalESP);

    // Se o sinal for HIGH, abre a fechadura
    if (sinal == HIGH) {
        digitalWrite(pinFechadura, HIGH); // Aciona a fechadura (abre)
        delay(5000); // Mantém a fechadura aberta por 5 segundos
        digitalWrite(pinFechadura, LOW);  // Fecha a fechadura novamente
    }
}