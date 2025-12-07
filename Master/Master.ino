/*
@mosesjaguar
Master - Robot Santa
*/

#include "Global.h"
#include <esp_now.h>
#include <WiFi.h>

// ================= KONFIGURASI ESP-NOW =================

// Alamat MAC penerima (Ganti FF:FF:... dengan MAC Address robot/receiver Anda)
// FF:FF:FF:FF:FF:FF artinya BROADCAST (dikirim ke semua ESP32 yang mendengarkan)
// 68:25:DD:E7:E1:04
uint8_t broadcastAddress[] = {0x68, 0x25, 0xDD, 0xE7, 0xE1, 0x04};

// Struktur data pesan (Harus SAMA PERSIS dengan di sisi Penerima)
typedef struct struct_message {
  char flag;      // 'M' untuk Motion, 'D' untuk Command/Button
  char cmd;       // 'a', 'b', 'c', 'd' (hanya terisi jika flag 'D')
  int x;          // Data X
  int y;          // Data Y
  int z;          // Data Z
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback saat data terkirim (Opsional, untuk debug)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Jika ingin melihat status pengiriman, uncomment baris bawah:
  // Serial.print("\r\nStatus Paket: ");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Sukses" : "Gagal");
}

// ================= END KONFIGURASI ESP-NOW =================

// ----------------- Main -------------------
void processGamepad(ControllerPtr ctl) {
  // -------------- Define Button ---------------- //
  bool Xpressed = ctl->buttons() == 0x0001;        // a
  bool TrianglePressed = ctl->buttons() == 0x0008; // b
  bool CirclePressed = ctl->buttons() == 0x0002;   // c
  bool SquarePressed = ctl->buttons() == 0x0004;   // d

  int joystickY_value = ctl->axisY();
  int joystickX_value = ctl->axisX();
  int joystickRY_value = ctl->axisRY(); // Tidak dipakai di logic mapping?
  int joystickRX_value = ctl->axisRX(); // Sepertinya duplikat dengan joystickX_value di kode asli Anda?

  // ----------- Control (Mapping) ----------------
  // Pastikan fungsi exponentialMapping ada di Motion.h atau di sini
  int x = exponentialMapping(joystickX_value, 140.0);
  int y = exponentialMapping(joystickY_value, 140.0);
  int z = exponentialMapping(joystickRX_value, 140.0);

  // ----------- PENGIRIMAN ESP-NOW (BUTTONS / Flag D) -----------
  // Kita kirim data jika tombol ditekan
  if (Xpressed) {
    myData.flag = 'D';
    myData.cmd = 'a';
    myData.x = 0; myData.y = 0; myData.z = 0; // Kosongkan nilai motor
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    // Beri sedikit delay agar tidak spamming paket terlalu cepat saat ditekan
    delay(10); 
  }
  
  if (TrianglePressed) {
    myData.flag = 'D';
    myData.cmd = 'b';
    myData.x = 0; myData.y = 0; myData.z = 0;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    delay(10);
  }

  if (CirclePressed) {
    myData.flag = 'D';
    myData.cmd = 'c';
    myData.x = 0; myData.y = 0; myData.z = 0;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    delay(10);
  }

  if (SquarePressed) {
    myData.flag = 'D';
    myData.cmd = 'd';
    myData.x = 0; myData.y = 0; myData.z = 0;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    delay(10);
  }

  // ----------- PENGIRIMAN ESP-NOW (MOTION / Flag M) -----------
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime > sendInterval) {
    myData.flag = 'M';
    myData.cmd = ' '; 
    myData.x = x;
    myData.y = y;
    myData.z = z;

    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    lastSendTime = currentTime; 
  }

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
}

// ---------------- Which controller do u use? ----------------
void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      if (myController->isGamepad()) {
        processGamepad(myController);
      } else {
        Serial.println("Unsupported controller");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // --- Setup ESP-NOW ---
  WiFi.mode(WIFI_STA); // ESP-NOW butuh mode Station
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register Peer (Penerima)
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // --- End Setup ESP-NOW ---

  Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
  const uint8_t* addr = BP32.localBdAddress();
  Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys();
  BP32.enableVirtualDevice(false);
}

void loop() {
  bool dataUpdated = BP32.update();
  if (dataUpdated) {
    processControllers();
  }
}