// ============================================================
//  MÁQUINA 2 — RECEPTORA (versão para fototransistor 2 pinos)
//  Lê a tensão analógica em A0 (divisor de tensão com o
//  fototransistor) e mede a DURAÇÃO de cada pulso de luz IR
//  para decidir se é ponto (.) ou traço (-).
//
//  Ligação:
//    Fototransistor + resistor 10kΩ em divisor de tensão → A0
//    (ver diagrama do circuito enviado)
// ============================================================

const int PIN_IR_SENSOR  = A0;
const int PIN_LED_STATUS = 13;

// Limiar: abaixo disso = "luz detectada" (LOW no fototransistor)
// Ajuste este valor testando no Monitor Serial se necessário.
const int THRESHOLD = 500;

// ── Temporização — deve ser compatível com a Máquina 1 ─────
const int DIT_MAX         = 350;    // até aqui = ponto
const int DAH_MAX         = 900;    // até aqui = traço
const int END_MSG_MIN     = 1000;   // pulso mais longo que isso = fim de msg
const int LETTER_TIMEOUT  = 400;    // CORRIGIDO: >200ms e <600ms (Pausa da letra)
const int WORD_TIMEOUT    = 1000;   // CORRIGIDO: >600ms e <1400ms (Pausa da palavra)

String morseBuffer  = "";
String morseMessage = "";
String decodedText  = "";
unsigned long pulseStart   = 0;
unsigned long silenceStart = 0;
bool wasLit        = false;
bool letterPending = false;

char decodeMorse(const String& code) {
  const char* CODES_L[] = {
    ".-","-...","-.-.","-..",".","..-.","--.","....",
    "..",".---","-.-",".-..","--","-.","---",".--.",
    "--.-",".-.","...","-","..-","...-",".--","-..-",
    "-.--","--.."
  };
  for (int i = 0; i < 26; i++) if (code == CODES_L[i]) return 'A' + i;
  
  const char* CODES_D[] = {
    "-----",".----","..---","...--","....-",
    ".....","-....","--...","---..","----."
  };
  for (int i = 0; i < 10; i++) if (code == CODES_D[i]) return '0' + i;
  return '?';
}

void finalizeLetter() {
  if (morseBuffer.length() == 0) return;
  if (morseMessage.length() > 0) morseMessage += ' ';
  morseMessage += morseBuffer;
  decodedText += decodeMorse(morseBuffer);
  morseBuffer = "";
}

void finalizeMessage() {
  finalizeLetter();
  if (decodedText.length() == 0) return;

  Serial.println(F("Processando..."));
  delay(200);
  Serial.println(F("Decodificando..."));
  delay(200);
  Serial.print(F("Código morse: "));
  Serial.println(morseMessage);
  Serial.print(F("Palavra: "));
  Serial.println(decodedText);
  Serial.println(F("----"));
  Serial.println(F("Aguardando próxima transmissão..."));

  morseBuffer = morseMessage = decodedText = "";
}

void setup() {
  Serial.begin(9600);
  pinMode(PIN_LED_STATUS, OUTPUT);
  Serial.println(F("MORSE RX PRONTO (modo fototransistor)"));
  Serial.println(F("Aguardando transmissão IR..."));
  silenceStart = millis();
}

void loop() {
  int reading = analogRead(PIN_IR_SENSOR);
  bool isLit = (reading < THRESHOLD);
  digitalWrite(PIN_LED_STATUS, isLit ? HIGH : LOW);
  unsigned long now = millis();

  // ── Início de um pulso de luz ──
  if (isLit && !wasLit) {
    pulseStart = now;
    wasLit = true;
  }

  // ── Fim de um pulso de luz → classifica ponto/traço/fim ──
  if (!isLit && wasLit) {
    unsigned long duration = now - pulseStart;
    wasLit = false;
    silenceStart = now;

    if (duration >= END_MSG_MIN) {
      finalizeMessage();
    } else if (duration <= DIT_MAX) {
      morseBuffer += '.';
      Serial.print('.');
    } else if (duration <= DAH_MAX) {
      morseBuffer += '-';
      Serial.print('-');
    }
    // pulsos fora dessas faixas são ignorados (ruído)
  }

  // ── Silêncio prolongado → fecha letra ou palavra ──
  if (!isLit) {
    unsigned long silence = now - silenceStart;
    if (silence >= WORD_TIMEOUT && morseBuffer.length() == 0
        && morseMessage.length() > 0 && decodedText.charAt(decodedText.length()-1) != ' ') {
      decodedText += ' ';
      if (morseMessage.length() > 0) morseMessage += " /";
      Serial.print(F(" / "));
    } else if (silence >= LETTER_TIMEOUT && morseBuffer.length() > 0) {
      Serial.print(' ');
      finalizeLetter();
    }
  }
}
