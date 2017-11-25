#include <Nefry.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>//これをいれるとBluetoothがうまくいく，なんでや，ちゃんと認識される

#include "DHT.h"
#define DHTPIN D2     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
uint8_t value = 0;

BLECharacteristic *pCharacteristic2;
int j = 0;
int red, green, blue;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
//serviceを複数作るときは複数のUUIDが必要
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//2つ目UUIDを設定
#define SERVICE_UUID2 "275cc101-1f61-4854-af72-57917900e866"
#define CHARACTERISTIC_UUID2 "e64bbdc5-196d-4198-ad89-71cdf05264d1"



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

//classがどういう機能をもつかを示す感じ
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic2) {
      std::string j = pCharacteristic2->getValue();

      randomSeed(analogRead(A0));

      if (j.length() > 0) {
        red = random(255);        //random関数は0-255の数値をランダムに返します。
        green = random(255);
        blue = random(255);
        Nefry.setLed(red, green, blue);
        delay(1000);
        Nefry.println("On");
      } else {
        Nefry.println("Off");
      }
    }
};
MyCallbacks myCallbacks;//名前を端的にわかりやすくした



void setup() {
  Nefry.setLed(0, 0, 0);
  dht.begin();
  randomSeed(analogRead(A0));

  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("MyESPWOW");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();//変更点
  pServer->setCallbacks(new MyServerCallbacks());//severにclassを渡す

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();
  ///////////////////////////////////////////////////////////////////////////////////////////
  // Create the BLE Service
  BLEService *pService2 = pServer->createService(SERVICE_UUID2);
  //serviceのどういうことをやるかをcharacteristic(特性)を指定する，READ，WRITEなど
  BLECharacteristic *pCharacteristic2 = pService2->createCharacteristic(
                                          CHARACTERISTIC_UUID2,
                                          BLECharacteristic::PROPERTY_READ |
                                          BLECharacteristic::PROPERTY_WRITE
                                        );
  pCharacteristic2->setCallbacks(&myCallbacks);//特性を指定したclassにもどす．機能の追加には＆をつける．classはpythonみたいな感じ．読み込んだ値なども格納されている．
  pCharacteristic2->setValue("Hello");

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml

  // Start the service
  pService2->start();


  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {

  Nefry.ndelay(1000);
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {//ここもいる
    Nefry.println("Failed to read from DHT sensor!");
    return;
  }

  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  if (deviceConnected) {

    Nefry.print("Temperature: ");
    Nefry.print(t);
    Nefry.print(" *C ");

    //温度が18℃を超えたらLEDが緑に光り，20℃を超えたら赤く光り，それ以外は青く光る

    if (t > 20) {
      boolean Flag = true;
      Nefry.print(Flag);
      Nefry.setLed(200, 0, 0, 50);
    } else if (t > 18) {
      boolean Flag = false;
      Nefry.print(Flag);
      Nefry.setLed(0, 200, 0, 50);
    } else {
      boolean Flag = false;
      Nefry.print(Flag);
      Nefry.setLed(0, 0, 200, 50);
    }
  }
  //delay(2000);
}
