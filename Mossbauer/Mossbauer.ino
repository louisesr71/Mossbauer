#include <ADC.h>
#include <ADC_util.h>

// INIT constants for IO pins:
int sampleHoldPin = A9;
int comparatorPin = 21;
int resetPin = 22;
//int piezoPin = A14;
  
// INIT variables:
//volatile bool serialFlag = false;
uint16_t adcValue;
uint32_t pha[4096];
uint32_t mcs[1024];
volatile uint16_t phase = 0;

// INIT ADC
ADC *adc = new ADC();
// INIT phaseTimer (as IntervalTimer)
// INIT serialTimer (as IntervalTimer)


volatile uint32_t start = 0;
volatile uint32_t end = 0;

void setup() {
  // SET IO pinMode
  pinMode(sampleHoldPin, INPUT);
  pinMode(comparatorPin, INPUT);
  pinMode(resetPin, INPUT);
  //pinMode(piezoPin, OUTPUT);
  
  Serial.begin(115200);
  
  // SET ADC parameters
  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);

  // Attach phaseTimer to call phase_isr every 100?? us
  // Attach serialTimer to call serial_isr every 5?? min
  // Attach hardware interrupt to call pulse_isr on falling edge of comparatorPin
  attachInterrupt(digitalPinToInterrupt(comparatorPin), pulse_isr, FALLING);

  // SET interrupt priorities:
    // phase_isr: <112
    // pulse_isr: <112
    // serial_isr: 128
    // (serial1 = 64, usb = 112, default = 128) (all multiples of 16, 0 - 255)

  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(10);
  pinMode(resetPin, INPUT);
}

void loop() {
  // IF serialFlag is true:
    // Disable interrupts
    // Make copy of pha and mcs
    // Enable interrupts
    // Output pha and mcs copies with Serial
    // SET serialFlag to false

  /*
  for (int i = 0; i < 4096; i++) {
    Serial.print(i);
    Serial.print(":  ");
    Serial.println(pha[i]);
  }
  delay(5000);
  */
}

void pulse_isr() {
  // SET adcValue to analogRead
  adcValue = analogRead(sampleHoldPin);
  //Serial.println(adcValue);

  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  //delayMicroseconds(5);
  pinMode(resetPin, INPUT);

  // INCREMENT pha at adcValue index
  // IF adcValue is within thresholds:
    // INCREMENT mcs at phase index
  pha[adcValue]++;
  mcs[phase]++;
}

void phase_isr() {
  // IF phase is less than 1023:
    // INCREMENT phase
  // ELSE:
    // SET phase to 0
  // Output sine wave to piezo with DAC
}

void serial_isr() {
  // SET serialFlag to true
}
