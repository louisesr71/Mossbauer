/*   TEST FOR DAC LOOKUP TABLE WITH LINEAR VELOCITY
 *    
 *   Setup:
 *     -> A14 to oscilloscope
 *     
 *   Timer calls an isr to update DAC value. The timer interval is such that there will be 4096 values. The position
 *   and velocity values are computed in setup() in a lookup table. The position of the piezo follow two quadratic 
 *   functions that are oriented in opposite directions to mimic a sine wave. Because position is quadratic, the 
 *   velocity is linear.
 *   
 *   Oscilloscope: https://docs.google.com/document/d/17og_Q3gC386qjY95RvBrrvwW2WMyVF_gk96I2N2M3tI/edit?usp=sharing
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
  for (int i = 0; i < 2048; i++) {
    posArray[i] = (int)(-1./512 * i * i + 4 * i + 2048);
    velArray[i] = (int)((-0.5 * i) + 1024);
  }
  for (int i = 2048; i < 4096; i++) {
    posArray[i] = (int)(1./512 * i * i - 12 * i + 18432);
    velArray[i] = (int)((0.5 * (i+1)) - 1024);
  }
  
  pinMode(piezoPin, OUTPUT);
  analogWriteResolution(12);
  
  Serial.begin(115200);

  Serial.print("Timer isr is called every ");
  Serial.print((int)((1./(freq*4096)) * 1000000));
  Serial.println(" us.");
  
  timer.begin(isr, (int)((1./(freq*4096)) * 1000000));

}

void loop() {
  
  for (int i = 0; i < 4096; i++) {
    Serial.println(velArray[i]);
  }
  delay(5000);

}

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
