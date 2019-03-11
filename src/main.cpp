#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "033214d2-0ff0-4cba-814e-c5074c1ad00c"
#define CHARACTERISTIC_UUID "ac6744a7-77f3-43e9-b3c8-9955ac6bb0d4"

static BLECharacteristic *characteristic;
static BLEAdvertising *advertising;

uint8_t devicesConnected = 0;

class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
        Serial.println("onWrite");
        std::string value = characteristic->getValue();

        Serial.println(value.c_str());
    }

    void onRead(BLECharacteristic *characteristic) {
        Serial.println("onRead");
        characteristic->setValue("Hello");
    }
};

class ServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *server) {
        Serial.println("Client connected");
        devicesConnected++;
        advertising->start();
    }

    void onDisconnect(BLEServer *server) {
        Serial.println("Client disconnected");
        devicesConnected--;
    }
};

void setup() {
    Serial.begin(115200);

    BLEDevice::init("ESP32");
    BLEServer *server = BLEDevice::createServer();

    server->setCallbacks(new ServerCallbacks());

    BLEService *service = server->createService(SERVICE_UUID);

    characteristic = service->createCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE);
    characteristic->addDescriptor(new BLE2902());
    characteristic->setCallbacks(new CharacteristicCallbacks());

    service->start();

    advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->setScanResponse(false);
    advertising->setMinPreferred(0x0);
    advertising->start();

    Serial.println("Waiting for clients to connect...");
}

void loop() {
    if (devicesConnected > 0) {
        Serial.println("Notifying devices");
        characteristic->setValue("Hello connected devices!");
        characteristic->notify();
    }
    delay(1000);
}
