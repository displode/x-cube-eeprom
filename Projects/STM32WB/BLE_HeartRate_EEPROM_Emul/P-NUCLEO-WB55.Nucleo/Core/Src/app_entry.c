/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_entry.c
 * @author  MCD Application Team
 * @brief   Entry point of the Application
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "main.h"
#include "app_entry.h"
#include "app_ble.h"
#include "ble.h"
#include "tl.h"
#include "stm32_seq.h"
#include "shci_tl.h"
#include "stm32_lpm.h"
#include "app_debug.h"

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH*4U*DIVC(( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ), 4U))

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t EvtPool[POOL_SIZE];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static TL_CmdPacket_t SystemCmdBuffer;
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t SystemSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255U];
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static uint8_t BleSpareEvtBuffer[sizeof(TL_PacketHeader_t) + TL_EVT_HDR_SIZE + 255];

/* USER CODE BEGIN PV */

/* Defines, prototypes and variables used to interface with the EEPROM emulation driver */
#define LED_OK       LED_GREEN
#define LED_KO       LED_RED
#define PWR_FLAG_WUF PWR_FLAG_WUF3
__IO uint32_t ErasingOnGoing = 0;
__IO uint32_t ErasingUpdate = 0;
uint32_t a_VarDataTab[NB_OF_VARIABLES] = {0};

#ifdef DUALCORE_FLASH_SHARING
#define HSEM_PROCESS_1 12U /* Number taken randomly to identify the process locking a semaphore in the driver context */
__IO uint32_t FlashSemaphoreTaken = 0;
static void EEPROM_Emul_Operation(void);
static void EEPROM_Emul_Ope_Launcher(void);
static void EEPROM_Emul_Init(void);
#endif

/* USER CODE END PV */

/* Private functions prototypes-----------------------------------------------*/
static void SystemPower_Config( void );
static void appe_Tl_Init( void );
static void APPE_SysStatusNot( SHCI_TL_CmdStatus_t status );
static void APPE_SysUserEvtRx( void * pPayload );
static void PVD_Config(void);

#if (CFG_HW_LPUART1_ENABLED == 1)
extern void MX_LPUART1_UART_Init(void);
#endif
#if (CFG_HW_USART1_ENABLED == 1)
extern void MX_USART1_UART_Init(void);
#endif

/* USER CODE BEGIN PFP */
static void Led_Init( void );
static void Button_Init( void );
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void APPE_Init( void )
{
  SystemPower_Config(); /**< Configure the system Power Mode */

  HW_TS_Init(hw_ts_InitMode_Full, &hrtc); /**< Initialize the TimerServer */

/* USER CODE BEGIN APPE_Init_1 */
  APPD_Init();
  
  /**
   * The Standby mode should not be entered before the initialization is over
   * The default state of the Low Power Manager is to allow the Standby Mode so an request is needed here
   */
  UTIL_LPM_SetOffMode(1 << CFG_LPM_APP, UTIL_LPM_DISABLE);

  Led_Init();

  Button_Init();
/* USER CODE END APPE_Init_1 */
  appe_Tl_Init();	/* Initialize all transport layers */

  /**
   * From now, the application is waiting for the ready event ( VS_HCI_C2_Ready )
   * received on the system channel before starting the Stack
   * This system event is received with APPE_SysUserEvtRx()
   */
/* USER CODE BEGIN APPE_Init_2 */

/* USER CODE END APPE_Init_2 */
   return;
}
/* USER CODE BEGIN FD */

/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief  Configure the system for power optimization
 *
 * @note  This API configures the system to be ready for low power mode
 *
 * @param  None
 * @retval None
 */
static void SystemPower_Config( void )
{

  /**
   * Select HSI as system clock source after Wake Up from Stop mode
   */
  LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);

  /* Initialize low power manager */
  UTIL_LPM_Init( );

#if (CFG_USB_INTERFACE_ENABLE != 0)
  /**
   *  Enable USB power
   */
  HAL_PWREx_EnableVddUSB();
