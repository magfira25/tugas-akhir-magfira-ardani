#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inisialisasi LCD dengan alamat 0x27, 16 kolom dan 2 baris
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin sensor TDS
const int tdsPin = A0;

// Variabel untuk menyimpan nilai pembacaan
float voltage;
float tdsValue;

void setup() {
  Serial.begin(9600); // Memulai komunikasi serial
  pinMode(tdsPin, INPUT); // Mengatur pin sensor sebagai input
  Serial.println("Sensor TDS siap!");

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("TDS Sensor Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Membaca tegangan dari sensor (dalam satuan volt)
  int sensorValue = analogRead(tdsPin);
  voltage = (sensorValue) * (5.0 / 1023); // Konversi nilai ADC ke volt

  // Menghitung nilai TDS (ppm) berdasarkan tegangan
  tdsValue = (voltage - 0.015) /0.0011 ; // Rumus TDS

  // Menampilkan hasil ke serial monitor
  Serial.print("Tegangan: ");
  Serial.print(voltage, 2);
  Serial.println(" V, TDS: "); 
  Serial.print(tdsValue, 2);
  Serial.println(" ppm");

  // Menampilkan hasil ke LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Volt: ");
  lcd.print(voltage, 2);
  lcd.print(" V");

  lcd.setCursor(0, 1);
  lcd.print("TDS: ");
  lcd.print(tdsValue, 2);
  lcd.print(" ppm");

  delay(15000); // Tunggu 15 detik sebelum pembacaan berikutnya
}
