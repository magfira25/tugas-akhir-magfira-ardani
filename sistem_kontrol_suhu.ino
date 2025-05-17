#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ====== Konfigurasi DHT22 ======
#define DHTPIN 2          // Pin DATA dari DHT22 terhubung ke pin D2 Arduino
#define DHTTYPE DHT22     // Tipe sensor

DHT dht(DHTPIN, DHTTYPE);

// ====== Konfigurasi LCD I2C ======
LiquidCrystal_I2C lcd(0x27, 20, 4); // Alamat I2C (umum: 0x27 atau 0x3F)

// ====== Konfigurasi Relay ======
#define RELAYPIN 8        // Pin untuk mengontrol relay
bool fanState = false;    // Status kipas

// ====== Batas suhu ======
#define SUHU_NYALA 26.0   // Suhu batas untuk menyalakan kipas (dalam °C)

void setup() {
  Serial.begin(9600);
  dht.begin();

  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH); // Relay OFF (HIGH = mati jika relay aktif LOW)

  lcd.setCursor(0, 0);
  lcd.print("Sistem Kipas Siap");
  delay(2000);
}

void loop() {
  // Membaca suhu dan kelembapan
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  // Cek error pembacaan
  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca dari sensor DHT22!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!");
    delay(2000);
    return;
  }

  // Tampilkan di Serial Monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, Kelembapan: ");
  Serial.print(kelembapan);
  Serial.println(" %");

  // Tampilkan di LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Suhu: ");
  lcd.print(suhu, 1);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Kelembapan: ");
  lcd.print(kelembapan, 1);
  lcd.print("%");

  // Kontrol kipas melalui relay
  if (suhu > SUHU_NYALA && !fanState) {
    digitalWrite(RELAYPIN, LOW); // Relay ON
    fanState = true;
    Serial.println("Kipas AKTIF (ON)");
  } 
  else if (suhu <= SUHU_NYALA && fanState) {
    digitalWrite(RELAYPIN, HIGH); // Relay OFF
    fanState = false;
    Serial.println("Kipas NONAKTIF (OFF)");
  }

  delay(2000); // Delay 2 detik
}
