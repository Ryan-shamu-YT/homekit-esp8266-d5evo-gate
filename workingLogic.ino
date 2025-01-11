//////////////////////////////////
// This is just logic. Not needed, just suplimentary
//////////////////////////////////

const int statusPin = 39;  // The status pin connected to the gate
unsigned long lastChangeTime = 0;  // Time of the last pin state change
unsigned long highDuration = 0;  // Duration the pin was HIGH
unsigned long lowDuration = 0;   // Duration the pin was LOW
unsigned long stableThreshold = 500;  // Time to confirm the gate is fully open/closed

// Flashing intervals
const unsigned long flashIntervalClosing = 150;  // Interval for "Closing" (HIGH/LOW)
const unsigned long flashIntervalOpening = 300;  // Interval for "Opening" (HIGH/LOW)
const unsigned long hysteresis = 50;  // Tolerance for timing variations

int lastPinState = LOW;  // Last known pin state
int currentPinState = LOW;  // Current pin state
int gateState = -1;  // Current gate state

// Gate states
enum GateState {
  UNKNOWN,  // 0: The state of the gate is unknown
  CLOSED,   // 1: The gate is fully closed
  OPEN,     // 2: The gate is fully open
  CLOSING,  // 3: The gate is in the process of closing
  OPENING   // 4: The gate is in the process of opening
};

void setup() {
  Serial.begin(115200);         // Start serial communication for debugging
  delay(1000);
  pinMode(statusPin, INPUT);    // Set the status pin as an input
  Serial.println("Monitoring gate states...");
  gateState = UNKNOWN;  // Initialize to an unknown state
}

void loop() {
  int newPinState = digitalRead(statusPin);  // Read the current pin state
  unsigned long currentTime = millis();  // Get the current time

  // Detect pin state changes
  if (newPinState != lastPinState) {
    unsigned long duration = currentTime - lastChangeTime;  // Duration of the previous state
    lastChangeTime = currentTime;  // Update the time of the change

    // Record HIGH/LOW durations
    if (newPinState == HIGH) {
      lowDuration = duration;
    } else {
      highDuration = duration;
    }

    lastPinState = newPinState;  // Update the last pin state

    // Determine flashing patterns
    if (isWithinRange(highDuration, flashIntervalClosing, hysteresis) &&
        isWithinRange(lowDuration, flashIntervalClosing, hysteresis)) {
      // Closing pattern detected
      if (gateState != CLOSING) {
        gateState = CLOSING;
        Serial.println("Gate is Closing");
      }
    } else if (isWithinRange(highDuration, flashIntervalOpening, hysteresis) &&
               isWithinRange(lowDuration, flashIntervalOpening, hysteresis)) {
      // Opening pattern detected
      if (gateState != OPENING) {
        gateState = OPENING;
        Serial.println("Gate is Opening");
      }
    }
  }

  // Determine stable states (Open/Closed)
  if ((currentTime - lastChangeTime) > stableThreshold) {
    if (lastPinState == LOW && gateState != CLOSED) {
      // Signal is stable LOW, indicating the gate is fully closed
      gateState = CLOSED;
      Serial.println("Gate is Closed");
    } else if (lastPinState == HIGH && gateState != OPEN) {
      // Signal is stable HIGH, indicating the gate is fully open
      gateState = OPEN;
      Serial.println("Gate is Open");
    }
  }

  // Add a small delay to avoid flooding the serial output
  delay(50);
}

// Helper function to check if a duration is within a range
bool isWithinRange(unsigned long value, unsigned long target, unsigned long tolerance) {
  return (value >= (target - tolerance)) && (value <= (target + tolerance));
}