#endif

  return;
}

static void appe_Tl_Init( void )
{
  TL_MM_Config_t tl_mm_config;
  SHCI_TL_HciInitConf_t SHci_Tl_Init_Conf;
  /**< Reference table initialization */
  TL_Init();

  /**< System channel initialization */
  UTIL_SEQ_RegTask( 1<< CFG_TASK_SYSTEM_HCI_ASYNCH_EVT_ID, UTIL_SEQ_RFU, shci_user_evt_proc );
  SHci_Tl_Init_Conf.p_cmdbuffer = (uint8_t*)&SystemCmdBuffer;
  SHci_Tl_Init_Conf.StatusNotCallBack = APPE_SysStatusNot;
  shci_init(APPE_SysUserEvtRx, (void*) &SHci_Tl_Init_Conf);

  /**< Memory Manager channel initialization */
  tl_mm_config.p_BleSpareEvtBuffer = BleSpareEvtBuffer;
  tl_mm_config.p_SystemSpareEvtBuffer = SystemSpareEvtBuffer;
  tl_mm_config.p_AsynchEvtPool = EvtPool;
  tl_mm_config.AsynchEvtPoolSize = POOL_SIZE;
  TL_MM_Init( &tl_mm_config );

  TL_Enable();

  return;
}

static void APPE_SysStatusNot( SHCI_TL_CmdStatus_t status )
{
  UNUSED(status);
  return;
}

/**
 * The type of the payload for a system user event is tSHCI_UserEvtRxParam
 * When the system event is both :
 *    - a ready event (subevtcode = SHCI_SUB_EVT_CODE_READY)
 *    - reported by the FUS (sysevt_ready_rsp == RSS_FW_RUNNING)
 * The buffer shall not be released
 * ( eg ((tSHCI_UserEvtRxParam*)pPayload)->status shall be set to SHCI_TL_UserEventFlow_Disable )
 * When the status is not filled, the buffer is released by default
 */
static void APPE_SysUserEvtRx( void * pPayload )
{
  UNUSED(pPayload);
  /* Traces channel initialization */
  APPD_EnableCPU2( );

  APP_BLE_Init( );
  
  /* We activate the semaphore 7 flash protection to be used by CPU2 */
  if( SHCI_C2_SetFlashActivityControl(FLASH_ACTIVITY_CONTROL_SEM7) != SHCI_Success )
  {
    Error_Handler();
  }
  
  /* EEPROM emulation driver init */
  EEPROM_Emul_Init();
  
  /* Registering of the task running EEPROM operations in the sequencer */
  UTIL_SEQ_RegTask( 1<< CFG_TASK_EEPROM_ID, UTIL_SEQ_RFU, EEPROM_Emul_Operation );
  
  /* Creation and configuration of the timer lauching the EEPROM_Emul_Operation periodically */
  uint8_t pTimerEeprom;
  HW_TS_Create(CFG_TIM_PROC_ID_ISR, &pTimerEeprom, hw_ts_Repeated, EEPROM_Emul_Ope_Launcher);
  HW_TS_Stop(pTimerEeprom);
  HW_TS_Start(pTimerEeprom, 128);

  UTIL_LPM_SetOffMode(1U << CFG_LPM_APP, UTIL_LPM_ENABLE);
  return;
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */
static void Led_Init( void )
{
#if (CFG_LED_SUPPORTED == 1)
  /**
   * Leds Initialization
   */

  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  BSP_LED_On(LED_GREEN);
#endif

  return;
}

static void Button_Init( void )
{
#if (CFG_BUTTON_SUPPORTED == 1)
  /**
   * Button Initialization
   */

  BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_EXTI);
  BSP_PB_Init(BUTTON_SW3, BUTTON_MODE_EXTI);
#endif

  return;
}
/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void UTIL_SEQ_Idle( void )
{
#if ( CFG_LPM_SUPPORTED == 1)
  UTIL_LPM_EnterLowPower( );
#endif
  return;
}

