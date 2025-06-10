#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ====== Konfigurasi DHT22 ======
#define DHTPIN 2          // Pin DATA dari DHT22 ke pin D2 Arduino
#define DHTTYPE DHT22     // Tipe sensor DHT22
DHT dht(DHTPIN, DHTTYPE);

// ====== Konfigurasi LCD I2C ======
LiquidCrystal_I2C lcd(0x27, 16, 2); // Ganti alamat jika perlu

// ====== Konfigurasi Relay 2 Channel ======
#define RELAY_KIPAS 8     // Relay channel 1: Kipas
#define RELAY_POMPA 9     // Relay channel 2: Pompa
bool fanState = false;
bool pumpState = false;

// ====== Batasan ======
#define SUHU_NYALA 26.0     // Suhu batas kipas ON
#define KELEMBAPAN_MIN 80.0 // Pompa aktif jika < 80%

// ====== Timer untuk Pompa Berkala ======
unsigned long previousPumpInterval = 0;
const unsigned long pumpInterval = 15UL * 60UL * 1000UL; // 15 menit
const unsigned long pumpDuration = 60UL * 1000UL;        // 1 menit
bool timedPumpActive = false;
unsigned long timedPumpStart = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();

  lcd.init();
  lcd.backlight();

  pinMode(RELAY_KIPAS, OUTPUT);
  pinMode(RELAY_POMPA, OUTPUT);
  digitalWrite(RELAY_KIPAS, HIGH); // Kipas mati (relay aktif LOW)
  digitalWrite(RELAY_POMPA, HIGH); // Pompa mati

  lcd.setCursor(0, 0);
  lcd.print("Sistem Kontrol Suhu");
  lcd.setCursor(0, 1);
  lcd.print("dan Kelembapan Siap");
  delay(2000);
}

void loop() {
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca dari sensor DHT22!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    delay(2000);
    return;
  }

  // ====== Kontrol Kipas ======
  if (suhu > SUHU_NYALA && !fanState) {
    digitalWrite(RELAY_KIPAS, LOW); // Kipas ON
    fanState = true;
    Serial.println("Kipas AKTIF");
  } else if (suhu <= SUHU_NYALA && fanState) {
    digitalWrite(RELAY_KIPAS, HIGH); // Kipas OFF
    fanState = false;
    Serial.println("Kipas NONAKTIF");
  }

  // ====== Kontrol Pompa Berdasarkan Kelembapan (jika tidak sedang aktif dari timer) ======
  if (!timedPumpActive) {
    if (kelembapan < KELEMBAPAN_MIN) {
      digitalWrite(RELAY_POMPA, LOW); // Pompa ON
      if (!pumpState) {
        Serial.println("Pompa AKTIF (Kelembapan)");
        pumpState = true;
      }
    } else {
      digitalWrite(RELAY_POMPA, HIGH); // Pompa OFF
      if (pumpState) {
        Serial.println("Pompa NONAKTIF (Kelembapan)");
        pumpState = false;
      }
    }
  }

  // ====== Kontrol Pompa Berkala Setiap 15 Menit ======
  unsigned long currentMillis = millis();

  if (!timedPumpActive && (currentMillis - previousPumpInterval >= pumpInterval)) {
    previousPumpInterval = currentMillis;
    digitalWrite(RELAY_POMPA, LOW); // Pompa ON
    timedPumpStart = currentMillis;
    timedPumpActive = true;
    Serial.println("Pompa AKTIF (Timer)");
  }

  if (timedPumpActive && (currentMillis - timedPumpStart >= pumpDuration)) {
    digitalWrite(RELAY_POMPA, HIGH); // Pompa OFF
    timedPumpActive = false;
    Serial.println("Pompa NONAKTIF (Timer)");
  }

  // ====== Tampilkan ke Serial Monitor ======
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" C (");
  Serial.print(fanState ? "Kipas ON" : "Kipas OFF");
  Serial.print("), Kelembapan: ");
  Serial.print(kelembapan);
  Serial.print(" % (");
  Serial.print(digitalRead(RELAY_POMPA) == LOW ? "Pompa ON" : "Pompa OFF");
  Serial.println(")");

  // ====== Tampilkan ke LCD ======
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(suhu, 1);
  lcd.print("C ");
  lcd.print(fanState ? "ON" : "OFF");

  lcd.setCursor(0, 1);
  lcd.print("Kelembapan: ");
  lcd.print(kelembapan, 1);
  lcd.print("% ");
  lcd.print(digitalRead(RELAY_POMPA) == LOW ? "ON" : "OF");

  delay(2000);
}


