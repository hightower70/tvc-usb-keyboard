// HardwareProfile.h

#ifndef _HARDWARE_PROFILE_H_
#define _HARDWARE_PROFILE_H_

    #define USB_A0_SILICON_WORK_AROUND
    #define RUN_AT_48MHZ
    //#define RUN_AT_24MHZ
    //#define RUN_AT_60MHZ
    
    // Various clock values
    #if defined(RUN_AT_48MHZ)
        #define GetSystemClock()            48000000UL
        #define GetPeripheralClock()        48000000UL
        #define GetInstructionClock()       (GetSystemClock() / 2) ???
    #elif defined(RUN_AT_24MHZ)
        #define GetSystemClock()            24000000UL
        #define GetPeripheralClock()        24000000UL
        #define GetInstructionClock()       (GetSystemClock() / 2) ???
    #elif defined(RUN_AT_60MHZ)    
        #define GetSystemClock()            60000000UL
        #define GetPeripheralClock()        60000000UL  // Will be divided down
        #define GetInstructionClock()       (GetSystemClock() / 2) ???
    #else
        #error Choose a speed
    #endif        

    // Clock values
    #define MILLISECONDS_PER_TICK       10                  // -0.000% error
    #define TIMER_PRESCALER             TIMER_PRESCALER_8   // At 60MHz
    #define TIMER_PERIOD                37500               // At 60MHz


//#define USE_USB_PLL

    #include <p32xxxx.h>
    #include <plib.h>


/** TRIS ***********************************************************/
#define INPUT_PIN           1
#define OUTPUT_PIN          0

#endif  

