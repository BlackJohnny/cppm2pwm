/*
 * This file is part of cppm2pwm.
 *
 * cppm2pwm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * cppm2pwm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cppm2pwm.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef Ccppm2pwm_h
#define Ccppm2pwm_h

#include "Arduino.h"
//#include <string.h>

#define _DEBUG_

#define RAWBUF 27
// Minumum frame space to look for
#define MIN_FRAME_SPACE 3000
#define MAX_FRAME_LENGTH 27000
#define LOW_PULSE_LENGTH 400
// Timer Resolution = (1 / (Input Frequency / Prescale)) = (Prescale / Input Frequency)
#define TIMER_RESOLUTION 4

typedef enum enumRecvState
{
    rcvIdle,
    rcvFrameSpace,
    rcvStart,
    rcvEnd
} RecvState;

typedef struct TimerDataStr{
  RecvState recvState;
  byte channels;           // Number of channels to read from PPM
  unsigned long pulseLength[RAWBUF];  // Pulses
  byte count;         // Counter of pulses
  byte channel2pwm1;   // the channel from the CPPM signal to output as PWM

#ifdef _DEBUG_
  TimerDataStr() {}
  TimerDataStr(const volatile TimerDataStr& timerData) {
    recvState = timerData.recvState;
    channels = timerData.channels;
    count = timerData.count;
    //memcpy(pulseLength, timerData.pulseLength, sizeof(unsigned long)*RAWBUF);
    for(int i = 0; i < 27; i++)
        pulseLength[i] = timerData.pulseLength[i];
  }
#endif

} TimerData;

class Ccppm2pwm
{
public:
    Ccppm2pwm(int channels);

    static void inCPPM();
    void begin();

    static void startTimerPwmOnPin3(int channel, float period);
    static void updatePwmOnPin3(float pulseWidthInMicroseconds);

#ifdef _DEBUG_
    TimerData getTimerData();
    static void dumpTimerData(const TimerData& timerData);
#endif

private:
    void setupTimer();
};


#endif
