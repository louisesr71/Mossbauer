/*   TEST FOR DAC SINE WAVE WITH LOOKUP TABLE
 *    
 *   Setup:
 *     -> A14 to oscilloscope
 *     
 *   Timer calls an isr to update DAC value. The timer interval is such that there will be 4096 values. The position
 *   and velocity values are computed in setup() in a lookup table. Reading from the lookup tables and writing to the 
 *   DAC only takes 1-2 us, so higher frequency waves can be produced than when computing the sine function in the isr. 
 *   Up to 40 Hz was tested.
 *   
 *   Oscilloscope: https://docs.google.com/document/d/1uT1uEcBPzVspWH77eibhvwxnDDtCBeIq4gasHe9gMfA/edit?usp=sharing
 */


const int piezoPin = A14;

const float twopi = 2 * 3.1415926;
const int freq = 40;

uint16_t posArray[4096];
uint16_t velArray[4096];

int phase = 0;

IntervalTimer timer;

volatile uint32_t start;
volatile uint32_t end;

void setup() {
  // Generate lookup table:
  float w = 0;
  int i = 0;
  uint16_t pos;
  uint16_t vel;
  while (w < twopi) {
    pos = (int)(2048*sin(w) + 2048);
    vel = ((int)(2048*cos(w) + 2048)) / 4;
    if (pos >= 4096) {
      pos--;
    }
    if (vel >= 1024) {
      vel--;
    }
    posArray[i] = pos;
    velArray[i] = vel;
    w += twopi / 4096;
    i++;    
  }
  
  pinMode(piezoPin, OUTPUT);
  analogWriteResolution(12);
  
  Serial.begin(115200);

  Serial.print("Timer isr is called every ");
  Serial.print((int)((1./(freq*4096)) * 1000000));
  Serial.println(" us.");
  
  timer.begin(isr, (int)((1./(freq*4096)) * 1000000));

}

void loop() {}

void isr() {
  //start = micros();
  analogWrite(piezoPin, posArray[phase]);
  // Use velArray to update variable for MCS
  phase++;
  if (phase >= 4096) {
    phase = 0;
  }
  //end = micros();
  //Serial.println(end-start);
}
