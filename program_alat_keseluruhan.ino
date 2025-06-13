#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Sensor DHT22 ===
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === LCD 20x4 ===
LiquidCrystal_I2C lcd(0x27, 20, 4);

// === Relay ===
#define RELAY_KIPAS 8
#define RELAY_POMPA_KELEMBAPAN 9
#define RELAY_POMPA_AIR 10
#define RELAY_POMPA_NUTRISI 11

// === Sensor JSN-SR04T (Ultrasonik) ===
#define TRIG_PIN 5
#define ECHO_PIN 6

// === Sensor TDS ===
#define TDS_PIN A0

// === Batasan ===
#define SUHU_NYALA 28.0
#define KELEMBAPAN_MIN 80.0
#define TINGGI_MIN_CM 5
#define TINGGI_MAKS_CM 10
#define PPM_MAX 1100
#define PPM_MIN 800

// === Status ===
bool fanState = false;
bool pumpHumidityState = false;
bool pumpAirState = false;
bool pumpNutrisiState = false;

// === Timer untuk Pompa Kelembapan Otomatis ===
unsigned long lastHumidityPumpTime = 0;
const unsigned long intervalPompaHumidity = 15UL * 60UL * 1000UL; // 15 menit
const unsigned long durasiPompaHumidity = 1UL * 60UL * 1000UL;    // 1 menit
bool pompaHumidityTimerAktif = false;

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.begin(20, 4);
  lcd.backlight();

  pinMode(RELAY_KIPAS, OUTPUT);
  pinMode(RELAY_POMPA_KELEMBAPAN, OUTPUT);
  pinMode(RELAY_POMPA_AIR, OUTPUT);
  pinMode(RELAY_POMPA_NUTRISI, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(RELAY_KIPAS, HIGH);
  digitalWrite(RELAY_POMPA_KELEMBAPAN, HIGH);
  digitalWrite(RELAY_POMPA_AIR, HIGH);
  digitalWrite(RELAY_POMPA_NUTRISI, HIGH);

  lcd.setCursor(0, 0);
  lcd.print("Sistem Nutrisi &");
  lcd.setCursor(0, 1);
  lcd.print("Lingkungan Aktif");
  delay(2000);
}

float bacaKetinggianCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long durasi = pulseIn(ECHO_PIN, HIGH);
  float jarakCM = durasi * 0.034 / 2.0;
  float tinggiMax = 30.0;
  float tinggiAir = tinggiMax - jarakCM;
  return max(0.0, min(tinggiAir, tinggiMax));
}

int bacaTDS() {
  int analogValue = analogRead(TDS_PIN);
  float voltage = analogValue * (5.0 / 1023.0);
  int ppm = (voltage - 0.015) / 0.0011;
  return ppm;
}

void loop() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  float tinggi = bacaKetinggianCM();
  int ppm = bacaTDS();

  // === Kipas (suhu)
  if (suhu > SUHU_NYALA && !fanState) {
    digitalWrite(RELAY_KIPAS, LOW);
    fanState = true;
  } else if (suhu <= SUHU_NYALA && fanState) {
    digitalWrite(RELAY_KIPAS, HIGH);
    fanState = false;
  }

  // === Pompa kelembapan berdasarkan kelembapan sensor
  if (kelembapan < KELEMBAPAN_MIN && !pumpHumidityState && !pompaHumidityTimerAktif) {
    digitalWrite(RELAY_POMPA_KELEMBAPAN, LOW);
    pumpHumidityState = true;
  } else if (kelembapan >= KELEMBAPAN_MIN && pumpHumidityState && !pompaHumidityTimerAktif) {
    digitalWrite(RELAY_POMPA_KELEMBAPAN, HIGH);
    pumpHumidityState = false;
  }

  // === Pompa kelembapan otomatis per 15 menit selama 1 menit
  unsigned long currentMillis = millis();
  if (!pompaHumidityTimerAktif && (currentMillis - lastHumidityPumpTime >= intervalPompaHumidity)) {
    digitalWrite(RELAY_POMPA_KELEMBAPAN, LOW); // Hidupkan pompa
    pompaHumidityTimerAktif = true;
    lastHumidityPumpTime = currentMillis;
    pumpHumidityState = true;
  }

  if (pompaHumidityTimerAktif && (currentMillis - lastHumidityPumpTime >= durasiPompaHumidity)) {
    digitalWrite(RELAY_POMPA_KELEMBAPAN, HIGH); // Matikan pompa
    pompaHumidityTimerAktif = false;
    pumpHumidityState = false;
  }

  // === Kontrol nutrisi dan air
  if (tinggi > TINGGI_MAKS_CM) {
    digitalWrite(RELAY_POMPA_AIR, HIGH);
    digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
    pumpAirState = false;
    pumpNutrisiState = false;
  } else {
    if ((tinggi < TINGGI_MIN_CM) || (ppm > PPM_MAX)) {
      digitalWrite(RELAY_POMPA_AIR, LOW);
      pumpAirState = true;
    } else {
      digitalWrite(RELAY_POMPA_AIR, HIGH);
      pumpAirState = false;
    }

    if (ppm < PPM_MIN) {
      digitalWrite(RELAY_POMPA_NUTRISI, LOW);
      pumpNutrisiState = true;
    } else {
      digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
      pumpNutrisiState = false;
    }
  }

  // === TAMPILKAN KE LCD 20x4 ===
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(suhu, 1);
  lcd.print("C ");
  lcd.print(fanState ? "Fan ON" : "Fan OFF");

  lcd.setCursor(0, 1);
  lcd.print("Hum : ");
  lcd.print(kelembapan, 1);
  lcd.print("% ");
  lcd.print(pumpHumidityState ? "Pompa ON" : "OFF");

  lcd.setCursor(0, 2);
  lcd.print("Tinggi: ");
  lcd.print(tinggi, 1);
  lcd.print("cm ");

  lcd.setCursor(0, 3);
  lcd.print("TDS:");
  lcd.print(ppm);
  lcd.print("ppm A:");
  lcd.print(pumpAirState ? "ON " : "OFF");
  lcd.print("N:");
  lcd.print(pumpNutrisiState ? "ON" : "OFF");

  delay(2000);
}

