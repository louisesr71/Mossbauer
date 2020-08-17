/*   TEST FOR DAC SINE WAVE
 *    
 *   Setup:
 *     -> A14 to oscilloscope
 *     
 *   Timer calls an isr to update DAC value. The timer interval is such that there will be 4096 values. The sine
 *   function is computed in the isr, and takes ~50 us. Since a 5 Hz wave has a timer interval of 48 us, this method
 *   cannot produce frequencies about 5 Hz. 
 *   
 *   Oscilloscope: https://docs.google.com/document/d/1iy0IGYuyeJUGWLiVSJXemOxV48CDUaN29y8BNlkmNJI/edit?usp=sharing
 */


const int piezoPin = A14;

const float twopi = 2 * 3.1415926;
const int freq = 5;

int phase = 0;

IntervalTimer timer;

volatile uint32_t start;
volatile uint32_t end;

void setup() {
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
  analogWrite(piezoPin, (int)(2048*sin(phase * (twopi / 4096)) + 2048));
  phase++;
  //end = micros();
  //Serial.println(end-start);
}