/**
  * @brief  This function is called by the scheduler each time an event
  *         is pending.
  *
  * @param  evt_waited_bm : Event pending.
  * @retval None
  */
void UTIL_SEQ_EvtIdle( UTIL_SEQ_bm_t task_id_bm, UTIL_SEQ_bm_t evt_waited_bm )
{
  UTIL_SEQ_Run( UTIL_SEQ_DEFAULT );
}

void shci_notify_asynch_evt(void* pdata)
{
  UTIL_SEQ_SetTask( 1<<CFG_TASK_SYSTEM_HCI_ASYNCH_EVT_ID, CFG_SCH_PRIO_0);
  return;
}

void shci_cmd_resp_release(uint32_t flag)
{
  UTIL_SEQ_SetEvt( 1<< CFG_IDLEEVT_SYSTEM_HCI_CMD_EVT_RSP_ID );
  return;
}

void shci_cmd_resp_wait(uint32_t timeout)
{
  UTIL_SEQ_WaitEvt( 1<< CFG_IDLEEVT_SYSTEM_HCI_CMD_EVT_RSP_ID );
  return;
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
  switch (GPIO_Pin)
  {
    case BUTTON_SW1_PIN:
     APP_BLE_Key_Button1_Action();
      break; 

    case BUTTON_SW2_PIN:
      APP_BLE_Key_Button2_Action();
      break; 

    case BUTTON_SW3_PIN:
      APP_BLE_Key_Button3_Action();
      break;

    default:
      break;

  }
  return;
}

static void EEPROM_Emul_Init(void)
{
  EE_Status ee_status = EE_OK;
  
  /* Enable and set FLASH Interrupt priority */
  /* FLASH interrupt is used for the purpose of pages clean up under interrupt */
  HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(FLASH_IRQn);
  
  /* Clear OPTVERR bit */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_OPTVERR) != RESET) ;
  
  /* Configure Programmable Voltage Detector (PVD) (optional) */
  /* PVD interrupt is used to suspend the current application flow in case
     a power-down is detected, allowing the flash interface to finish any
     ongoing operation before a reset is triggered. */
  PVD_Config();
  
  /* Configure LED_KO & LED_OK & LED_BLUE */
  BSP_LED_Init(LED_KO);
  BSP_LED_Init(LED_OK);
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Off(LED_KO);
  BSP_LED_Off(LED_OK);
  BSP_LED_Off(LED_BLUE);

  /* When a direct writing is requested - that is to say no page transfer
     is required to achieve the write - the call of EE_WriteVariableXXbits
     function can return the EE_FLASH_USED value meaning that the flash is currently
     used by CPU2 (semaphore 7 locked). When this is the case, the driver 
     also activates the interrupt associated to the release of the semaphore 
     (flash not used by CPU2 anymore).
     Then, to benefit from this possibility HSEM_IRQn interrupts are configured.
     HAL_HSEM_FreeCallback is called when the semaphore is released. */
  __HAL_RCC_HSEM_CLK_ENABLE();
  HAL_NVIC_SetPriority(HSEM_IRQn, 15, 0);
  HAL_NVIC_EnableIRQ(HSEM_IRQn);
  
  /*  Wait for the flash semaphore to be free and take it */    
  while(HAL_HSEM_Take(CFG_HW_FLASH_SEMID, HSEM_PROCESS_1) != HAL_OK)
  {
    while( HAL_HSEM_IsSemTaken(CFG_HW_FLASH_SEMID) ) ;
  }

  /* Unlock the Flash Program Erase controller for intialization */
  HAL_FLASH_Unlock();
  
  /* Set EEPROM emulation firmware to erase all potentially incompletely erased
   pages if the system came from an asynchronous reset. Conditional erase is
   safe to use if all Flash operations where completed before the system reset */
  if(__HAL_PWR_GET_FLAG(PWR_FLAG_SB) == RESET)
  {
    /* Blink LED_OK (Green) twice at startup */
    BSP_LED_On(LED_OK);
    HAL_Delay(100);
    BSP_LED_Off(LED_OK);
    HAL_Delay(100);
    BSP_LED_On(LED_OK);
    HAL_Delay(100);
    BSP_LED_Off(LED_OK);
    
    /* System reset comes from a power-on reset: Forced Erase */
    /* Initialize EEPROM emulation driver (mandatory) */
    ee_status = EE_Init(EE_FORCED_ERASE);
    if(ee_status != EE_OK) {Error_Handler();}
  }
  else
  {
    /* Clear the Standby flag */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    
    /* Check and Clear the Wakeup flag */
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_WUF) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WUF);
    }
    
    /* Blink LED_OK (Green) upon wakeup */
    BSP_LED_On(LED_OK);
    HAL_Delay(100);
    BSP_LED_Off(LED_OK);
    
    /* System reset comes from a STANDBY wakeup: Conditional Erase*/
    /* Initialize EEPROM emulation driver (mandatory) */
    ee_status = EE_Init(EE_CONDITIONAL_ERASE);
    if(ee_status != EE_OK) {Error_Handler();}
  }
  
  /* Lock the Flash Program Erase controller and release flash semaphore */
  HAL_FLASH_Lock();
  HAL_HSEM_Release(CFG_HW_FLASH_SEMID, HSEM_PROCESS_1);
}

