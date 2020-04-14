/******************************************************************************
 * @author Makers For Life
 * @copyright Copyright (c) 2020 Makers For Life
 * @file mass_flow_meter.cpp
 * @brief Mass Flow meter management
 *****************************************************************************/

/**
 * SFM3300-D sensirion mass flow meter is connected on I2C bus.
 * To perform the integral of the mass flow, i2c polling must be done in a high priority timer
 */

// Associated header
#include "../includes/mass_flow_meter.h"

// External
#include "../includes/config.h"
#include <Arduino.h>
#include <IWatchdog.h>
#include <OneButton.h>

// Internal
#include "../includes/parameters.h"

// Linked to Hardware v2
#ifdef MASS_FLOW_METER

// 2 khz => prescaler = 50000 => OK for a 16 bit timer. it cannnot be slower
// 10 khz => nice
#define MASS_FLOW_TIMER_FREQ 10000

// the timer period in microsecond, 100us precision (10 khz)
#define MASS_FLOW_PERIOD_US 1000

HardwareTimer* massFlowTimer;

// TO REMOVE
boolean testcon = false;

int32_t airVolumeSum = 0;

void MFM_Timer_Callback(HardwareTimer*) {

    // TO REMOVE : measure period here.
    testcon = !testcon;
    digitalWrite(PIN_SERIAL_TX, testcon);

    int16_t air = 150;

    airVolumeSum += air;
}

/**
 *  Returns true if there is a Mass Flow Meter connected
 *  If not detected, you will always read volume = 0 mL
 */
boolean MFM_init(void) {

    // set the timer
    massFlowTimer = new HardwareTimer(MASS_FLOW_TIMER);

    // prescaler. stm32f411 clock is 100mhz
    massFlowTimer->setPrescaleFactor((massFlowTimer->getTimerClkFreq() / MASS_FLOW_TIMER_FREQ) - 1);

    // set the period
    massFlowTimer->setOverflow(MASS_FLOW_TIMER_FREQ / MASS_FLOW_PERIOD_US);
    massFlowTimer->setMode(MASS_FLOW_CHANNEL, TIMER_OUTPUT_COMPARE, NC);
    massFlowTimer->attachInterrupt(MFM_Timer_Callback);

    // interrupt priority is documented here:
    // https://stm32f4-discovery.net/2014/05/stm32f4-stm32f429-nvic-or-nested-vector-interrupt-controller/
    massFlowTimer->setInterruptPriority(2, 0);

    // start the timer
    massFlowTimer->resume();

    // detect if the sensor is connected
    return true;
}

/**
 * return the number of milliliters since last reset
 * Can also perform the volume reset in the same atomic operation
 */
int32_t MFM_read_liters(boolean reset_after_read) {

    int32_t result;

    // this should be an atomic operation (32 bits aligned data)
    result = airVolumeSum;

    if (reset_after_read) {
        airVolumeSum = 0;
    }

    // compute the result in ml

    return 0;
}

void setup(void) {
    Serial.begin(115200);
    Serial.println("coucou, tu veux voir ma ... ?");
    Serial.println("init mass flow meter");
    MFM_init();

    pinMode(PIN_SERIAL_TX, OUTPUT);

    Serial.println("init done");
}

void loop(void) {

    delay(1000);
    Serial.print("volume = ");
    Serial.println(MFM_read_liters(false));
}

#endif