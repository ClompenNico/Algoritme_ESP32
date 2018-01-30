const int fsrPin = 32;     // The FSR and 10K pulldown are connected to pin 32
int fsrReading; // The analog reading from the FSR resistor
int TUSSENTIJDSlist[20];

//int -> ToSendValue and PreviousValue
int TeVersturenWaarde;
int VorigeVerstuurdeWaarde;
String tijdstip;

//For Time
#include <TimeLib.h>

//For Bluetooth Low Energy
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//BLE characteristics
BLECharacteristic *pCharacteristic;

//bool for Device connected or not
bool deviceConnected = false;
float txValue = 0;

//Define the service and characteristics
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

#define TIME_HEADER  "T"   // Header tag for serial time sync message

//Class for the servercallbacks (Device connected or not)
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

//================================================================

void setup(void) {
  // Debugging information via the Serial monitor
  Serial.begin(9600);

  //CODE FOR THE BLUETOOTH, SERVER, SERVICE, CHARACTERISTICS

  // Create the BLE Device
  BLEDevice::init("Imani tracker 0001"); // Bluetooth naamgeving

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");
}

void loop(void) {
  if (Serial.available()) {
    int inByte = Serial.read();
    Serial.print(inByte, inByte);

  }
  //  TUSSENTIJDSlistlength = 0;
  for (int i = 0; i < 20; i++) {
    fsrReading = analogRead(fsrPin);
    //Serial.print(String(fsrReading));

    if (fsrReading < 700) {
      //Serial.println(" - zitten ");
      TUSSENTIJDSlist[i] = 1;

    }
    else if (fsrReading > 700) {
      //Serial.println(" - staan");
      TUSSENTIJDSlist[i] = 2;

    }
    delay(500);
  }

  if (timeStatus() != timeNotSet) {
    //digitalClockDisplay();
    tijdstipToString();
  }

  int countStaan = 0;

  for (int i = 0; i < 20; i++) {
    if (TUSSENTIJDSlist[i] == 2) {
      countStaan += 1;
    }
  }


  if (countStaan < 5) {
    //Serial.println("aantal staan:" + String(countStaan));
    TeVersturenWaarde = 0;

  } else {
    //Serial.println("aantal staan:" + String(countStaan));
    TeVersturenWaarde = 1;

  }


  if (VorigeVerstuurdeWaarde != TeVersturenWaarde) {
    // THIS VALUE HAS TO BE SEND: TeVersturenWaarde
    // in plaats van de print stuur je (ook) door
    Serial.println("Value to send:" + TeVersturenWaarde);
    Serial.println("Time: " + tijdstip);

    VorigeVerstuurdeWaarde = TeVersturenWaarde;
    countStaan = 0;

  //==================================================================
    //BLE send
  
    if (deviceConnected) {
        //If the device is connected da data will be sent
        //txValue = analogRead(readPin) // Sensor reading!
        txValue = TeVersturenWaarde;
    
        // Conversion from the value
        char txString[8];
        dtostrf(txValue, 1, 2, txString);
        
      //pCharacteristic->setValue(&txValue, 1); // Integer value
      //pCharacteristic->setValue("Hello!"); // Test message
        pCharacteristic->setValue(txString);
        
        pCharacteristic->notify(); // Sending from the value to the app!
        Serial.print("Sent value: ");
        Serial.print(txString);
      }
    }
  
}

//================================================================


//Time calculation
void tijdstipToString() {

  String seconde;
  String minuut;
  String uur;
  String dag;
  String maand;

  if (second() < 10) {
    seconde = "0" + String(second());
  } else {
    seconde = second();
  }
  if (minute() < 10) {
    minuut = "0" + String(minute());
  } else {
    minuut = minute();
  }
  if (hour() < 10) {
    uur = "0" + String(hour());
  } else {
    uur = hour();
  }
  if (day() < 10) {
    dag = "0" + String(day());
  } else {
    dag = day();
  }
  if (month() < 10) {
    maand = "0" + String(month());
  } else {
    maand = month();
  }

  tijdstip = String(dag + "/" + maand + "/") + String(year()) + " ";
  tijdstip += String(uur + ":" + minuut + ":" + seconde);

  //Serial.println("tijdstip: " + tijdstip);


}
