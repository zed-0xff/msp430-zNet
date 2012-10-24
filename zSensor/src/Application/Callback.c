/**
 *  @file Callback.c
 *
 *  @brief Function Definitions for ISR Extensions, brought upto Application Layer.
 *
 *  @version  1.0
 *
 *  @attention IMPORTANT: Your use of this Software is limited to those specific 
 *             rights granted under the terms of a software license agreement 
 *             between the user who downloaded the software, his/her employer 
 *             (which must be your employer) and Anaren (the "License"). You may
 *             not use this Software unless you agree to abide by the terms of 
 *             the License. The License limits your use, and you acknowledge,
 *             that the Software may not be modified, copied or distributed unless
 *             in connection with an authentic Anaren product. Other than for the 
 *             foregoing purpose, you may not use, reproduce, copy, prepare 
 *             derivative works of, modify, distribute, reverse engineer, decompile,
 *             perform, display or sell this Software and/or its documentation 
 *             for any purpose. 
 *             YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION
 *             ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS 
 *             OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY  WARRANTY OF 
 *             MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR 
 *             PURPOSE. IN NO EVENT SHALL ANAREN OR ITS LICENSORS BE LIABLE OR
 *             OBLIGATED UNDER CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION,
 *             BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR 
 *             INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY 
 *             INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, 
 *             LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS,
 *             TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT
 *             NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS. 
 */

#include "Callback.h"

volatile unsigned char ADCConversionDone = 0;                                            //!< Flag to indicate that ADC conversion is done.
volatile unsigned char pushButtonAction = 0;                                             //!< Specify push button action based on the press type.
volatile unsigned char swTimerAction = 0;                                                //!< Specify software timer action.

LEDStateControl greenLEDStateControl = {0,0,0,0};                               //!< Initialize green LED state controls.                          
LEDStateControl redLEDStateControl = {0,0,0,0};                                 //!< Initialize red LED state controls.   


/**
 * @fn unsigned char ADCEventHandler(const void * params)
 *
 * @brief This function is an extension of Data Converter ISR. It sets a flag 
 *        that indicates the application that Sensor ADC data is available.
 * 
 * @param params a constant void pointer
 *
 * @return an unsigned character
 */
unsigned char ADCEventHandler(const void * params)
{
  // Get event parameters.
  EventParameters * p = (EventParameters *) params;

  if(p->event == __BSP_ADC1_CHAN9_EVENT)
  {
    ADCConversionDone = 1;                                                      // Set sensor ADC data available flag for application.
  }
  
  return EVENT_WAKE_UP;
}

/**
 * @fn unsigned char PortEventHandler(const void * params)
 *
 * @brief This function is an extension of GPIO Port ISR.
 * 
 * @param params a constant void pointer
 *
 * @return an unsigned character
 */
unsigned char PortEventHandler(const void * params)
{
  // Get event parameters.
  EventParameters * p = (EventParameters *) params;

  if (p->event & __BSP_PUSHBUTTON1_EVENT)
  {
    PushButtonEventHandler(__BSP_PUSHBUTTON1, __BSP_CONFIG_PUSHBUTTON_MINIMUM_CLICK_TIME);
  }
  
  if (p->event & __BSP_RADIO1_GDO0_EVENT)
  {
    volatile unsigned char tempVar  = 0;
  
    if(__BSP_RADIO1_GDO0_EVENT & __BSP_RADIO1_GDO0_EDGE_SELECT)                 // End of Packet
    {
      if(pCurrentFrameworkRadio(&appRadio[RadioLocal.currentRadio])->radioState == RADIO_TRANSMIT){
        RadioLocal.FrameState = FRAME_SENT;                                     // Update frame state
        //GDO0 asserted slightly before RF transmission ends, so add a little delay
        for(tempVar=0;tempVar<200;tempVar++){
          __no_operation();
        }
      }
      else if(pCurrentFrameworkRadio(&appRadio[RadioLocal.currentRadio])->radioState == RADIO_RECEIVE){
        RadioLocal.FrameState = FRAME_RECEIVED;                                 // Update frame state
        TimerCCRDisableInterrupt(__BSP_TIMER1, __BSP_TIMER1_CCR1);
      }
      pCurrentFrameworkRadio(&appRadio[RadioLocal.currentRadio])->radioState = RADIO_CALIBRATED;
    }
    else                                                                        // SyncWord sent/received
    {
      RadioSetInterruptEdge(pCurrentFrameworkRadio(&appRadio[RadioLocal.currentRadio]), __BSP_RADIO1_INTERRUPT1, 1);
      
      RadioLocal.FrameState = WAITING_FOR_EOP;                                  // Update frame state
  
      StartTimeoutTimer(EOP_TIMEOUT);                                           // Load timer with timeout value.  The radio should be placed in a low-power state if frame timeout occurs.
    }
  }

  return EVENT_WAKE_UP;
}

/**
 * @fn unsigned char UARTRXEventHandler(const void * params)
 *
 * @brief This function is an extension of UART Receive ISR.
 * 
 * @param params a constant void pointer
 *
 * @return an unsigned character
 */
unsigned char UARTRXEventHandler(const void * params)
{
  unsigned char readValue = 0;

  // Get event parameters.
  EventParameters * p = (EventParameters *) params;

  // Check event type.
  if (p->event == __BSP_UART1_RX_EVENT)
  {
    UARTReceiveByte(__BSP_UART1, &readValue);
    CmdIFCollect(readValue);                                                    // Collect the received command character by character.
  }
  
  return EVENT_WAKE_UP;
}

/**
 * @fn unsigned char Timer1CCREventHandler(const void * params)
 *
 * @brief This function is an extension of Timer1 ISR.
 * 
 * @param params a constant void pointer
 *
 * @return an unsigned character
 */
unsigned char Timer1CCREventHandler(const void * params)
{
  // Get event parameters.
  EventParameters * p = (EventParameters *) params;
  
  // Check event type.
  if (p->event == __BSP_TIMER1_CCR0_EVENT)
  {
    // moved the code here for config timeout, and it does not work!
    if(ConfigTimeout.configTimeoutCount)
    {
      ConfigTimeout.configTimeoutCount--;
      if(!ConfigTimeout.configTimeoutCount)
      {
        ConfigTimeout.configTimeout = 1;
      }
    }
  }
  else if(p->event & __BSP_TIMER1_CCR1_EVENT)
  {
    TimerCCRDisableInterrupt(__BSP_TIMER1, __BSP_TIMER1_CCR1);
    RadioLocal.frameTimeout = 1;
  }
  else if(p->event & __BSP_TIMER1_CCR2_EVENT)
  {
  }
  
  return EVENT_WAKE_UP;
}

/**
 * @fn unsigned char Timer2CCREventHandler(const void * params)
 *
 * @brief This function is an extension of Timer2 ISR.
 * 
 * @param params a constant void pointer
 *
 * @return an unsigned character
 */
unsigned char Timer2CCREventHandler(const void * params)
{
  return EVENT_NO_ACTION;
}

/**
 * @fn unsigned char SWTimer0EventHandler(const void * params)
 *
 * @brief Software Timer0 Event Handler.
 * 
 * @param params a constant void pointer
 *
 * @return an unsigned character
 */
unsigned char SWTimer0EventHandler(const void * params)
{
  swTimerAction = DISABLE_PAIRING;
  
  return EVENT_NO_ACTION;
}

