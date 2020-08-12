/*   TEST PRIORITY OF INTERVAL TIMER AND HARDWARE INTERRUPTS
 *   
 *   Setup:
 *     -> 555 output to 14
 *   
 *   Counts how many hardware interrupts occur while an IntervalTimer isr is being processed.
 *   If count is 0-2, timer_isr is not interrupted by count_isr.
 *   If count is ~16000, timer_isr is interrupted by count_isr.
 *   
 *   In order for count_isr to interrupt timer_isr, the hardware interrupt priority must be lower than the timer priority.
 *   (i.e. timer = 240 and count = 128 works, but timer = 128 and count = 128 doesn't work)
 *   
 *   If either routine calls Serial.print (default priority is 112), both count and timer must have priorities >112.
 */

const int interruptPin = 14;
volatile uint16_t count;
volatile bool flag;

IntervalTimer timer;

void setup() {
  pinMode(interruptPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(interruptPin), count_isr, RISING);

  timer.begin(timer_isr, 1000000);
  timer.priority(96);
  NVIC_SET_PRIORITY(IRQ_PORTD, 64);
  // Priority can be any multiple of 16, 0-240, with 0 as the highest priority.
  // Pin ports: https://forum.pjrc.com/threads/23950-Parallel-GPIO-on-Teensy-3-0?p=34158&viewfull=1#post34158
  
  Serial.begin(115200);
}

void loop() {
  if (flag) {
    Serial.println(count);
  }
  flag = false;
}

void timer_isr() {
  count = 0;
  delay(800);
  flag = true;
}

void count_isr() {
  count++;
}
