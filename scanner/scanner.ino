#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define BEACON_UUID "8ec76ea3-6668-48da-9866-75be8bc86f4d"  // Ваш UUID
#define SCAN_TIME 5  // Секунд на один цикл сканирования

// Little-endian байты UUID для сравнения (из вашего beacon)
const uint8_t targetUUID[16] = {0x4D, 0x6F, 0xC8, 0x8B, 0xBE, 0x75, 0x66, 0x98, 
                                0xDA, 0x48, 0x68, 0x66, 0xA3, 0x6E, 0xC7, 0x8E};

BLEScan* pBLEScan;
int currentRSSI[4] = {0, 0, 0, 0};  // Текущие RSSI для маячков 1-3 (0 = не найден)

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Проверяем manufacturer data (iBeacon в нём)
    String manufacturerData = advertisedDevice.getManufacturerData();
    if (manufacturerData.length() < 25) return;  // Минимум для iBeacon

    const uint8_t* data = (const uint8_t*)manufacturerData.c_str();
    
    // iBeacon: 4C00 0215 [UUID 16b] [major 2b] [minor 2b] [power 1b]
    if (data[0] == 0x4C && data[1] == 0x00 && data[2] == 0x02 && data[3] == 0x15) {
      // Сравниваем UUID (байты 4-19)
      bool uuidMatch = true;
      for (int i = 0; i < 16; i++) {
        if (data[4 + i] != targetUUID[i]) {
          uuidMatch = false;
          break;
        }
      }
      if (!uuidMatch) return;

      // Извлекаем major (байты 20-21, big-endian)
      uint16_t major = (data[20] << 8) | data[21];
      // Minor (байты 22-23, big-endian)
      uint16_t minor = (data[22] << 8) | data[23];

      // ID на основе major/minor (предполагаем major == minor)
      int id = (major == minor && major >= 1 && major <= 3) ? major : 0;
      if (id == 0) return;  // Не наш beacon

      // Обновляем RSSI (берём максимальный/ближайший за цикл, но для простоты — последний)
      currentRSSI[id] = advertisedDevice.getRSSI();
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 BLE iBeacon Scanner starting...");

  BLEDevice::init("ESP32 Scanner");
  pBLEScan = BLEDevice::getScan();  // Создаём сканер
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  // Активный скан (запрашивает дополнительные данные)
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // Почти 100% duty cycle
  Serial.println("Scan started...");
}

void loop() {
  // Сканируем на SCAN_TIME секунд
  pBLEScan->start(SCAN_TIME, false);  // false = не печатать summary
  
  // Выводим циклично информацию о маячках
  Serial.println("--- Scan cycle ---");
  for (int id = 1; id <= 3; id++) {
    if (currentRSSI[id] != 0) {
      Serial.printf("Beacon %d: RSSI %d dBm\n", id, currentRSSI[id]);
    } else {
      Serial.printf("Beacon %d: RSSI 0 dBm (не обнаружен)\n", id);
    }
  }
  Serial.println();  // Пустая строка для читаемости
  
  delay(1000);  // Пауза 1 сек перед следующим циклом (опционально, чтобы не слишком часто)
}