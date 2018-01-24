const int fsrPin = 32;     // the FSR and 10K pulldown are connected to a0
const int LED = 2; //Denkbeeldige LED (kunnen we gebruiken voor datum te ontvangen)
int fsrReading;     // the analog reading from the FSR resistor divider
int TUSSENTIJDSlist[20];
//int TUSSENTIJDSlistlength = 0;
//int lijst[10];
//int zitStaanList[20];
//int zitStaanListlength = 0;


int TeVersturenWaarde;
int VorigeVerstuurdeWaarde;
String tijdstip;

//Voor de tijd
#include <TimeLib.h>

//Voor bluetooth
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//BLE characteristics
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;

//Definiëren van de service en characteristics
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

//Definiëren van de tijd
#define TIME_HEADER  "T"   // Header tag for serial time sync message

//Deze tijd moet meegegeven worden door de Xamarin App (WRITE?) --> valt weg
long Ttime = 0; // hier komt het aantal seconden die nico berekent. Nu - (0:00:00 01 01 1970) + 3600 (--> tijdzone)

//Klasse voor de servercallbacks (Device verbonden of niet)
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
  // Debugging informatie via de Serial monitor
  Serial.begin(9600);
  setTime(Ttime);

  //HIERNA VOLGT ALLEMAAL CODE VOOR DE BLUETOOTH, ONDERANDERE SERVER, SERVICE, CHARACTERISTICS

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
  Serial.println("Wachten op een client connection om notify te versturen...");
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
    // DIT MOET JE DOORSTUREN: TeVersturenWaarde, (tijdstip)
    // in plaats van de print stuur je (ook) door
    Serial.println("Te versturen waarde:" + TeVersturenWaarde);
    Serial.println("Tijdstip: " + tijdstip);


    VorigeVerstuurdeWaarde = TeVersturenWaarde;
    countStaan = 0;


  //==================================================================
    //BLE verzenden
  
    if (deviceConnected) {
        //Als de device verbonden is geven we de te versturen waarde mee om deze te versturen
        //txValue = analogRead(readPin) // Sensor reading!
        txValue = TeVersturenWaarde;
    
        // Conversie van de waarde
        char txString[8];
        dtostrf(txValue, 1, 2, txString);
        
      //pCharacteristic->setValue(&txValue, 1); // integer value
      //pCharacteristic->setValue("Hello!"); // Test bericht
        pCharacteristic->setValue(txString);
        
        pCharacteristic->notify(); // Verzenden van de waarde naar de app!
        Serial.print("Verzonden waarde: ");
        Serial.print(txString);
      }
    }
  
}

//================================================================


//Hier komt de tijdberekening
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
