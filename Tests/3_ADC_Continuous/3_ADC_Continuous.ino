/*   TEST FOR ADC IN CONTINUOUS MODE
 *   
 *   Setup:
 *     -> 555 output to SAH input
 *     -> SAH output to A9 (23)
 *     -> SAH output to comparator input
 *     -> Comparator output to 21
 *     -> 22 to SAH reset
 *   
 *   Processes the 555 pulses with the same logic as in 2_Comparator, but also reads the sample and hold values
 *   with the ADC in continuous mode. The ADC triggers an interrupt whenever it is finished with a reading. The 
 *   adc0_isr is given the highest priority, so that it can interrupt the comparator isr with new readings. A flag
 *   is necessary to force the comparator isr to wait for a reading that was started after the pulse was detected.
 *   Even so, it is only ~80% accurate. 
 *   
 *   While printing the ADC values, the SAH circuit is high for ~12 us. Without printing, it is high for ~3 us.
 *   
 *   Oscilloscope: https://docs.google.com/document/d/1JgwEvrjb5Nql0LQB7jF7oNHLPBpkSYnJeAKj1VzuWus/edit#heading=h.ru5w6zsrmt95
 *   
 *   ADC parameter data: https://docs.google.com/spreadsheets/d/1jV3Vywv2AzowY_LrOCaZFMoDmrVypA77NcN4mhOevPw/edit?usp=sharing
 */

#include <ADC.h>
#include <ADC_util.h>

const int sampleHoldPin = A9;
const int comparatorPin = 21;
const int resetPin = 22;

volatile uint16_t value;
volatile bool newValue = false;

ADC *adc = new ADC();

void setup() {
  pinMode(comparatorPin, INPUT);
  
  // Set up ADC
  pinMode(A9, INPUT);
  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);
  adc->adc0->enableInterrupts(adc0_isr);
  adc->adc0->startContinuous(sampleHoldPin);

  // Give the adc0_isr a higher priority than the comparator isr
  NVIC_SET_PRIORITY(IRQ_ADC0, 0);
  
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(10);
  pinMode(resetPin, INPUT);

  attachInterrupt(digitalPinToInterrupt(comparatorPin), isr, FALLING);

  Serial.begin(115200);
}

void loop() {}

void isr() {
  newValue = false;
  while (!newValue) {}
  
  //Serial.println(value);
  
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(comparatorPin) == LOW) {}
  pinMode(resetPin, INPUT);
}

void adc0_isr() {
  value = (uint16_t)adc->adc0->analogReadContinuous();
  newValue = true;
}
