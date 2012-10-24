#include "Application.h"
#include "Callback.h"

unsigned int userBuffer[4] = {0};                                               //!< Data buffer defined by user.

void main()
{
  InitSystem();                                                                 // Initialize peripherals, application and protocol.

  if(!ADCConversionDone){
      // ZZZ initial send 0
      userBuffer[0] = 0;
      WirelessOperation(&userBuffer[0]);
  }
  
  while(1)
  {
    // ---------- Get data from hardware-----------
    if (ADCConversionDone){                                                    // Check for new analog data (local temperature).
      ADCConversionDone = 0;                                                   // Clear flag.
      userBuffer[0] = ADCGetConversionValue(__BSP_ADC1);                       // Get temperature data from ADC(hardware) and store it to user buffer.
//      #ifdef __COMMAND_INTERFACE
//        MapSensorsFromHW(0, &userBuffer[0]);                                   // Get local only hardware data to send it to the paired module.
//        ReportData(0);                                                         // Report Local only hardware states to GUI(temperature and RedLED)
//      #endif
    
      // ---------- Send data to radio (SENSOR)-----------
      WirelessOperation(&userBuffer[0]);
    }

    // ------------ Go to sleep if there is nothing else to do -------------
    __BSP_LOW_POWER_MODE(LOW_POWER_MODE);                                      // Nothing else to do.  Sleep until next interrupt arrives.
  }
}
