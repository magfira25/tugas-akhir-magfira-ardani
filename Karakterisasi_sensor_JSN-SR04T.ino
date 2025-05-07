#define TRIG_PIN 9   // Pin Trig sensor
#define ECHO_PIN 8   // Pin Echo sensor

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT); // Pin Trig sebagai output
  pinMode(ECHO_PIN, INPUT);  // Pin Echo sebagai input
  Serial.println("Pengukuran Dimulai...");
}

void loop() {
  long duration;
  float distance, outputDistance;

  // Mengirim pulsa ultrasonik
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Membaca durasi pantulan
  duration = pulseIn(ECHO_PIN, HIGH);

  // Menghitung jarak (cm)
  distance = (duration * 0.034) / 2;

  // Validasi jarak dan pengurangan 20 cm
  if (distance >= 20 && distance <= 400) {
    outputDistance = fabs(distance - 30); // Sesuaikan output jarak
    Serial.print("Jarak Sensor: ");
    Serial.print(distance, 2);
    Serial.print(" cm, Output Tampilan: ");
    Serial.print(outputDistance, 2);
    Serial.println(" cm");
  } else if (distance < 21) {
    Serial.println("Jarak terlalu dekat, tidak valid!");
  } else {
    Serial.println("Jarak di luar jangkauan sensor!");
  }

  delay(10000); // Tunggu 1 detik sebelum pembacaan berikutnya
}

