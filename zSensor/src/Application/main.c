#include "Application.h"
#include "Callback.h"

unsigned int userBuffer[4] = {0};                                               //!< Data buffer defined by user.

void main()
{
  unsigned int nMinutes = 0;
  InitSystem();                                                                 // Initialize peripherals, application and protocol.
  
  while(1)
  {
    userBuffer[0] = nMinutes;
    WirelessOperation(&userBuffer[0]);

    Delay(6000);
    nMinutes++;
//    LEDToggle(__BSP_LEDRED1);
    
    // ------------ Go to sleep if there is nothing else to do -------------
 //   __BSP_LOW_POWER_MODE(LOW_POWER_MODE);                                      // Nothing else to do.  Sleep until next interrupt arrives.
  }
}
