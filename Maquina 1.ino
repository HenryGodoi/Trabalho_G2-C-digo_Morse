// ============================================================
//  MÁQUINA 1 — TRANSMISSORA (Ligação Padrão no GND)
// ============================================================

const int PIN_IR_LED     = 3;   
const int PIN_LED_STATUS = 13;  

// ── Temporização Morse (ms) ────────────────────────────────
const int DIT         = 200;    
const int DAH         = 600;    
const int SYMBOL_GAP  = 200;    
const int LETTER_GAP  = 600;    
const int WORD_GAP    = 1400;   

const char* MORSE_TABLE[] = {
  ".-","-...","-.-.","-..",".","..-.","--.","....",
  "..",".---","-.-",".-..","--","-.","---",".--.",
  "--.-",".-.","...","-","..-","...-",".--","-..-",
  "-.--","--.."
};
const char* MORSE_DIGITS[] = {
  "-----",".----","..---","...--","....-",
  ".....","-....","--...","---..","----."
};

const char* charToMorse(char c) {
  c = toupper(c);
  if (c >= 'A' && c <= 'Z') return MORSE_TABLE[c - 'A'];
  if (c >= '0' && c <= '9') return MORSE_DIGITS[c - '0'];
  return nullptr;
}

void printMorse(const String& text) {
  bool first = true;
  for (int i = 0; i < (int)text.length(); i++) {
    char c = text[i];
    if (c == ' ') { Serial.print(F(" / ")); first = true; continue; }
    const char* m = charToMorse(c);
    if (!m) continue;
    if (!first) Serial.print(' ');
    Serial.print(m);
    first = false;
  }
}

// Liga o LED IR
void emitPulse(bool isDah) {
  digitalWrite(PIN_IR_LED, HIGH);     // LIGA o LED
  digitalWrite(PIN_LED_STATUS, HIGH); 
  delay(isDah ? DAH : DIT);
  digitalWrite(PIN_IR_LED, LOW);      // DESLIGA o LED
  digitalWrite(PIN_LED_STATUS, LOW);
  delay(SYMBOL_GAP);
}

void sendLetter(const char* morse) {
  for (int i = 0; morse[i] != '\0'; i++) {
    emitPulse(morse[i] == '-');
  }
  delay(LETTER_GAP - SYMBOL_GAP);
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_IR_LED, OUTPUT);
  pinMode(PIN_LED_STATUS, OUTPUT);
  
  // Garante que o LED inicie APAGADO
  digitalWrite(PIN_IR_LED, LOW);
  
  Serial.println(F("MORSE TX PRONTO (modo padrao)"));
}

void loop() {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  Serial.println(F("Processando e Enviando..."));
  
  for (int i = 0; i < (int)input.length(); i++) {
    char c = input[i];
    if (c == ' ') { delay(WORD_GAP); continue; }
    const char* morse = charToMorse(c);
    if (!morse) continue;
    sendLetter(morse);
  }

  // Pulso longo especial de "fim de mensagem"
  digitalWrite(PIN_IR_LED, HIGH); // LIGA
  delay(1200);
  digitalWrite(PIN_IR_LED, LOW);  // DESLIGA

  Serial.println(F("Mensagem enviada!"));
  Serial.println(F("----"));
}
