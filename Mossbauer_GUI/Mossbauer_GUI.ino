#include <ADC.h>
#include <ADC_util.h>

// INIT constants for IO pins:
const int sampleHoldPin = A9;
const int comparatorPin = 21;
const int resetPin = 22;
const int piezoPin = A14;
  
// INIT variables:
volatile bool serialFlag = false;
bool recording = false;
bool piezoEnabled = false;
uint16_t adcValue;
volatile uint16_t phase = 0;
volatile uint16_t velPhase = 0;

volatile uint32_t pha[4096];
volatile uint32_t mcs[1024];

// INIT constants:
uint16_t serialInterval = 1; // in seconds
uint16_t freq = 13; // in Hz
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
  
  // Generate pos/vel look up tables
  /* SINE WAVE 
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
  */

  /* QUADRATIC WAVE WITH LINEAR VELOCITY */
  for (int i = 0; i < 2048; i++) {
    posArray[i] = (int)(-1./512 * i * i + 4 * i + 2048);
    velArray[i] = (int)((-0.5 * (i+1)) + 1024);
  }
  for (int i = 2048; i < 4096; i++) {
    posArray[i] = (int)(1./512 * i * i - 12 * i + 18432);
    velArray[i] = (int)((0.5 * i) - 1024);
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

  // SET interrupt priorities:  
  NVIC_SET_PRIORITY(IRQ_PORTD, 96);
  serialTimer.priority(240);
  phaseTimer.priority(64);
  
  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  delay(10);
  pinMode(resetPin, INPUT);
}

void loop() {
  if (recording && serialFlag) {
    uint32_t phaCopy[4096];
    uint32_t mcsCopy[1024];
    
    detachInterrupt(digitalPinToInterrupt(comparatorPin)); // Only disable pulse_isr
    memcpy(phaCopy, pha, sizeof phaCopy);
    memcpy(mcsCopy, mcs, sizeof mcsCopy);
    attachInterrupt(digitalPinToInterrupt(comparatorPin), pulse_isr, FALLING); // Reenable pulse_isr

    // Copying the arrays takes ~250 us, so some pulses will be missed
    // Reset sample and hold; missed pulses will leave sample and hold high, preventing comparator falling edge
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, LOW);
    while (digitalRead(comparatorPin) == LOW ) {}
    pinMode(resetPin, INPUT);
    
    // Send phaCopy and mcsCopy over serial
    for (int i = 0; i < 4096; i++) {
      Serial.print(phaCopy[i]);
      Serial.print(",");
    }
    for (int i = 0; i < 1023; i++) {
      Serial.print(mcsCopy[i]);
      Serial.print(",");
    }
    Serial.println(mcsCopy[1023]);
    
    serialFlag = false;
  }

  if (Serial.available()) {
    char message = Serial.read();
    switch (message) {
      case 'a':
        start_record();
        break;
      case 'o':
        stop_record();
        break;
      case 'r':
        reset_record();
        break;
      case 'e':
        enable_piezo();
        break;
      case 'd':
        disable_piezo();
      case 'u':
        update_const();
        break;
    }
  }
}



/* ISR FUNCTIONS */

void pulse_isr() {
  adcValue = analogRead(sampleHoldPin);

  // Reset sample and hold circuit
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(comparatorPin) == LOW ) {}
  pinMode(resetPin, INPUT);

  pha[adcValue]++;
  // Only add pulse to mcs if the piezo is running
  if (piezoEnabled) { 
    if (adcValue >= 0 && adcValue <= 4095) {  // Change to mcs thresholds
      mcs[velPhase]++;
    }
  }

  // Check for overflow
  if (pha[adcValue] >= 4294967200 || mcs[velPhase] >= 4294967200) { 
    serialFlag = true;
  }
}

void phase_isr() {
  if (piezoEnabled) {
    analogWrite(piezoPin, posArray[phase]);
    velPhase = velArray[phase];
    phase++;
    if (phase >= 4096) {
      phase = 0;
    }
  }
}

void serial_isr() {
  serialFlag = true;
}



/* SERIAL UPDATE FUNCTIONS */

void start_record() {
  reset_record();
  attachInterrupt(digitalPinToInterrupt(comparatorPin), pulse_isr, FALLING);
  recording = true;
  serialInterval = Serial.parseInt();
  serialTimer.update(serialInterval * 1000000);
}

void stop_record() {
  detachInterrupt(digitalPinToInterrupt(comparatorPin));
  recording = false;  
}

void reset_record() {
  // Clear pha and mcs arrays
  if (recording) { detachInterrupt(digitalPinToInterrupt(comparatorPin)); }
  memset(pha, 0, sizeof pha);
  memset(mcs, 0, sizeof mcs);
  if (recording) { attachInterrupt(digitalPinToInterrupt(comparatorPin), pulse_isr, FALLING); }
  
  // Reset sample and hold
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);
  while (digitalRead(comparatorPin) == LOW ) {}
  pinMode(resetPin, INPUT);
}

void enable_piezo() {
  piezoEnabled = true;
  freq = Serial.parseInt();
  phaseTimer.update((int)((1./(freq*4096)) * 1000000));
}

void disable_piezo() {
  analogWrite(piezoPin, 0);
  piezoEnabled = false;
}

void update_const() {
  // Update timer intervals
  freq = Serial.parseInt();
  phaseTimer.update((int)((1./(freq*4096)) * 1000000));
  serialInterval = Serial.parseInt();
  serialTimer.update(serialInterval * 1000000);
}
