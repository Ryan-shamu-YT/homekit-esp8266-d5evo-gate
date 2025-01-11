#include "HomeSpan.h"
#include "DEV_GATE.h"

const int statusPin = 39;  // The status pin connected to the gate
const int relayPin = 35; // The relay pin that actually controls the gate

void setup()
{
    Serial.begin(115200);

    homeSpan.enableWebLog(100,"pool.ntp.org","UTC+10",""); //  enableWebLog(uint16_t maxEntries, const char *timeServerURL, const char *timeZone, const char *logURL)`
    homeSpan.setWifiCredentials("", "");  // Set your Wi-Fi credentials
    homeSpan.setPairingCode("06533259");                                  // pairing pin
    // homeSpan.setControlPin(0);
    // homeSpan.setStatusPin(15);
    // homeSpan.setStatusAutoOff(10);

    // homeSpan.enableOTA(true);

    homeSpan.begin(Category::GarageDoorOpeners, "front gate");
    new SpanAccessory();
        new Service::AccessoryInformation();
            new Characteristic::Identify();
            new Characteristic::Name("front gate");
            new Characteristic::Manufacturer("f.mcgr.gr/g/HSgate");
            new Characteristic::Model("homespan-garage-gate");
            new Characteristic::FirmwareRevision("2.0.0");
            new Characteristic::CurrentDoorState();
            new Characteristic::TargetDoorState();
            new Characteristic::ObstructionDetected();

        new DEV_GATE(statusPin, relayPin); // statusPin, relayPin
        // new DEV_GATE(); 
}

void loop()
{
    homeSpan.poll();
}
