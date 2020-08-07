#include <ADC.h>
#include <ADC_util.h>

// INIT constants for IO pins:
const int sampleHoldPin = A9;
const int comparatorPin = 21;
const int resetPin = 22;
const int piezoPin = A14;
  
// INIT variables:
volatile bool serialFlag = false;
uint16_t adcValue;
volatile uint16_t phase = 0;
volatile uint16_t velPhase = 0;

volatile uint32_t pha[4096];
volatile uint32_t mcs[1024];

// INIT constants:
const uint16_t serialInterval = 5; // in seconds
const uint16_t freq = 13; // in Hz
const float twopi = 2 * 3.1415926;
// Lookup Tables:
uint16_t posArray[4096];
uint16_t velArray[4096];

// INIT ADC
ADC *adc = new ADC();
// INIT phaseTimer (as IntervalTimer)
IntervalTimer phaseTimer;
// INIT serialTimer (as IntervalTimer)
IntervalTimer serialTimer;


volatile uint32_t start = 0;
volatile uint32_t end = 0;

void setup() {
  Serial.begin(115200);
  //while (!Serial) {}
  
  // Generate pos/vel look up tables
  float w = 0;
  uint16_t i = 0;
  uint16_t pos;
  uint16_t vel;
  while (w < twopi) {
    pos = (uint16_t)(2048*sin(w) + 2048);
    if (pos >= 4096) {
      pos = 4095;
    }
    vel = ((uint16_t)(2048*cos(w) + 2048)) / 4;
    if (vel >= 1024) {
      vel = 1023;
    }    
    posArray[i] = pos;
    velArray[i] = vel; 
    w += twopi / 4096;
    i++;
  }
  
  // SET IO pinMode
  pinMode(sampleHoldPin, INPUT);
  pinMode(comparatorPin, INPUT);
  pinMode(resetPin, INPUT);
  pinMode(piezoPin, OUTPUT);
  
  // SET ADC parameters
  adc->adc0->setAveraging(1);
  adc->adc0->setResolution(12);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::LOW_SPEED);

  // SET DAC parameters
  analogWriteResolution(12);

  // Attach phaseTimer to call phase_isr every 100?? us
  phaseTimer.begin(phase_isr, (int)((1./(freq*4096)) * 1000000));
  // Attach serialTimer to call serial_isr every 5?? min
  serialTimer.begin(serial_isr, serialInterval*1000000);
  // Attach hardware interrupt to call pulse_isr on falling edge of comparatorPin
  attachInterrupt(digitalPinToInterrupt(comparatorPin), pulse_isr, FALLING);

  // SET interrupt priorities:
    // phase_isr: <112
    // pulse_isr: <112
    // serial_isr: 128
    // (serial1 = 64, usb = 112, default = 128) (all multiples of 16, 0 - 255)
    
  //NVIC_SET_PRIORITY(IRQ_PORTD, 96) // works without this line
  // Port assignments: https://forum.pjrc.com/threads/23950-Parallel-GPIO-on-Teensy-3-0?p=34158&viewfull=1#post34158
  serialTimer.priority(240);
  phaseTimer.priority(0);
  

  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(10);
  pinMode(resetPin, INPUT);


}

void loop() {
  if (serialFlag) {
    uint32_t phaCopy[4096];
    uint32_t mcsCopy[1024];
    noInterrupts();
    memcpy(phaCopy, pha, sizeof phaCopy);
    memcpy(mcsCopy, mcs, sizeof mcsCopy);
    interrupts();

    // OUTPUT pha array
    for (int i = 0; i < 4096; i++) {
      Serial.print(i);
      Serial.print(":   ");
      Serial.println(phaCopy[i]);
    }
    //OUTPUT mcs array
    for (int i = 0; i < 1024; i++) {
      Serial.print(i);
      Serial.print(":   ");
      Serial.println(mcsCopy[i]);
    }
    
    serialFlag = false;
  }
}

void pulse_isr() {
  adcValue = analogRead(sampleHoldPin);
  //Serial.println(adcValue);

  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(comparatorPin) == LOW ) {}
  //delayMicroseconds(5); // Add delay if necessary
  pinMode(resetPin, INPUT);

  pha[adcValue]++;
  mcs[velPhase]++;
}

void phase_isr() {
  analogWrite(piezoPin, posArray[phase]);
  velPhase = velArray[phase];
  phase++;
  if (phase >= 4096) {
    phase = 0;
  }
}

void serial_isr() {
  serialFlag = true;
}
