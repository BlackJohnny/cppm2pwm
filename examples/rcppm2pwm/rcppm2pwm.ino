#include "cppm2pwm.h"
/* This can be usefull if you are using a FrSky D4R receiver configured with
 * CPPM output and you want to extract one channel and use it as PWM for
 * controlling another device. You would also need to use an arduino Nano or Pro mini
 * between the Receiver and the device you want to control with the remote.
 */
// Init the CPPM decoder reading 8 channels from the CPPM signal
Ccppm2pwm cppm2pwm(8);

void setup() {
  Serial.begin(9600);
  cppm2pwm.begin(); 
  attachInterrupt(0, Ccppm2pwm::inCPPM, CHANGE);
  cppm2pwm.startTimerPwmOnPin3(0, 0.018);
}

void loop() {
  //delay(10000);
  /* Paste the dump into a file (whithout the header) and plot the signal
  * using gnuplot with the command:
  *   plot "sig.dat" using 1:2 with lines"
  */
  //Ccppm2pwm::dumpTimerData(cppm2pwm.getTimerData());
}
