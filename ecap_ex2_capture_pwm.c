//#############################################################################
//
// FILE:    ecap_capture_pwm.c
//
// TITLE:   Capture ePWM3.
//
//! \addtogroup driver_example_list
//! <h1>eCAP Capture PWM Example</h1>
//!
//! This example configures ePWM3A for:
//! - Up count mode
//! - Period starts at 500 and goes up to 8000
//! - Toggle output on PRD
//!
//! eCAP1 is configured to capture the time between rising
//! and falling edge of the ePWM3A output.
//!
//! \b External \b Connections \n
//! - eCAP1 is on GPIO16
//! - ePWM3A is on GPIO4
//! - Connect GPIO4 to GPIO16.
//!
//! \b Watch \b Variables \n
//! - \b ecap1PassCount - Successful captures.
//! - \b ecap1IntCount - Interrupt counts.
//
//#############################################################################
// $TI Release: F2837xD Support Library v3.10.00.00 $
// $Release Date: Tue May 26 17:13:46 IST 2020 $
// $Copyright:
// Copyright (C) 2013-2020 Texas Instruments Incorporated - http://www.ti.com/
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "driverlib.h"
#include "device.h"

//
// Defines
//
#define PWM3_TIMER_MIN     1000U
#define PWM3_TIMER_MAX     8000U
#define EPWM_TIMER_UP      1U
#define EPWM_TIMER_DOWN    0U

//
// Globals
//
//uint32_t ecap1IntCount;
//uint32_t ecap1PassCount;
uint32_t epwm3TimerDirection;
volatile uint32_t PCB_A_Temp_P;
volatile uint32_t PCB_B_Temp_P;
volatile uint32_t PCB_C_Temp_P;
volatile uint32_t Mosfet_A_Temp_P;
volatile uint32_t Mosfet_B_Temp_P;
volatile uint32_t Mosfet_C_Temp_P;


//
// Function Prototypes
//
void error(void);
void initECAP(void);
void initEPWM(void);
__interrupt void ecap1ISR(void);
__interrupt void ecap2ISR(void);
__interrupt void ecap3ISR(void);
__interrupt void ecap4ISR(void);
__interrupt void ecap5ISR(void);
__interrupt void ecap6ISR(void);


