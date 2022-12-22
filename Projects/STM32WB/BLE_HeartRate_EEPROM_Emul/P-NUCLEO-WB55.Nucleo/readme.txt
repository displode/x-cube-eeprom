/**
  @page BLE_HeartRate + EEPROM emulation example
  
  @verbatim
  ******************************************************************************
  * @file    BLE_HeartRate_EEPROM_Emul/readme.txt 
  * @author  MCD Application Team
  * @brief   Description of the BLE_HeartRate example.
  ******************************************************************************
  *
  * Copyright (c) 2020 STMicroelectronics. All rights reserved.
  *
  * This software component is licensed by ST under Ultimate Liberty license 
  * SLA0044, the "License"; You may not use this file except in compliance with 
  * the License. You may obtain a copy of the License at:
  *                               www.st.com/SLA0044
  *
  ******************************************************************************
  @endverbatim

@par Example Description

How to process EEPROM operations in parallel to running the Heart Rate profile as specified by the BLE SIG.

@note Care must be taken when using HAL_Delay(), this function provides accurate delay (in milliseconds)
      based on variable incremented in SysTick ISR. This implies that if HAL_Delay() is called from
      a peripheral ISR process, then the SysTick interrupt must have higher priority (numerically lower)
      than the peripheral interrupt. Otherwise the caller ISR process will be blocked.
      To change the SysTick interrupt priority you have to use HAL_NVIC_SetPriority() function.
      
@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 
  
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/stm32wbxx_hal_conf.h		HAL configuration file
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/stm32wbxx_it.h          	Interrupt handlers header file
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/main.h                  	Header for main.c module
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/app_ble.h          Header for app_ble.c module
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/app_common.h            	Header for all modules with common definition
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/app_conf.h              	Parameters configuration file of the application
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/app_entry.h            	Parameters configuration file of the application
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/ble_conf.h         BLE Services configuration
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/ble_dbg_conf.h     BLE Traces configuration of the BLE services
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/dis_app.h          Header for dis_app.c module
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/hrs_app.h          Header for hrs_app.c module
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/hw_conf.h           		Configuration file of the HW
  - BLE_HeartRate_EEPROM_Emul/Core/Inc/utilities_conf.h    		Configuration file of the utilities
  - BLE_HeartRate_EEPROM_Emul/Core/Src/stm32wbxx_it.c          	Interrupt handlers
  - BLE_HeartRate_EEPROM_Emul/Core/Src/main.c                  	Main program
  - BLE_HeartRate_EEPROM_Emul/Core/Src/system_stm32wbxx.c      	stm32wbxx system source file
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/app_ble.c      	BLE Profile implementation
  - BLE_HeartRate_EEPROM_Emul/Core/Src/app_entry.c      		Initialization of the application
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/dis_app.c      	Device Information Service application
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/App/hrs_app.c      	Heart Rate Service application
  - BLE_HeartRate_EEPROM_Emul/STM32_WPAN/Target/hw_ipcc.c      	IPCC Driver
  - BLE_HeartRate_EEPROM_Emul/Core/Src/stm32_lpm_if.c			Low Power Manager Interface
  - BLE_HeartRate_EEPROM_Emul/Core/Src/hw_timerserver.c 		Timer Server based on RTC
  - BLE_HeartRate_EEPROM_Emul/Core/Src/hw_uart.c 				UART Driver

     
@par Hardware and Software environment

  - This example runs on STM32WB55xx devices.
  
  - This example has been tested with an STMicroelectronics STM32WB55VG-Nucleo
    board and can be easily tailored to any other supported device 
    and development board.
	
  - This example is by default configured with low power mode deactivated.

@par How to use it ? 

This application requests having the stm32wb5x_BLE_Stack_fw.bin binary flashed on the Wireless Coprocessor.
If it is not the case, you need to use STM32CubeProgrammer to load the appropriate binary.
Those binaries are located under the /Projects/STM32_Copro_Wireless_Binaries directory in the STM32Cube_FW_WB package available on st.com..
Refer to UM2237 to learn how to use/install STM32CubeProgrammer.
Refer to /Projects/STM32_Copro_Wireless_Binaries/ReleaseNote.html for the detailed procedure to change the
Wireless Coprocessor binary.  

In order to make the program work, you must do the following:
 - Open your toolchain 
 - Rebuild all files and flash the board with the executable file

 On the android/ios device, enable the Bluetooth communications, and if not done before,
 - Install the ST BLE Profile application on the android device
	https://play.google.com/store/apps/details?id=com.stm.bluetoothlevalidation&hl=en
    https://itunes.apple.com/fr/App/st-ble-profile/id1081331769?mt=8

 - Install the ST BLE Sensor application on the ios/android device
	https://play.google.com/store/apps/details?id=com.st.bluems
	https://itunes.apple.com/us/App/st-bluems/id993670214?mt=8

 - Power on the Nucleo board with the BLE_HeartRate_EEPROM_Emul application
 - Then, click on the App icon, ST BLE Sensor (android device)
 - connect to a device
 - select the HRMEM in the device list
 - pairing is supported ( SW1 clears the security database, SW2 requests the slave req pairing )
 - This example supports switch to 2Mbits PHY ( SW3 is used to enable the feature )
 
The Heart Rate is displayed each second on the android device.

In parallel, EEPROM emulation operations handled by the task EEPROM_Emul_Operation defined in app_entry.c
are processed. The BLUE and GREEN leds toggling testifies that these operations are processed.
If the RED led toggling, it means an error has occured.

For more details on the BLE functionalities refer to the Application Note: 
  AN5289 - Building a Wireless application 
 
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
 