#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>  // Только для advertising, без Server/Beacon

#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"  // Ваш UUID как строка (не используется напрямую)
#define BEACON_MAJOR 3
#define BEACON_MINOR 3

// Raw iBeacon manufacturer data (25 bytes: company + type + UUID + major + minor + power)
static const uint8_t beaconPayload[25] = {
  0x4C, 0x00,  // Apple company ID
  0x02, 0x15,  // iBeacon type
  // UUID (16 bytes, little-endian)
  0x4D, 0x6F, 0xC8, 0x8B, 0xBE, 0x75, 0x66, 0x98, 
  0xDA, 0x48, 0x68, 0x66, 0xA3, 0x6E, 0xC7, 0x8E,
  (BEACON_MAJOR >> 8) & 0xFF, BEACON_MAJOR & 0xFF,  // Major (big-endian)
  (BEACON_MINOR >> 8) & 0xFF, BEACON_MINOR & 0xFF,  // Minor
  0xC5  // Tx Power (-59 = 0xC5 signed)
};

// AD structure for manufacturer data: len (0x1A=26) + type (FF) + payload (25 bytes)
static uint8_t manufData[27];  // Убрал const, чтобы можно было memcpy

BLEAdvertising *pAdvertising;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 iBeacon starting...");

  // Инициализируем manufData
  manufData[0] = 0x1A;  // Length (26 bytes follow)
  manufData[1] = 0xFF;  // Type: Manufacturer Specific Data
  
  // Копируем payload в manufData[2..26]
  memcpy(&manufData[2], beaconPayload, 25);

  BLEDevice::init("ESP32 iBeacon 3");  // Имя устройства (не используется в beacon)

  // Настройка advertising
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setMinPreferred(0x06);  // Интервал 0.625*6=3.75ms
  pAdvertising->setMaxPreferred(0x12);  // 0.625*18=11.25ms

  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setFlags(0x04);  // BR/EDR not supported
  oAdvertisementData.addData((char*)manufData, 27);  // Raw bytes с length

  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->start();
  Serial.println("iBeacon advertising started!");

  // Debug: Print raw data
  Serial.print("Raw manuf data: ");
  for (int i = 0; i < 27; i++) {
    Serial.printf("%02X ", manufData[i]);
  }
  Serial.println();
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
}

void loop() {
  delay(1000);
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());  // Мониторинг
}