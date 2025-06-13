#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Konfigurasi LCD 20x4 ===
LiquidCrystal_I2C lcd(0x27, 20, 4);

// === Pin Relay ===
#define RELAY_POMPA_AIR 10
#define RELAY_POMPA_NUTRISI 11

// === Sensor TDS ===
#define TDS_PIN A0

// === Sensor JSN-SR04T ===
#define TRIG_PIN 5
#define ECHO_PIN 6

// === Batasan Parameter ===
#define TINGGI_BATAS_CM 10.0
#define PPM_MIN 800
#define PPM_MAX 1100

bool pumpAirState = false;
bool pumpNutrisiState = false;

void setup() {
  Serial.begin(9600);
  lcd.begin(20, 4);
  lcd.backlight();

  pinMode(RELAY_POMPA_AIR, OUTPUT);
  pinMode(RELAY_POMPA_NUTRISI, OUTPUT);
  digitalWrite(RELAY_POMPA_AIR, HIGH);       // Mati di awal
  digitalWrite(RELAY_POMPA_NUTRISI, HIGH);   // Mati di awal

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.setCursor(0, 0);
  lcd.print("Sistem Nutrisi Aktif");
  delay(2000);
}

float bacaTinggiAirCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH);
  float jarak = durasi * 0.034 / 2.0;
  float tinggiMaks = 30.0;
  float tinggiAir = tinggiMaks - jarak;
  return max(0.0, min(tinggiAir, tinggiMaks));
}

int bacaTDS() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * (5.0 / 1023.0);
  int ppm = (voltage - 0.015) / 0.0011;
  return ppm;
}

void loop() {
  float tinggi = bacaTinggiAirCM();
  int ppm = bacaTDS();

  // === Logika Kontrol Pompa ===
  if (tinggi >= TINGGI_BATAS_CM) {
    digitalWrite(RELAY_POMPA_AIR, HIGH);
    digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
    pumpAirState = false;
    pumpNutrisiState = false;
  } else {
    if (ppm > PPM_MAX) {
      digitalWrite(RELAY_POMPA_AIR, LOW);    // Pompa air hidup
      digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
      pumpAirState = true;
      pumpNutrisiState = false;
    } else if (ppm < PPM_MIN) {
      digitalWrite(RELAY_POMPA_AIR, HIGH);
      digitalWrite(RELAY_POMPA_NUTRISI, LOW); // Pompa nutrisi hidup
      pumpAirState = false;
      pumpNutrisiState = true;
    } else {
      digitalWrite(RELAY_POMPA_AIR, HIGH);
      digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
      pumpAirState = false;
      pumpNutrisiState = false;
    }
  }

  // === Tampilkan ke LCD ===
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tinggi: ");
  lcd.print(tinggi, 1);
  lcd.print(" cm");

  lcd.setCursor(0, 1);
  lcd.print("TDS: ");
  lcd.print(ppm);
  lcd.print(" ppm");

  lcd.setCursor(0, 2);
  lcd.print("Air: ");
  lcd.print(pumpAirState ? "ON " : "OFF");

  lcd.setCursor(0, 3);
  lcd.print("Nutrisi: ");
  lcd.print(pumpNutrisiState ? "ON" : "OFF");

  delay(2000);
}

