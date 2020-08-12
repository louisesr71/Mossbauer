/*   TEST FOR ADC IN SINGLE SHOT MODE
 *    
 *   Setup:
 *     -> 555 output to SAH input
 *     -> SAH output to A9 (23)
 *     -> SAH output to comparator input
 *     -> Comparator output to 21
 *     -> 22 to SAH reset
 *     
 *   Processes the 555 pulses with the same logic as in 2_Comparator, but also reads the sample and hold values
 *   with the ADC in single shot mode. While printing ADC values, the SAH circuit is high for ~ us. Without 
 *   printing, it is high for ~6 us. The ADC values are more accurate than when using the continuous mode.
 *   
 *   Oscilloscope: https://docs.google.com/document/d/1lHOMYb1rqW-5h7KrXkjQQNRIrfRCEmRFkz6j4gOYrWI/edit?usp=sharing
 *   
 *   ADC parameter data: https://docs.google.com/spreadsheets/d/1jV3Vywv2AzowY_LrOCaZFMoDmrVypA77NcN4mhOevPw/edit?usp=sharing
 */

#include <ADC.h>
#include <ADC_util.h>

const int sampleHoldPin = A9;
const int comparatorPin = 21;
const int resetPin = 22;

volatile uint16_t value;

ADC *adc = new ADC();

void setup() {
  pinMode(comparatorPin, INPUT);
  
  // Set up ADC
  pinMode(A9, INPUT);
  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);
  
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
  value = analogRead(sampleHoldPin);

  //Serial.println(value);

  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(comparatorPin) == LOW) {}
  pinMode(resetPin, INPUT);
}
