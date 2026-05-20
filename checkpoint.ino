#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>



const int LDR = A0;

#define DHTPIN 10
#define DHTTYPE DHT22

const int LED_VERDE    = 13;
const int LED_AMARELO  = 12;
const int LED_VERMELHO = 11;
const int BUZZER       = 4;


const int LDR_ESCURO = 700;
const int LDR_MEIA   = 300;

const float TEMP_MIN = 10.0;
const float TEMP_MAX = 15.0;
const float UMID_MIN = 50.0;
const float UMID_MAX = 70.0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

DHT dht(DHTPIN, DHTTYPE);

float temperatura  = 0;
float umidade      = 0;
int   luzRaw       = 0;
int   luminosidade = 0;

unsigned long ultimaLeitura = 0;
const unsigned long INTERVALO = 5000;

int telaAtual = 0;

void setup() {
  pinMode(LED_VERDE,    OUTPUT);
  pinMode(LED_AMARELO,  OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER,       OUTPUT);

  Serial.begin(9600);
  dht.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Vinheria");
  lcd.setCursor(0, 1);
  lcd.print("Agnello");
  delay(2000);
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultimaLeitura >= INTERVALO) {
    ultimaLeitura = agora;

    lerSensores();
    atualizarAlertas();
    exibirLCD();
    exibirSerial();
  }
}

void lerSensores() {
  const int N = 5;

  float somaTemp = 0;
  float somaUmid = 0;
  long  somaLuz  = 0;
  int   leituras = 0;

  for (int i = 0; i < N; i++) {
    float t = dht.readTemperature();
    float u = dht.readHumidity();
    int   l = analogRead(LDR);

    if (!isnan(t) && !isnan(u)) {
      somaTemp += t;
      somaUmid += u;
      somaLuz  += l;
      leituras++;
    }
    delay(200);
  }

  if (leituras > 0) {
    temperatura = somaTemp / leituras;
    umidade     = somaUmid / leituras;
    luzRaw      = somaLuz / leituras;
    luminosidade = map(luzRaw, 1023, 0, 0, 100);
  } else {
    Serial.println("Erro: leituras do DHT22 falharam!");
  }
}

void atualizarAlertas() {
  digitalWrite(LED_VERDE,    LOW);
  digitalWrite(LED_AMARELO,  LOW);
  digitalWrite(LED_VERMELHO, LOW);
  noTone(BUZZER);

  bool buzzerAtivo = false;

  if (luzRaw > LDR_ESCURO) {
    digitalWrite(LED_VERDE, HIGH);       

  } else if (luzRaw >= LDR_MEIA) {
    digitalWrite(LED_AMARELO, HIGH);     

  } else {
    digitalWrite(LED_VERMELHO, HIGH);    
    buzzerAtivo = true;                  
  }

  if (temperatura < TEMP_MIN || temperatura > TEMP_MAX) {
    digitalWrite(LED_AMARELO, HIGH);
    buzzerAtivo = true;
  }

  if (umidade < UMID_MIN || umidade > UMID_MAX) {
    digitalWrite(LED_VERMELHO, HIGH);
    buzzerAtivo = true;
  }

  if (buzzerAtivo) {
    tone(BUZZER, 1000);
  }
}

void exibirLCD() {
  lcd.clear();

  if (telaAtual == 0) {

    if (luzRaw > LDR_ESCURO) {
      lcd.setCursor(0, 0);
      lcd.print("Ambiente Escuro");
      lcd.setCursor(0, 1);
      lcd.print("Luz: ");
      lcd.print(luminosidade);
      lcd.print("%");

    } else if (luzRaw >= LDR_MEIA) {
      lcd.setCursor(0, 0);
      lcd.print("Ambiente a meia");
      lcd.setCursor(0, 1);
      lcd.print("luz");

    } else {
      lcd.setCursor(0, 0);
      lcd.print("Ambiente muito");
      lcd.setCursor(0, 1);
      lcd.print("CLARO");
    }

  } else if (telaAtual == 1) {

    if (temperatura >= TEMP_MIN && temperatura <= TEMP_MAX) {
      lcd.setCursor(0, 0);
      lcd.print("Temperatura OK");
      lcd.setCursor(0, 1);
      lcd.print("Temp. = ");
      lcd.print(temperatura, 1);
      lcd.print("C");

    } else if (temperatura > TEMP_MAX) {
      lcd.setCursor(0, 0);
      lcd.print("Temp. ALTA");
      lcd.setCursor(0, 1);
      lcd.print("Temp. = ");
      lcd.print(temperatura, 1);
      lcd.print("C");

    } else {
      lcd.setCursor(0, 0);
      lcd.print("Temp. BAIXA");
      lcd.setCursor(0, 1);
      lcd.print("Temp. = ");
      lcd.print(temperatura, 1);
      lcd.print("C");
    }

  } else {

    if (umidade >= UMID_MIN && umidade <= UMID_MAX) {
      lcd.setCursor(0, 0);
      lcd.print("Umidade OK");
      lcd.setCursor(0, 1);
      lcd.print("Umidade = ");
      lcd.print(umidade, 0);
      lcd.print("%");

    } else if (umidade > UMID_MAX) {
      lcd.setCursor(0, 0);
      lcd.print("Umidade ALTA");
      lcd.setCursor(0, 1);
      lcd.print("Umidade = ");
      lcd.print(umidade, 0);
      lcd.print("%");

    } else {
      lcd.setCursor(0, 0);
      lcd.print("Umidade BAIXA");
      lcd.setCursor(0, 1);
      lcd.print("Umidade = ");
      lcd.print(umidade, 0);
      lcd.print("%");
    }
  }

  telaAtual = (telaAtual + 1) % 3;
}

void exibirSerial() {
  Serial.println("======== STATUS ========");
  Serial.print("LDR raw:      "); Serial.println(luzRaw);
  Serial.print("Luminosidade: "); Serial.print(luminosidade); Serial.println("%");
  Serial.print("Temperatura:  "); Serial.print(temperatura, 1); Serial.println(" C");
  Serial.print("Umidade:      "); Serial.print(umidade, 1); Serial.println(" %");
  Serial.println("========================");
}