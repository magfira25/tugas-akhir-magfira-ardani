#include <DHT.h>

// Pin untuk sensor DHT22
#define DHTPIN 2  // DHT22 terhubung ke pin digital 2
#define DHTTYPE DHT22  // Tipe sensor yang digunakan

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Memulai komunikasi serial
  Serial.println("Program Karakterisasi Sensor DHT22");

  dht.begin(); // Memulai sensor DHT22
}

void loop() {
  // Membaca data dari sensor
  float humidity = dht.readHumidity();    // Membaca kelembapan
  float temperature = dht.readTemperature(); // Membaca suhu dalam Celcius

  // Mengecek apakah pembacaan berhasil
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Gagal membaca data dari sensor DHT22");
    return;
  }

  // Menampilkan data di Serial Monitor
  Serial.print("Kelembapan: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Suhu: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(300000); // Tunggu 5 menit sebelum pembacaan berikutnya
}




