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

#include "cppm2pwm.h"

#include <assert.h>

volatile TimerData gtimerData;
TimerData lastTimerData;

Ccppm2pwm::Ccppm2pwm(int channels) {

    gtimerData.count = 0;
    gtimerData.recvState = rcvIdle;
    gtimerData.channels = channels;
    gtimerData.channel2pwm1 = -1;
}

void Ccppm2pwm::begin() {
    setupTimer();
}

void Ccppm2pwm::setupTimer() {
    // Setup timer1
    TCCR1A = 0; // No PWM
    TCCR1B = (0 << CS12) | (1 << CS11) | (1 << CS10);       // Input clock is set to F(cpu)/64 ... prescale of 64
    TCNT1 = 0;
    /* Ptr prescale 256 max 2097.152 mS
     * Timer Resolution = (1 / (Input Frequency / Prescale)) = (Prescale / Input Frequency)
     *                  = 256 / 16 x 10^6 s
     *                  = 16 us
     *
     * Ptr prescale 64 max 524.288 mS
     * Timer Resolution = (64 / 16 ) * 10^-6 s
     *                  = 4 us
    */
}

/*
 * channel - The channel index from the PPM signal to output as PWM
 * period - the cycle length in seconds
 */
void Ccppm2pwm::startTimerPwmOnPin3(int channel, float period) {
    pinMode(3, OUTPUT);
    // Varying the timer top limit: phase-correct PWM
    TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM20); // Phase correct PWM
    TCCR2B = _BV(WGM22) | _BV(CS22) | _BV(CS21) | _BV(CS20); // Prescale 1024 -> 61.03515625 Hz
    /*
     * Output B frequency: 16 MHz / Prescale / OCR2A / 2 / 2 = Hz
     * OCR2A = 16 * 10^6 Hz / Prescale / Hz / 2
     * Hz = 1 / Period (if we want 18ms cycle then Hz = 1/(18*10^-3))
     *
     * OCR2B = OCR2A * Pulse width * Hz (pulse width is in seconds)
     * OCR2B = (16 * 10^6 Hz / Prescale / Hz / 2) * Pulse Width * Hz
     *       = (16 * 10^6 Hz / Prescale / 2) * Pulse Width
     *
    */
    OCR2A = round((16000000* period) / 1024 / 2);

    // Start with a 10% pulse width out of the period
    OCR2B = OCR2A * (0.1 * period) * (1/period);

#ifdef _DEBUG_
    if(channel >= gtimerData.channels)
        assert(false);
#endif

    gtimerData.channel2pwm1 = channel;
}

void Ccppm2pwm::inCPPM() {

    unsigned long delta;

    switch(gtimerData.recvState) {

        case rcvIdle:
            // Reset timer
            TCNT1 = 0;
            gtimerData.recvState = rcvFrameSpace;
            return;

        case rcvFrameSpace:
            delta = TCNT1*TIMER_RESOLUTION;
            // Reset timer
            TCNT1 = 0;

            // If we are not measuring the frame space we continue
            if(delta < MIN_FRAME_SPACE) {
                return;
            }

            // If the frame space is to high we consider it an anomaly, reset and keep
            // searching for the frame space
            if(delta > MAX_FRAME_LENGTH) {
                return;
            }

            gtimerData.recvState = rcvStart;
            return;

        case rcvStart:
            // Reset timer
            TCNT1 = 0;

            gtimerData.recvState = rcvEnd;
            return;

        case rcvEnd:
            delta = TCNT1*TIMER_RESOLUTION;

            // Reset timer
            TCNT1 = 0;

            // Reset and return if this is a frame space
            if(delta >= MIN_FRAME_SPACE) {
                gtimerData.count = 0;
                return;
            }

            gtimerData.pulseLength[gtimerData.count] = delta;

            // Output PWM
            if(gtimerData.channel2pwm1 == gtimerData.count)
                //Serial.println(delta);
                Ccppm2pwm::updatePwmOnPin3(delta);

            gtimerData.count ++;

            // Get only the first channels as requested and ignore the rest
            if(gtimerData.count >= gtimerData.channels)
            {
                gtimerData.recvState = rcvFrameSpace;
                lastTimerData = (TimerData)gtimerData;
                gtimerData.count = 0;
                return;
            }

            gtimerData.recvState = rcvStart;
            return;
    }
}

void Ccppm2pwm::updatePwmOnPin3(float pulseWidthInMicroseconds) {
    // OCR2B = (16000000 / 1024 / 2) * Pulse Width;
    OCR2B = round((16 * (pulseWidthInMicroseconds + LOW_PULSE_LENGTH)) / 2048.0);
}

#ifdef _DEBUG_

TimerData Ccppm2pwm::getTimerData() {
    return lastTimerData;
}

void Ccppm2pwm::dumpTimerData(const TimerData& timerData) {

    int time = 0;
    Serial.print("Frame ");
    Serial.print(timerData.count, DEC);
    Serial.println(" dump:");

    for(int i = 0; i < timerData.count; i++) {
        Serial.print(time, DEC);
        Serial.print("\t");
        Serial.print(0, DEC);
        Serial.println();

        Serial.print(time, DEC);
        Serial.print("\t");
        Serial.print(1, DEC);
        Serial.println();

        time += timerData.pulseLength[i];

        Serial.print(time, DEC);
        Serial.print("\t");
        Serial.print(1, DEC);
        Serial.println();

        Serial.print(time, DEC);
        Serial.print("\t");
        Serial.print(0, DEC);
        Serial.println();
        time += 400;
    }
}

#endif
