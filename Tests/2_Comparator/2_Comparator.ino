/*   TEST FOR COMPARATOR CIRCUIT
 *    
 *   Setup:
 *     -> 555 timer to SAH input (across potentiometer to vary 555 pulse height)
 *     -> SAH output to op amp inverting input
 *     -> Potentiometer to op amp non-inverting input
 *     -> Op amp output to 21
 *     -> 22 to SAH reset
 *     
 *   Comparator circuit output is LOW when SAH output is above potentiometer threshold. Falling comparator 
 *   edge triggers interrupt that resets SAH. SAH should be high for ~3 us.
 *   
 *   Oscilloscope: https://docs.google.com/document/d/125AWIiQw9gpMkm1Jhu2Yh9l5JAxgCBbcBelvbIytXbE/edit?usp=sharing
 */

const int comparatorPin = 21;
const int resetPin = 22;

void setup() {
  pinMode(comparatorPin, INPUT);
  
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(10);
  pinMode(resetPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(comparatorPin), isr, FALLING);
}

void loop() {}

void isr() {
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(comparatorPin) == LOW) {}
  pinMode(resetPin, INPUT);
}