static void EEPROM_Emul_Ope_Launcher( void )
{
  
  UTIL_SEQ_SetTask( 1<< CFG_TASK_EEPROM_ID, 1);
  return;
}

static void EEPROM_Emul_Operation(void)
{
  static uint32_t VarValue = 1;
  static uint32_t Index = 1;
  static uint32_t Step = 0;
  static EE_Status ee_status = EE_OK;
  static EE_Status ee_status2 = EE_OK;
  
  /* If an erasing pages operation has just finished, we set OFF the Erase Activity mechanism seen by CPU2 */
  if (ErasingUpdate == 1)
  {
    SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_OFF);
    ErasingUpdate = 0;
  }
  
  /* Wait any cleanup is completed before accessing flash again */
  if (ErasingOnGoing == 1)
  { 
    return;
  }
    
  /*  Wait for the flash semaphore to be free */    
  if( HAL_HSEM_IsSemTaken(CFG_HW_FLASH_SEMID) ) return;
  if(HAL_HSEM_Take(CFG_HW_FLASH_SEMID, HSEM_PROCESS_1) != HAL_OK) return;
  
  /* Unlock the Flash Program Erase controller */
  HAL_FLASH_Unlock();
  
  /* STEP 0: Store 10 values of all variables in EEPROM, ascending order, progressively each time that this task is called */
  /* Cleanup with interrupts is used if needed */
  if(Step == 0)
  {
    /* Toggle LED to show the EEPROM operations are effectively running */
    BSP_LED_Toggle(LED_BLUE);

#ifdef DUALCORE_FLASH_SHARING
    ee_status = EE_WriteVariable32bits(Index, Index*VarValue);
    /* If flash is used by CPU2 */
    if(ee_status == EE_FLASH_USED){
      /* Lock the Flash Program Erase controller and release the semaphore */
      HAL_FLASH_Lock();
      HAL_HSEM_Release(CFG_HW_FLASH_SEMID, HSEM_PROCESS_1);
      /* Give back the control to sequencer */
      return;
    }    
#else
      ee_status = EE_WriteVariable32bits(Index, Index*VarValue);
#endif
      
    ee_status|= EE_ReadVariable32bits(Index, &a_VarDataTab[Index-1]);
    if (Index*VarValue != a_VarDataTab[Index-1]) {Error_Handler();}
    
    /* Start cleanup IT mode, if cleanup is needed */
    if ((ee_status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) {ErasingOnGoing = 1; ee_status|= EE_CleanUp_IT();}
    if ((ee_status & EE_STATUSMASK_ERROR) == EE_STATUSMASK_ERROR) {Error_Handler();}
        
    /* Prepare the static parameters for the next time next the function is called by the sequencer */
    Index++;
    if(Index >= (NB_OF_VARIABLES+1) ){
      VarValue++;
      Index = 1;
    }
    if(VarValue > 10){
      BSP_LED_Off(LED_BLUE);
      VarValue = 1;
      Index = 1;
      Step++;
    }
  }
  /* STEP 1 verifies that the STEP 0 operations have been processed successfully */
  else if (Step == 1)
  {
    /* Toggle LED to show the EEPROM operations are effectively running */
    BSP_LED_Toggle(LED_GREEN);
    
    ee_status = EE_ReadVariable32bits(Index, &VarValue);
    if (VarValue != a_VarDataTab[Index-1]) {Error_Handler();}
    if (ee_status != EE_OK) {Error_Handler();}
    
    /* Prepare the static parameters for the next time next the function is called by the sequencer */
    Index++;
    if( Index == (NB_OF_VARIABLES+1) )
    {
        BSP_LED_Off(LED_GREEN);
        VarValue = 1;
        Index = 1;
        Step++;
    }
  }
  /* STEP 2: Store 100 values of Variable1,2,3 in EEPROM */
  /* Cleanup without interrupts is used if needed */
  else if (Step == 2)
  {
    /* Toggle LED to show the EEPROM operations are effetively running */
    BSP_LED_Toggle(LED_BLUE);

    /* Update Variable 1 */
#ifdef DUALCORE_FLASH_SHARING
    FlashSemaphoreTaken=1;
    ee_status = EE_WriteVariable32bits(1, VarValue);
    /* If flash is used by CPU2 we wait for the interrupt associated to the semaphore 7 releasing
      to be raised before trying to write again */
    while(ee_status == EE_FLASH_USED)
    {
      while(FlashSemaphoreTaken==1) { /* User can implement activities while waiting */ }        
      ee_status = EE_WriteVariable32bits(1, VarValue);     
    }
#else    
    ee_status = EE_WriteVariable32bits(1, VarValue);
#endif
    
    /* Verification for previous writing */
    ee_status|= EE_ReadVariable32bits(1, &a_VarDataTab[0]);
    if (VarValue != a_VarDataTab[0]) {Error_Handler();}

    /* Update Variable 2 */
#ifdef DUALCORE_FLASH_SHARING
    FlashSemaphoreTaken=1;
    ee_status2 = EE_WriteVariable32bits(2, ~VarValue);
    while(ee_status2 == EE_FLASH_USED)
    {
      while(FlashSemaphoreTaken==1) { /* User can implement activities while waiting */ }        
      ee_status2 |= EE_WriteVariable32bits(2, ~VarValue);    
    }
    ee_status |= ee_status2;
#else    
    ee_status |= EE_WriteVariable32bits(2, ~VarValue);
#endif
    
    /* Verification for previous writing */
    ee_status|= EE_ReadVariable32bits(2, &a_VarDataTab[1]);
    if (~VarValue != a_VarDataTab[1]) {Error_Handler();}

    /* Update Variable 3 */
#ifdef DUALCORE_FLASH_SHARING
    FlashSemaphoreTaken=1;
    ee_status2 = EE_WriteVariable32bits(3, VarValue << 1);
    while(ee_status2 == EE_FLASH_USED)
    {
      while(FlashSemaphoreTaken==1) { /* User can implement activities while waiting */ }        
      ee_status2 = EE_WriteVariable32bits(3, VarValue << 1);    
    }
    ee_status |= ee_status2;
#else    
    ee_status |= EE_WriteVariable32bits(3, VarValue << 1);
#endif
      
    /* Verification for previous writing */
    ee_status |= EE_ReadVariable32bits(3, &a_VarDataTab[2]);
    if ((VarValue << 1) != a_VarDataTab[2]) {Error_Handler();}
    
    /* Start cleanup mode, if cleanup is needed */
    if ((ee_status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) {ErasingOnGoing = 0; ee_status|= EE_CleanUp();}
    if ((ee_status & EE_STATUSMASK_ERROR) == EE_STATUSMASK_ERROR) {Error_Handler();}
    
    /* Prepare the static parameters for the next time the function is called by the sequencer */
    VarValue++;
    if(VarValue > 100){
      BSP_LED_Off(LED_BLUE);
      VarValue = 1;
      Index = 1;
      Step++;
    }
  }
  /* STEP 3 verifies that the STEP 2 operations have been processed successfully */
  else if (Step == 3)
  {
    /* Toggle LED to show the EEPROM operations are effetively running */
    BSP_LED_Toggle(LED_GREEN);
    
    ee_status = EE_ReadVariable32bits(Index, &VarValue);
    if (VarValue != a_VarDataTab[Index-1]) {Error_Handler();}
    if (ee_status != EE_OK) {Error_Handler();}
    
    /* Prepare the static parameters for the next time next the function is called by the sequencer */
    Index++;
    if( Index == (NB_OF_VARIABLES+1) )
    {
        BSP_LED_Off(LED_GREEN);
        VarValue = 1;
        Index = 1;
        Step = 0;
    }
  }
  
  /* Lock the Flash Program Erase controller and release flash semaphore if needed */
  HAL_FLASH_Lock();
  if(ErasingOnGoing != 1) HAL_HSEM_Release(CFG_HW_FLASH_SEMID, HSEM_PROCESS_1);
  
  return;
}

/**
  * @brief  Programmable Voltage Detector (PVD) Configuration
  *         PVD set to level 6 for a threshold around 2.9V.
  * @param  None
  * @retval None
  */
static void PVD_Config(void)
{
  PWR_PVDTypeDef sConfigPVD;
  sConfigPVD.PVDLevel = PWR_PVDLEVEL_6;
  sConfigPVD.Mode     = PWR_PVD_MODE_IT_RISING;
  if (HAL_PWR_ConfigPVD(&sConfigPVD) != HAL_OK) {Error_Handler();}

  /* Enable PVD */
  HAL_PWR_EnablePVD();
 
  /* Enable and set PVD Interrupt priority */
  HAL_NVIC_SetPriority(PVD_PVM_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PVD_PVM_IRQn);
}

/**
  * @brief  FLASH end of operation interrupt callback.
  * @param  ReturnValue: The value saved in this parameter depends on the ongoing procedure
  *                  Mass Erase: Bank number which has been requested to erase
  *                  Page Erase: Page which has been erased
  *                  Program: Address which was selected for data program
  * @retval None
  */
void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{    
  /* Call CleanUp callback when all requested pages have been erased */
  if( (ReturnValue == (START_PAGE+PAGES_NUMBER/2-1)) || (ReturnValue == (START_PAGE+PAGES_NUMBER-1)) )
  {
    EE_EndOfCleanup_UserCallback();
  }
}

/**
  * @brief  Clean Up end of operation interrupt callback.
  * @param  None
  * @retval None
  */
void EE_EndOfCleanup_UserCallback(void)
{
  /* When cleanup is finished, this function clear erasing relative bits, 
    locks back the flash, releases the flash semaphore
    and updates the variables relative to the Erasing state */
  CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));
  HAL_FLASH_Lock();
  HAL_HSEM_Release(CFG_HW_FLASH_SEMID, HSEM_PROCESS_1);
  ErasingUpdate = 1;
  ErasingOnGoing = 0;
}

#ifdef DUALCORE_FLASH_SHARING
/**
  * @brief Semaphore Released Callback.
  * @param SemMask: Mask of Released semaphores
  * @retval None
  */
void HAL_HSEM_FreeCallback(uint32_t SemMask)
{
  /* When semaphore 7 is released, this function set FlashSemaphoreTaken to 0
    so that the EEPROM operation task know it can try again to write the flash.
    User can do other operations while waiting for the release. */
  if((SemMask & __HAL_HSEM_SEMID_TO_MASK(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID))!= 0)
  {  
    FlashSemaphoreTaken = 0;
  }
}
#endif

/* USER CODE END FD_WRAP_FUNCTIONS */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
