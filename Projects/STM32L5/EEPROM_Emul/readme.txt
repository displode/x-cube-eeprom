/**
  @page EEPROM_Emulation application to show an eeprom emulation

  @verbatim
  ******************************************************************************
  * @file    EEPROM_Emul/STM32L5/readme.txt 
  * @author  MCD Application Team
  * @brief   Description of the EEPROM_Emulation application.
  ******************************************************************************
  *
  * Copyright (c) 2020 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                       opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
   @endverbatim

@par Example Description

This application is a software solution that allows substituting a standalone EEPROM 
by emulating the EEPROM mechanism using the on-chip Flash devices. The emulation
is achieved by using at least two Flash pages and the coherence mechanism is ensured 
by swapping between the Flash pages.
The system is configured and used in non-secure mode.

NUCLEO-STM32L552ZE-Q's LEDs can be used to monitor the application status:
  - LED2 (BLUE) blinks twice at start up
  - LED2 (BLUE) remains lit for 3s after demonstration completes successfully and before entering Standby Mode
  - LED2 (BLUE) blinks once upon wakeup from Standby Mode
  - LED3 (RED) toggles in case of error.
  
The EEPROM demonstration completes in approximately 15-20s and then enters Standby mode.

NUCLEO-STM32L552ZE-Q's push buttons can be used to generate events:
  - Press the Reset (Black) button during the emulated EEPROM demonstration to restart it; in this case, it formats the emulated EEPROM and starts the read and write operations. The Reset button can also be used to wake up the MCU from Standby mode; upon wakeup, the emulated EEPROM is initialized (but not formatted) and the demonstration restarts.
  - Press the User Button (B1) to wake up the MCU from Standby mode; upon wakeup, the emulated EEPROM is initialized and the demonstration restarts.


@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application need to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 


 - EEPROM_Emul/STM32L5/Inc/eeprom_emul_conf.h          EEPROM Emulation Configuration file
 - EEPROM_Emul/STM32L5/Inc/main.h                      Header for main.c module 
 - EEPROM_Emul/STM32L5/Inc/stm32l5xx_hal_conf.h        HAL Configuration file
 - EEPROM_Emul/STM32L5/Inc/stm32l5xx_it.h              Header for stm32l5xx_it.c
 - EEPROM_Emul/STM32L5/Src/main.c                      Main program
 - EEPROM_Emul/STM32L5/Src/stm32l5xx_it.c              Interrupt handlers
 - EEPROM_Emul/STM32L5/Src/system_stm32l5xx.c          STM32L5xx system clock configuration file 

@par Hardware and Software environment 

  - This example runs on STM32L552xx devices.
    
  - This example has been tested with NUCLEO-STM32L552ZE-Q board and can be
    easily tailored to any other supported device and development board.

@par How to use it ? 

In order to make the program work, you must do the following :
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the example

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
