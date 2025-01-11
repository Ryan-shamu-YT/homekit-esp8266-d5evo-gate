struct DEV_GATE : Service::GarageDoorOpener { // A Garage Door Opener (but I am using it for a gate)

  int statusPin; // The status pin connected to the gate
  int relayPin;  // The relay pin of the microcontrller that actually controls the gate

  unsigned long lastChangeTime = 0;    // Time of the last pin state change
  unsigned long highDuration = 0;      // Duration the pin was HIGH
  unsigned long lowDuration = 0;       // Duration the pin was LOW
  unsigned long stableThreshold = 1000; // Time to confirm the gate is fully open/closed

  // Flashing intervals
  const unsigned long flashIntervalClosing = 150; // Interval for "Closing" (HIGH/LOW)
  const unsigned long flashIntervalOpening = 300; // Interval for "Opening" (HIGH/LOW)
  const unsigned long hysteresis = 50;            // Tolerance for timing variations

  int lastPinState = LOW;    // Last known pin state
  int currentPinState = LOW; // Current pin state
  int gateState = -1;        // Current gate state

  // Create a new enum for the gate states
    enum GateState {
      OPEN,       // 0
      CLOSED,     // 1
      OPENING,    // 2
      CLOSING,    // 3
      STOPPED     // 4
  };

  ////////////////////////////////////////////////

  SpanCharacteristic *current;     // reference to the Current Door State Characteristic (specific to Garage Door Openers)
  SpanCharacteristic *target;      // reference to the Target Door State Characteristic (specific to Garage Door Openers)
  SpanCharacteristic *obstruction; // reference to the Obstruction Detected Characteristic (specific to Garage Door Openers)

  DEV_GATE(int statusPin, int relayPin) : Service::GarageDoorOpener()
  { // constructor() method

    // Initialize the characteristics

    Serial.println("Configuring Garage Door Controller");

    this->relayPin = relayPin;
    this->statusPin = statusPin;

    pinMode(relayPin, OUTPUT);
    pinMode(statusPin, INPUT_PULLUP);

    current = new Characteristic::CurrentDoorState(Characteristic::CurrentDoorState::CLOSED);                 // Default state: Closed
    target = new Characteristic::TargetDoorState(Characteristic::TargetDoorState::CLOSED);                    // Default target state: Closed
    obstruction = new Characteristic::ObstructionDetected(Characteristic::ObstructionDetected::NOT_DETECTED); // Default: No obstruction
    obstruction->setVal(false);
  }

  bool update()
  {
    Serial.println("Update method called"); // Debug print to trace execution

    int targetState = target->getVal(); // Get the target state from HomeKit
    WEBLOG(("Target state requested: " + String(targetState)).c_str());
    Serial.println("Target state requested: " + String(targetState)); // Debug print to trace execution

    if (targetState == Characteristic::TargetDoorState::OPEN)
    {
      if (gateState == OPENING)
      {
        // Gate is already opening, do nothing
        WEBLOG("Gate is already opening");
        return true;
      }
      else if (gateState == CLOSING)
      {
        WEBLOG("Gate was closing; now opening");
        // Gate is closing, pulse the relay twice to reverse direction
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
        delay(500);
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      else
      {
        WEBLOG("Opening the gate");
        // Open the gate
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      if (current->getVal() != Characteristic::CurrentDoorState::OPEN) {
        current->setVal(Characteristic::CurrentDoorState::OPEN); // Update the current state
      }
      if (target->getVal() != Characteristic::TargetDoorState::OPEN) {
        target->setVal(Characteristic::TargetDoorState::OPEN);   // Confirm the target state
      }
      WEBLOG("Gate state updated to OPEN");
    }
    else if (targetState == Characteristic::TargetDoorState::CLOSED)
    {
      if (gateState == CLOSING)
      {
        // Gate is already closing, do nothing
        WEBLOG("Gate is already closing");
        return true;
      }
      else if (gateState == OPENING)
      {
        WEBLOG("Gate was opening; now closing");
        // Gate is opening, pulse the relay twice to reverse direction
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
        delay(500);
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      else
      {
        WEBLOG("Closing the gate");
        // Close the gate
        digitalWrite(relayPin, HIGH);
        delay(500);
        digitalWrite(relayPin, LOW);
      }
      if (current->getVal() != Characteristic::CurrentDoorState::CLOSED) {
        current->setVal(Characteristic::CurrentDoorState::CLOSED); // Update the current state
      }
      if (target->getVal() != Characteristic::TargetDoorState::CLOSED) {
        target->setVal(Characteristic::TargetDoorState::CLOSED);   // Confirm the target state
      }
      WEBLOG("Gate state updated to CLOSED");
    }

    return true; // return true
  }

  void loop()
  {
    // this is how we know the state of the gate

    int newPinState = digitalRead(statusPin); // Read the current pin state
    unsigned long currentTime = millis();     // Get the current time

    // Detect pin state changes
    if (newPinState != lastPinState)
    {
      unsigned long duration = currentTime - lastChangeTime; // Duration of the previous state
      lastChangeTime = currentTime;                          // Update the time of the change

      // Record HIGH/LOW durations
      if (newPinState == HIGH)
      {
        lowDuration = duration;
      }
      else
      {
        highDuration = duration;
      }

      lastPinState = newPinState; // Update the last pin state

      // Determine flashing patterns
      if (isWithinRange(highDuration, flashIntervalClosing, hysteresis) &&
          isWithinRange(lowDuration, flashIntervalClosing, hysteresis))
      {
        // Closing pattern detected
        if (gateState != CLOSING)
        {
          gateState = CLOSING;
          Serial.println("Gate is Closing");
          WEBLOG("Gate is Closing");
          if (current->getVal() != Characteristic::CurrentDoorState::CLOSING) {
            current->setVal(Characteristic::CurrentDoorState::CLOSING); // Update HomeKit
          }
          if (target->getVal() != Characteristic::TargetDoorState::CLOSED) {
            target->setVal(Characteristic::TargetDoorState::CLOSED);    // Confirm the target state
          }
        }
      }
      else if (isWithinRange(highDuration, flashIntervalOpening, hysteresis) &&
               isWithinRange(lowDuration, flashIntervalOpening, hysteresis))
      {
        // Opening pattern detected
        if (gateState != OPENING)
        {
          gateState = OPENING;
          Serial.println("Gate is Opening");
          WEBLOG("Gate is Opening");
          if (current->getVal() != Characteristic::CurrentDoorState::OPENING) {
            current->setVal(Characteristic::CurrentDoorState::OPENING); // Update HomeKit
          }
          if (target->getVal() != Characteristic::TargetDoorState::OPEN) {
            target->setVal(Characteristic::TargetDoorState::OPEN);      // Confirm the target state so homekit doesnt keep trying to open it
          }
        }
      }
    }

    // Determine stable states (Open/Closed)
    if ((currentTime - lastChangeTime) > stableThreshold)
    {
      if (lastPinState == LOW && gateState != CLOSED)
      {
        // Signal is stable LOW, indicating the gate is fully closed
        gateState = CLOSED;
        Serial.println("Gate is Closed");
        WEBLOG("Gate is Closed");
        if (current->getVal() != Characteristic::CurrentDoorState::CLOSED) {
          current->setVal(Characteristic::CurrentDoorState::CLOSED); // Update HomeKit
        }
        if (target->getVal() != Characteristic::TargetDoorState::CLOSED) {
          target->setVal(Characteristic::TargetDoorState::CLOSED);   // Confirm the target state
        }
      }
      else if (lastPinState == HIGH && gateState != OPEN)
      {
        // Signal is stable HIGH, indicating the gate is fully open
        gateState = OPEN;
        Serial.println("Gate is Open");
        WEBLOG("Gate is Open");
        if (current->getVal() != Characteristic::CurrentDoorState::OPEN) {
          current->setVal(Characteristic::CurrentDoorState::OPEN); // Update HomeKit
        }
        if (target->getVal() != Characteristic::TargetDoorState::OPEN) {
          target->setVal(Characteristic::TargetDoorState::OPEN);   // Confirm the target state
        }
      }
    }

    // Add a small delay to avoid flooding the serial output
    // delay(50);
  }

  bool isWithinRange(unsigned long value, unsigned long target, unsigned long tolerance)
  {
    return (value >= (target - tolerance)) && (value <= (target + tolerance));
  }

};
