#include <esp_now.h>
#include <WiFi.h>

// ================= KONFIGURASI =================

#define RXD2 16
#define TXD2 17

typedef struct struct_message {
  char flag;
  char cmd;
  int x;
  int y;
  int z;
} struct_message;

struct_message myData;

// ================= CALLBACK PENERIMA =================

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));

  // ---------------------------------------------------------
  // 1. KIRIM SEMUA DATA KE PC (SERIAL USB) UNTUK DEBUG/PYTHON
  // ---------------------------------------------------------
  if (myData.flag == 'M') {
    Serial.printf("M %d,%d,%d\n", myData.x, myData.y, myData.z);
  } else if (myData.flag == 'D') {
    Serial.printf("D %c\n", myData.cmd);
  }

  // ---------------------------------------------------------
  // 2. KIRIM HANYA 'M' KE ROBOT (SERIAL2)
  // ---------------------------------------------------------
  if (myData.flag == 'M') {
    Serial2.printf("M %d,%d,%d\n", myData.x, myData.y, myData.z);
  }
}

// ================= SETUP =================

void setup() {
  // Serial Utama ke PC
  Serial.begin(500000);

  // Serial2 ke Robot/Driver (Hanya TX yang krusial di sini)
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  Serial.println("DEBUG: Slave Siap. Menunggu Data...");
}

void loop() {
  // Kosong
}