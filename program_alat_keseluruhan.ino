#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// === Konfigurasi DHT22 ===
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// === LCD 20x4 I2C ===
LiquidCrystal_I2C lcd(0x27, 20, 4);

// === Relay Aktuator ===
#define RELAY_KIPAS 8
#define RELAY_POMPA_KELEMBAPAN 9
#define RELAY_POMPA_AIR 10
#define RELAY_POMPA_NUTRISI 11

// === Batasan Kendali ===
#define SUHU_NYALA 26.0
#define KELEMBAPAN_MIN 88.0
#define PPM_TINGGI 1100
#define PPM_RENDAH 800
#define KETINGGIAN_MIN 5.0
#define KETINGGIAN_MAX 10.0

// === Timer Pompa Kelembapan ===
unsigned long intervalPompa = 10UL * 60UL * 1000UL;
unsigned long durasiPompa = 1UL * 60UL * 1000UL;
unsigned long waktuSebelum = 0;
bool pompaTimerAktif = false;

// === Status Isi Ulang ===
bool statusIsiUlang = false;

int bacaTDS() {
  int sensorValue = analogRead(A0);
  float voltage = sensorValue * (5.0 / 1023.0);
  float tdsValue = (voltage - 0.015) / 0.0011;
  if (tdsValue < 0) tdsValue = 0;
  return (int)tdsValue;
}

float bacaKetinggian() {
  long duration;
  float distance;

  digitalWrite(6, LOW);
  delayMicroseconds(2);
  digitalWrite(6, HIGH);
  delayMicroseconds(10);
  digitalWrite(6, LOW);
  duration = pulseIn(7, HIGH);
  distance = duration * 0.034 / 2;

  float tinggiMax = 32.0;
  return tinggiMax - distance;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.init();
  lcd.backlight();

  pinMode(RELAY_KIPAS, OUTPUT);
  pinMode(RELAY_POMPA_KELEMBAPAN, OUTPUT);
  pinMode(RELAY_POMPA_AIR, OUTPUT);
  pinMode(RELAY_POMPA_NUTRISI, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, INPUT);

  digitalWrite(RELAY_KIPAS, HIGH);
  digitalWrite(RELAY_POMPA_KELEMBAPAN, HIGH);
  digitalWrite(RELAY_POMPA_AIR, HIGH);
  digitalWrite(RELAY_POMPA_NUTRISI, HIGH);

  waktuSebelum = millis();
}

void loop() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();
  int ppm = bacaTDS();
  float tinggi = bacaKetinggian();
  unsigned long waktuSekarang = millis();

  // === Kontrol kipas ===
  digitalWrite(RELAY_KIPAS, suhu > SUHU_NYALA ? LOW : HIGH);

  // === Pompa kelembapan (dengan timer & sensor) ===
  bool pompaSensor = (kelembapan < KELEMBAPAN_MIN);

  if (!pompaTimerAktif && waktuSekarang - waktuSebelum >= intervalPompa) {
    pompaTimerAktif = true;
    waktuSebelum = waktuSekarang;
  }

  if (pompaTimerAktif && waktuSekarang - waktuSebelum < durasiPompa) {
    digitalWrite(RELAY_POMPA_KELEMBAPAN, LOW);
  } else if (pompaTimerAktif && waktuSekarang - waktuSebelum >= durasiPompa) {
    pompaTimerAktif = false;
    waktuSebelum = waktuSekarang;
    digitalWrite(RELAY_POMPA_KELEMBAPAN, pompaSensor ? LOW : HIGH);
  } else {
    digitalWrite(RELAY_POMPA_KELEMBAPAN, pompaSensor ? LOW : HIGH);
  }

  // === Logika Pompa Air & Nutrisi ===
  if (tinggi <= 0.0) {
    // Ketinggian 0 cm → kedua pompa mati
    digitalWrite(RELAY_POMPA_AIR, HIGH);
    digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
    statusIsiUlang = false;
  } 
  else if (tinggi >= KETINGGIAN_MAX) {
    // Ketinggian >= 10 cm → kedua pompa mati
    digitalWrite(RELAY_POMPA_AIR, HIGH);
    digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
    statusIsiUlang = false;
  } 
  else {
    // Tinggi < 10 cm & > 0 cm
    if (tinggi < KETINGGIAN_MIN) {
      statusIsiUlang = true;
    }

    if (statusIsiUlang) {
      // Isi ulang sampai tinggi >= 10 cm
      digitalWrite(RELAY_POMPA_AIR, LOW);
      digitalWrite(RELAY_POMPA_NUTRISI, LOW);
    } else {
      // Tinggi antara 5–10 cm → kontrol PPM
      if (ppm > PPM_TINGGI) {
        // PPM terlalu tinggi → Air ON
        digitalWrite(RELAY_POMPA_AIR, LOW);
        digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
      } 
      else if (ppm < PPM_RENDAH) {
        // PPM terlalu rendah → Nutrisi ON
        digitalWrite(RELAY_POMPA_AIR, HIGH);
        digitalWrite(RELAY_POMPA_NUTRISI, LOW);
      } 
      else {
        // PPM dalam rentang → Air ON dulu
        digitalWrite(RELAY_POMPA_AIR, LOW);
        digitalWrite(RELAY_POMPA_NUTRISI, HIGH);
      }
    }
  }

  // === Tampilkan di LCD ===
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu:");
  lcd.print(suhu, 1);
  lcd.print("C ");
  lcd.print(digitalRead(RELAY_KIPAS) == LOW ? "Fan ON" : "Fan OFF");

  lcd.setCursor(0, 1);
  lcd.print("Hum:");
  lcd.print(kelembapan, 1);
  lcd.print("% ");
  lcd.print(digitalRead(RELAY_POMPA_KELEMBAPAN) == LOW ? "Pump ON" : "Pump OFF");

  lcd.setCursor(0, 2);
  lcd.print("PPM:");
  lcd.print(ppm);
  lcd.print(" ");
  lcd.print(digitalRead(RELAY_POMPA_AIR) == LOW ? "Air ON" : "Air OFF");

  lcd.setCursor(0, 3);
  lcd.print("Tinggi:");
  lcd.print(tinggi, 1);
  lcd.print("cm ");
  lcd.print(digitalRead(RELAY_POMPA_NUTRISI) == LOW ? "Nut ON" : "Nut OFF");

  delay(2000);
}
