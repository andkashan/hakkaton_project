#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

double rssi[3];
double bx[3], by[3];
double txp[3] = {
  -54.6, // Для beacon 1
  -54.6, // Для beacon 2
  -54.6
};
double n = 2.0; // стандартное значение; можно изменить

double rssi_to_distance(double rssi, double txPower, double n) {
    
    return pow(10.0, (txPower - rssi) / (10.0 * n));
}


int trilaterate(double x1,double y1,double r1,
                double x2,double y2,double r2,
                double x3,double y3,double r3,
                double *out_x, double *out_y) {
    double A = -2.0*x1 + 2.0*x2;
    double B = -2.0*y1 + 2.0*y2;
    double C = r1*r1 - r2*r2 - x1*x1 + x2*x2 - y1*y1 + y2*y2;

    double D = -2.0*x2 + 2.0*x3;
    double E = -2.0*y2 + 2.0*y3;
    double F = r2*r2 - r3*r3 - x2*x2 + x3*x3 - y2*y2 + y3*y3;

    double denom = A * E - B * D;
    if (fabs(denom) < 1e-9) return 1; // вырожденный случай

    *out_x = (C * E - B * F) / denom;
    *out_y = (A * F - C * D) / denom;
    return 0;
}



// Тот же самый UUID, что и в коде для маячков
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

int scanTime = 5; // Время сканирования в секундах
BLEScan* pBLEScan;

// Класс для обработки найденных устройств
  class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    public:
    MyAdvertisedDeviceCallbacks(double* rssi, unsigned int count) :BLEAdvertisedDeviceCallbacks() {
      if(!rssi) 
        throw("No rssi array assigned!");
      
      _rssi = rssi;
      _count = count;
    }


    void onResult(BLEAdvertisedDevice advertisedDevice) {
      // Проверяем, есть ли у найденного устройства наш сервисный UUID
      if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
        Serial.print("Found our Beacon: ");
        // Выводим имя устройства
        String devName = advertisedDevice.getName();
        const char* name = devName.c_str();
        
        Serial.print(name);
        // Выводим мощность сигнала (RSSI)
        Serial.print(" | RSSI: ");
        double rssi_b = (double)advertisedDevice.getRSSI();
        Serial.println(rssi_b);
        
        // Первый байт в названии маяка - его номер
        uint b_count = (uint8_t) name[0] - (uint8_t)'0' - 1;
        Serial.println(b_count);
        if(b_count < _count) 
          _rssi[b_count] = rssi_b;

      }
    }

    private:
      double* _rssi;
      unsigned int _count;
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning for BLE Beacons...");


  // Инициализация BLE
  BLEDevice::init("");
  
  // Получаем указатель на объект для сканирования
  pBLEScan = BLEDevice::getScan();
  // Устанавливаем callback-функцию, которая будет вызываться при нахождении устройства
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks((double*)rssi, 3));
  // Включаем активное сканирование
  pBLEScan->setActiveScan(true);
  // Устанавливаем интервал сканирования
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {


  

  // Запускаем сканирование на 'scanTime' секунд
  // The start function returns a pointer, so we store it in a pointer variable.
  BLEScanResults* foundDevices = pBLEScan->start(scanTime, false);
  
  Serial.print("Scan done! Found ");
  // We use the -> operator to access members of an object through a pointer.
  Serial.print(foundDevices->getCount());
  Serial.println(" devices.");
  Serial.println("---------------------------------");
  

  double dist[3];
  for (int i = 0; i < 3; ++i) {
        dist[i] = rssi_to_distance(rssi[i], txp[i], n);
        printf("\nDistation to beacon %d: %.3f м (RSSI=%.1f dBm, TxPower=%.1f dBm)\n",
               i+1, dist[i], rssi[i], txp[i]);
    }


    double px, py;
    int ok = trilaterate(bx[0],by[0],dist[0],
                         bx[1],by[1],dist[1],
                         bx[2],by[2],dist[2],
                         &px, &py);
    if (ok == 0) {
        printf("\nCords: X = %.3f, Y = %.3f\n", px, py);
    } else {
        printf("\nТрилатерация неудачна: вырожденная конфигурация маяков либо числовая ошибка.\n");
        printf("Попробуйте изменить координаты маяков или проверьте измерения.\n");
    }
  // Очищаем результаты для следующего сканирования
  pBLEScan->clearResults();
  
  // Ждем перед следующим сканированием
  delay(1);
}