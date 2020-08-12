/*   TEST FOR SAMPLE AND HOLD CIRCUIT
 *   
 *   Setup:
 *     -> 555 timer to SAH input
 *     -> SAH output to 23
 *     -> 22 to SAH reset
 *     
 *   Rising SAH output triggers interrupt that resets SAH. SAH should be high for ~3 us.
 *   
 *   Oscilloscope: https://docs.google.com/document/d/1C9luFUOWhSa_FcHO-zwdNHkUPhLz5-YzhxA3D5pOWl0/edit?usp=sharing
 */



const int sampleHoldPin = 23;
const int resetPin = 22;

void setup() {
  pinMode(sampleHoldPin, INPUT);
  
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(10);
  pinMode(resetPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(sampleHoldPin), isr, RISING);
}

void loop() {}

void isr() {
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(sampleHoldPin) == HIGH) {}
  pinMode(resetPin, INPUT);
}