//
// Main
//
void main(void)
{
    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pullups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // Configure GPIO4/5 as ePWM3A/3B
    //
    GPIO_setPadConfig(4, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_4_EPWM3A);
    GPIO_setPadConfig(5, GPIO_PIN_TYPE_STD);
    GPIO_setPinConfig(GPIO_5_EPWM3B);

    //
    // Configure GPIOs as eCAP inputs
    //
    XBAR_setInputPin(XBAR_INPUT7, 67);   //PCBA Temp APWM i/p
    GPIO_setPinConfig(GPIO_67_GPIO67);
    GPIO_setDirectionMode(67, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(67, GPIO_QUAL_ASYNC);

    XBAR_setInputPin(XBAR_INPUT8, 71);  //PCBB Temp APWM i/p
    GPIO_setPinConfig(GPIO_71_GPIO71);
    GPIO_setDirectionMode(71, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(71, GPIO_QUAL_ASYNC);

    XBAR_setInputPin(XBAR_INPUT8, 25);  //PCBC Temp APWM i/p
    GPIO_setPinConfig(GPIO_25_GPIO25);
    GPIO_setDirectionMode(25, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(25, GPIO_QUAL_ASYNC);

    XBAR_setInputPin(XBAR_INPUT7, 68);   //Mosfet A Temp APWM i/p
    GPIO_setPinConfig(GPIO_67_GPIO67);
    GPIO_setDirectionMode(67, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(67, GPIO_QUAL_ASYNC);

    XBAR_setInputPin(XBAR_INPUT8, 70);  //Mosfet B Temp APWM i/p
    GPIO_setPinConfig(GPIO_71_GPIO71);
    GPIO_setDirectionMode(71, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(71, GPIO_QUAL_ASYNC);

    XBAR_setInputPin(XBAR_INPUT8, 24);  //Mosfet C Temp APWM i/p
    GPIO_setPinConfig(GPIO_24_GPIO24);
    GPIO_setDirectionMode(24, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(24, GPIO_QUAL_ASYNC);
    //
    // Interrupts that are used in this example are re-mapped to ISR functions
    // found within this file.
    //
    Interrupt_register(INT_ECAP1, &ecap1ISR);
    Interrupt_register(INT_ECAP2, &ecap2ISR);
    Interrupt_register(INT_ECAP3, &ecap3ISR);
    Interrupt_register(INT_ECAP4, &ecap4ISR);
    Interrupt_register(INT_ECAP5, &ecap5ISR);
    Interrupt_register(INT_ECAP6, &ecap6ISR);

    //
    // Configure ePWM and eCAP
    //
    initEPWM();
    initECAP();

    //
    // Initialize counters:
    //

    PCB_A_Temp_P = 0U;
    PCB_B_Temp_P = 0U;
    PCB_C_Temp_P = 0U;
    Mosfet_A_Temp_P = 0U;
    Mosfet_B_Temp_P = 0U;
    Mosfet_C_Temp_P = 0U;
    //
    // Enable interrupts required for this example
    //
    Interrupt_enable(INT_ECAP1);
    Interrupt_enable(INT_ECAP2);
    Interrupt_enable(INT_ECAP3);
    Interrupt_enable(INT_ECAP4);
    Interrupt_enable(INT_ECAP5);
    Interrupt_enable(INT_ECAP6);
    //
    // Enable Global Interrupt (INTM) and Real time interrupt (DBGM)
    //
    EINT;
    ERTM;

    //
    // Loop forever. Suspend or place breakpoints to observe the buffers.
    //
    for(;;)
    {
       NOP;
    }
}

//
// initEPWM - Configure ePWM
//
void initEPWM()
{
    //
    // Disable sync(Freeze clock to PWM as well)
    //
    SysCtl_disablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);

    //
    // Configure ePWM
    //       Counter runs in up-count mode.
    //       Action qualifier will toggle output on period match
    //
    EPWM_setTimeBaseCounterMode(EPWM3_BASE, EPWM_COUNTER_MODE_UP);
    EPWM_setTimeBasePeriod(EPWM3_BASE, PWM3_TIMER_MIN);
    EPWM_setPhaseShift(EPWM3_BASE, 0U);
    EPWM_setActionQualifierAction(EPWM3_BASE,
                                  EPWM_AQ_OUTPUT_A,
                                  EPWM_AQ_OUTPUT_TOGGLE,
                                  EPWM_AQ_OUTPUT_ON_TIMEBASE_PERIOD);
    EPWM_setClockPrescaler(EPWM3_BASE,
                           EPWM_CLOCK_DIVIDER_1,
                           EPWM_HSCLOCK_DIVIDER_1);

    epwm3TimerDirection = EPWM_TIMER_UP;

    //
    // Enable sync and clock to PWM
    //
    SysCtl_enablePeripheral(SYSCTL_PERIPH_CLK_TBCLKSYNC);
}

//
// initECAP - Configure eCAP
//
void initECAP()
{
    //
    // Disable ,clear all capture flags and interrupts
    //
    ECAP_disableInterrupt(ECAP1_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_disableInterrupt(ECAP2_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_disableInterrupt(ECAP3_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_disableInterrupt(ECAP4_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_disableInterrupt(ECAP5_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_disableInterrupt(ECAP6_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));

    ECAP_clearInterrupt(ECAP1_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_clearInterrupt(ECAP2_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_clearInterrupt(ECAP3_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_clearInterrupt(ECAP4_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_clearInterrupt(ECAP5_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));
    ECAP_clearInterrupt(ECAP6_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1));

    //
    // Disable CAP1-CAP4 register loads
    //
    ECAP_disableTimeStampCapture(ECAP1_BASE);
    ECAP_disableTimeStampCapture(ECAP2_BASE);
    ECAP_disableTimeStampCapture(ECAP3_BASE);
    ECAP_disableTimeStampCapture(ECAP4_BASE);
    ECAP_disableTimeStampCapture(ECAP5_BASE);
    ECAP_disableTimeStampCapture(ECAP6_BASE);
    //
    // Configure eCAP
    //    Enable capture mode.
    //    One shot mode, stop capture at event 4.
    //    Set polarity of the events to rising, falling, rising, falling edge.
    //    Set capture in time difference mode.
    //    Select input from XBAR7.
    //    Enable eCAP module.
    //    Enable interrupt.
    //
    ECAP_stopCounter(ECAP1_BASE);
    ECAP_stopCounter(ECAP2_BASE);
    ECAP_stopCounter(ECAP3_BASE);
    ECAP_stopCounter(ECAP4_BASE);
    ECAP_stopCounter(ECAP5_BASE);
    ECAP_stopCounter(ECAP6_BASE);

    ECAP_enableCaptureMode(ECAP1_BASE);
    ECAP_enableCaptureMode(ECAP2_BASE);
    ECAP_enableCaptureMode(ECAP3_BASE);
    ECAP_enableCaptureMode(ECAP4_BASE);
    ECAP_enableCaptureMode(ECAP5_BASE);
    ECAP_enableCaptureMode(ECAP6_BASE);

    ECAP_setCaptureMode(ECAP1_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);
    ECAP_setCaptureMode(ECAP2_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);
    ECAP_setCaptureMode(ECAP3_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);
    ECAP_setCaptureMode(ECAP4_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);
    ECAP_setCaptureMode(ECAP5_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);
    ECAP_setCaptureMode(ECAP6_BASE, ECAP_CONTINUOUS_CAPTURE_MODE, ECAP_EVENT_1);



    ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_1,ECAP_EVNT_RISING_EDGE );
    ECAP_setEventPolarity(ECAP2_BASE, ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);
    ECAP_setEventPolarity(ECAP3_BASE, ECAP_EVENT_1,ECAP_EVNT_RISING_EDGE );
    ECAP_setEventPolarity(ECAP4_BASE, ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);
    ECAP_setEventPolarity(ECAP5_BASE, ECAP_EVENT_1,ECAP_EVNT_RISING_EDGE );
    ECAP_setEventPolarity(ECAP6_BASE, ECAP_EVENT_1, ECAP_EVNT_RISING_EDGE);


    ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(ECAP2_BASE, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(ECAP3_BASE, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(ECAP4_BASE, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(ECAP5_BASE, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(ECAP6_BASE, ECAP_EVENT_1);


    XBAR_setInputPin(XBAR_INPUT7, 67);
    XBAR_setInputPin(XBAR_INPUT8, 71);
    XBAR_setInputPin(XBAR_INPUT9, 25);
    XBAR_setInputPin(XBAR_INPUT10, 68);
    XBAR_setInputPin(XBAR_INPUT11, 70);
    XBAR_setInputPin(XBAR_INPUT12, 24);

    ECAP_enableLoadCounter(ECAP1_BASE);
    ECAP_enableLoadCounter(ECAP2_BASE);
    ECAP_enableLoadCounter(ECAP3_BASE);
    ECAP_enableLoadCounter(ECAP4_BASE);
    ECAP_enableLoadCounter(ECAP5_BASE);
    ECAP_enableLoadCounter(ECAP6_BASE);

    ECAP_setSyncOutMode(ECAP1_BASE, ECAP_SYNC_OUT_SYNCI);
    ECAP_setSyncOutMode(ECAP2_BASE, ECAP_SYNC_OUT_SYNCI);
    ECAP_setSyncOutMode(ECAP3_BASE, ECAP_SYNC_OUT_SYNCI);
    ECAP_setSyncOutMode(ECAP4_BASE, ECAP_SYNC_OUT_SYNCI);
    ECAP_setSyncOutMode(ECAP5_BASE, ECAP_SYNC_OUT_SYNCI);
    ECAP_setSyncOutMode(ECAP6_BASE, ECAP_SYNC_OUT_SYNCI);



    ECAP_startCounter(ECAP1_BASE);
    ECAP_startCounter(ECAP2_BASE);
    ECAP_startCounter(ECAP3_BASE);
    ECAP_startCounter(ECAP4_BASE);
    ECAP_startCounter(ECAP5_BASE);
    ECAP_startCounter(ECAP6_BASE);


    ECAP_enableTimeStampCapture(ECAP1_BASE);
    ECAP_enableTimeStampCapture(ECAP2_BASE);
    ECAP_enableTimeStampCapture(ECAP3_BASE);
    ECAP_enableTimeStampCapture(ECAP4_BASE);
    ECAP_enableTimeStampCapture(ECAP5_BASE);
    ECAP_enableTimeStampCapture(ECAP6_BASE);



    ECAP_reArm(ECAP1_BASE);
    ECAP_reArm(ECAP2_BASE);
    ECAP_reArm(ECAP3_BASE);
    ECAP_reArm(ECAP4_BASE);
    ECAP_reArm(ECAP5_BASE);
    ECAP_reArm(ECAP6_BASE);



    ECAP_enableInterrupt(ECAP1_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_enableInterrupt(ECAP2_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_enableInterrupt(ECAP3_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_enableInterrupt(ECAP4_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_enableInterrupt(ECAP5_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_enableInterrupt(ECAP6_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
}

//
// eCAP 1 ISR
//
__interrupt void ecap1ISR(void)
{
    //Get the capture counts.

    PCB_A_Temp_P = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_1);




    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP1_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(ECAP1_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP1_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}

// eCAP 2 ISR
//
__interrupt void ecap2ISR(void)
{
    //
    // Get the capture counts.
    //
    PCB_B_Temp_P = ECAP_getEventTimeStamp(ECAP2_BASE, ECAP_EVENT_1);


    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP2_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(ECAP2_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP2_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}

// eCAP 3 ISR
//
__interrupt void ecap3ISR(void)
{
    //
    // Get the capture counts.
    //
    PCB_C_Temp_P = ECAP_getEventTimeStamp(ECAP3_BASE, ECAP_EVENT_1);


    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP3_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(ECAP3_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP3_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}
// eCAP 4 ISR
interrupt void ecap4ISR(void)
{
    //
    // Get the capture counts.
    //
    PCB_C_Temp_P = ECAP_getEventTimeStamp(ECAP4_BASE, ECAP_EVENT_1);


    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP4_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(ECAP4_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP4_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}
// eCAP 5 ISR
interrupt void ecap5ISR(void)
{
    //
    // Get the capture counts.
    //
    PCB_C_Temp_P = ECAP_getEventTimeStamp(ECAP5_BASE, ECAP_EVENT_1);


    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP5_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(ECAP5_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP5_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}

// eCAP 6 ISR
interrupt void ecap6ISR(void)
{
    //
    // Get the capture counts.
    //
    PCB_C_Temp_P = ECAP_getEventTimeStamp(ECAP6_BASE, ECAP_EVENT_1);


    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP6_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_1);
    ECAP_clearGlobalInterrupt(ECAP6_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP6_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}
//
// error - Error function
//
void error()
{
    ESTOP0;
}
