#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

// Уникальный идентификатор (UUID) для наших маячков.
// Сканер будет искать устройства именно с этим UUID.
// Можно сгенерировать свой UUID, например, на сайте https://www.uuidgenerator.net/
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

// !!! ИЗМЕНИТЬ ЭТО ИМЯ ДЛЯ КАЖДОГО МАЯЧКА !!!
const char* deviceName = "3_Beacon"; // Для второго "Beacon_2", для третьего "Beacon_3"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Beacon...");

  // Инициализация BLE с заданным именем
  BLEDevice::init(deviceName);

  // Создаем BLE сервер
  BLEServer *pServer = BLEDevice::createServer();

  // Создаем сервис с нашим уникальным UUID
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pService->start();

  // Настраиваем и запускаем трансляцию (advertising)
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06); // для iOS и Android
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.print("Beacon '");
  Serial.print(deviceName);
  Serial.println("' started.");
  Serial.println("Service UUID: " + String(SERVICE_UUID));
}

void loop() {
  // Код в loop не нужен, так как BLE работает в фоновом режиме

  delay(2000);
}