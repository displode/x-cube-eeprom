---
pagetitle: Release Notes for STM32 Middleware WPAN
lang: en
---

::: {.row}

::: {.col-sm-12 .col-lg-4}

::: {.card .fluid}

::: {.sectione .dark}

<center>
# <small>Release Notes for</small> STM32 Middleware WPAN
Copyright &copy; 2019 STMicroelectronics\
    
[![ST logo](_htmresc/st_logo.png)](https://www.st.com){.logo}
</center>

:::

:::

License
=======

This software component is licensed by ST under Ultimate Liberty license
SLA0044, the "License";

You may not use this file except in compliance with the License.

You may obtain a copy of the License at:
[SLA0044](http://www.st.com/SLA0044)

Purpose
=======

STM32 Wireless Personal Area Network Middleware developed within the
STM32WB framework is used to support:

-   Bluetooth Low Energy 5 Certified Applications

-   802.15.4 Thread Certified Applications (based on
    [OpenThread](thread\openthread\Release_Notes.html) stack)

-   802.15.4 MAC layer

-   Zigbee Applications

**NOTE** : Depending on the kind of Application targeted, the appropriate
Wireless Coprocessor Firmware needs to be loaded.

All available binaries are located
under/Projects/STM32\_Copro\_Wireless\_Binaries directory.

Refer to UM2237 to learn how to use/install STM32CubeProgrammer.

Refer to /Projects/STM32\_Copro\_Wireless\_Binaries/ReleaseNote.html for
the detailed procedure to change the Coprocessor binary.
:::

::: {.col-sm-12 .col-lg-8}
Update History
==============

::: {.collapse}
<input type="checkbox" id="collapse-section7" checked aria-hidden="true">
<label for="collapse-section7" aria-hidden="false">V1.5.0 / 22-January-2020</label>
<div>

## Main Changes

Interface:

-	Added new commmand SHCI_C2_SetFlashActivityControl() to configure BLE timing protection

Zigbee:

- Support of the following Zigbee clusters:
    - Basic
    - On/Off
    - Device Temperature Configuration
    - Identify
    - Power Profile
    - Thermostat-UI-Config
    - Ballast-Configuration
    - Illuminance-Measurement
    - Temperature Measurement
    - Pressure Measurement
    - Occupancy-Sensing
    - Messaging
    - Meter Identification

BLE-Mesh:

- BLE-Mesh library version 1.12.000
    - Embedded Provisioner example added including config-client
    - Multi Net Key support has been added(limited to 2 Net Keys)
    - Key Refresh updated for Multiple Keys
    - Vendor Model updated to add Read and Write messages
    - TID check added in the firmware
    - Light_LC sensor property updated
    - Generic Power OnOff Message updated
    - Multi elements support limited to 3


</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6" aria-hidden="true">
<label for="collapse-section6" aria-hidden="true">V1.4.0 / 22-November-2019</label>
<div>

## Main Changes

Interface:

-	Added new commmand SHCI_C2_ExtpaConfig() to support external PA
-	Update System and BLE transport layer so that a user event packet is released by default

BLE:

-	Added support to ACI_GATT_READ_EX_EVENT
-	Added macro HCI_LE_ADVERTISING_REPORT_RSSI_0(p) to extract properly RSSI from the event packet


</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5" aria-hidden="true">
<label for="collapse-section5" aria-hidden="true">V1.3.0 / 4-September-2019</label>
<div>

## Main Changes

General:

-   Introducing support of Zigbee 

Interface:

-	Cleanup inclusion file dependencies
-	Fix local buffer size used in command SHCI_C2_FUS_StoreUsrKey()
-	ble user events are now reported one by one (hci_notify_asynch_evt is called for each packet reported)
-	system users event are now reported one by one (shci_notify_asynch_evt is called for each packet reported)

Zigbee:

- 	First release supporting Zigbee (FFD :Full Function Device)
-	Supporting only On/Off Cluster

BLE-Mesh:

-	BLE-Mesh library version 1.10.004
	-	Light LC Controller state machine for Occupancy detection is implemented
	-	Light LC Controller state machine for Ambient Lux Level is implemented 
	-	Light LC Server Mode messages added
	-	Light LC Server Occupancy Mode messages added
	-	Light LC Server Property messages added
	-	Save State in NVM controlled by Macro ENABLE_SAVE_MODEL_STATE_NVM
	-	Mode of Save State in NVM selected by Macro SAVE_MODEL_STATE_FOR_ALL_MESSAGES and SAVE_MODEL_STATE_POWER_FAILURE_DETECTION
	-	Generic Power OnOff message added
	-	Generic Power OnOff Setup message added
	-	Generic Default Transition Time message added

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" aria-hidden="true">
<label for="collapse-section4" aria-hidden="true">V1.2.0 / 27-June-2019</label>
<div>

## Main Changes

General:

-   Following utilities : Scheduler and Low Power Manager reworked and moved to "Utilities" directory
-	Split tl_if.c into hci_tl_if.c (BLE) and shci_tl_if.c (System) to remove dependencies to the BLE library when the application is built on top of HCI.

BLE:

- 	Add 2 new GATT events: aci_gatt_indication_event_rp0 and aci_gatt_notification_event_rp0
-	Rework BLE folder architecture to separate the Core from the Services implementation
-	Mesh library V1.10.000:
	- 	Sensors Model example updated 
	-	Sensor data publishing updated 
	-	PB-ADV implementation added 
	-	APIs to control frequency of unprovisioned device beacon, secure network beacon, provisioning service advertisement and proxy service advertisement 
	-	Node unprovision on 5 consecutive Power Replug or Reset cycle with duration of each cycle(ON) less than 2 seconds 
	-	SIG Models handling optimized 
	-	printf statement is replaced with TRACE_M(Function name print) and TRACE_I in application code 

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3" aria-hidden="true">
<label for="collapse-section3" aria-hidden="true">V1.1.1 / 10-May-2019</label>
<div>

## Main Changes

BLE:

-	Fix race condition in transport layer when an operating system is used.
-   Mesh Library V1.09.000:
	-	Fix of the BD Address issue.
	-	Light HSL Model implementation.
	-	Sensor Server Model Example.
	-	CID, PID Configuration.
-	STM32 Cryptographic Library V3.1.1/ 20 April 2018:
	-	Two new STM32 Cryptographic Libraries provided for IAR Embedded Workbench for ARM (EWARM) Toolchain v8.22:
		-	**STM32CryptographicV3.1.1_CM4_IARv8.a**: First official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High size*** optimization.
		-	**STM32CryptographicV3.1.1_CM4_IARv8_otnsc.a**: First official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High speed*** optimization and the option ***No Size constraints*** is enabled.
-	STM32 Cryptographic Library V3.0.0/ 05 June 2015:
	-	Two STM32 Cryptographic Libraries provided for each Development Toolchain:
		-	IAR Embedded Workbench for ARM (EWARM) toolchain v7.40:
			-	**STM32CryptographicV3.0.0_CM4_IAR.a**: New official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High size*** optimization.
			-	**STM32CryptographicV3.0.0_CM4_IAR_otnsc.a**: First official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High speed*** optimization and the option ***No Size constraints*** is enabled.
		-	RealView Microcontroller Development Kit (MDK-ARM) toolchain v5.14:
			-	**STM32CryptographicV3.0.0_CM4_KEIL_slsm1elfspf.lib**: New official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High size*** optimization.
			-	**STM32CryptographicV3.0.0_CM4_KEIL_otslsm1elfspf.lib**: First official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High speed*** optimization and the option ***Split Load and Store Multiple*** and ***One elf Section per Function*** are enabled.
		-	Atollic TrueSTUDIO STM32 (TrueSTUDIO) toolchain v5.3.0:
			-	**STM32CryptographicV3.0.0_CM4_GCC.a**: First official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High size*** optimization.
			-	**STM32CryptographicV3.0.0_CM4_GCC_ot.a**: First official release of optimized STM32 Cryptographic Library for ***Cortex M4*** with ***High speed*** optimization.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2" aria-hidden="true">
<label for="collapse-section2" aria-hidden="true">V1.1.0 / 29-March-2019</label>
<div>

## Main Changes

General:

-   Licenses in utilities and patterns moved from sla0044 to 3-clauses  BSD
-   Customer Key Storage APIs
-   Get Wireless Coprocessor Firmware information using following API : SHCI\_GetWirelessFwInfo()

BLE:

-   New BLE Mesh lib to fix provisioning issue

Thread:

-	802.15.4 radio driver robustness improvement with additional error checks; the application is now notified in case of radio error
    detected inside the wireless binary
-	TxPower management improvement:
	-   New APIs provided to control the Tx power: otPlatRadioGetTransmitPower() and otPlatRadioSetTransmitPower()
	-	The default Tx power is now set to 0dBm\

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1" aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">V1.0.0 / 31-January-2019</label>
<div>			

## Main Changes

First release

</div>
:::


For complete documentation on STM32WBxx, visit:
\[[www.st.com/stm32wb](http://www.st.com/stm32wb)\]
:::
:::
