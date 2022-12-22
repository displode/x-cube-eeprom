/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN PD */

#define LED_OK       LED_GREEN
#define LED_KO       LED_RED
/* At the end of this emulation the STM32 is put in standby mode.
 * You can exit standby mode thanks to WAKEUP PIN 3 (PC12).
 * Unfortunatelly no WAKEUP PIN is associated to a button for P-NUCLEO-WB55.Nucleo.
 * So to use WAKEUP PIN 3 PC12, you need to link it to GND trough a pull down resistor.
 * When you link the node between PC12 and this resistor to 3.3V after the system has
 * entered standby mode, it triggers a wakeup event and exit standby mode.
*/
#define PWR_FLAG_WUF PWR_FLAG_WUF3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint32_t Index = 1;
__IO uint32_t ErasingOnGoing = 0;
uint32_t a_VarDataTab[NB_OF_VARIABLES] = {0};
uint32_t VarValue = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static void PVD_Config(void);
static void EXTI12_IRQHandler_Config(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  EE_Status ee_status = EE_OK;
  
  /* USER CODE END 1 */
  
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* Enable and set FLASH Interrupt priority */
  /* FLASH interrupt is used for the purpose of pages clean up under interrupt */
  HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(FLASH_IRQn);
  
  /* Unlock the Flash Program Erase controller */
  HAL_FLASH_Unlock();
  
  /* Disable the wake up pin */
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN3);
  
  /* Clear OPTVERR bit */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_OPTVERR) != RESET) ;

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */
  
  /* Configure Programmable Voltage Detector (PVD) (optional) */
  /* PVD interrupt is used to suspend the current application flow in case
     a power-down is detected, allowing the flash interface to finish any
     ongoing operation before a reset is triggered. */
  PVD_Config();
  
  /* Configure LED_KO & LED_OK */
  BSP_LED_Init(LED_KO);
  BSP_LED_Init(LED_OK);
  BSP_LED_Off(LED_KO);
  BSP_LED_Off(LED_OK);
  
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  
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
  
  /* Store 10 values of all variables in EEPROM, ascending order */
  for (VarValue = 1; VarValue <= 10; VarValue++)
  {
    for (Index = 1; Index < NB_OF_VARIABLES+1; Index++)
    {
      /* Wait any cleanup is completed before accessing flash again */
      while (ErasingOnGoing == 1) { }

      ee_status = EE_WriteVariable32bits(Index, Index*VarValue);
      ee_status|= EE_ReadVariable32bits(Index, &a_VarDataTab[Index-1]);
      if (Index*VarValue != a_VarDataTab[Index-1]) {Error_Handler();}
      
      /* Start cleanup IT mode, if cleanup is needed */
      if ((ee_status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) {ErasingOnGoing = 1;ee_status|= EE_CleanUp_IT();}
      if ((ee_status & EE_STATUSMASK_ERROR) == EE_STATUSMASK_ERROR) {Error_Handler();}
    }
  }

  /* Read all the variables */
  for (Index = 1; Index < NB_OF_VARIABLES+1; Index++)
  {
    ee_status = EE_ReadVariable32bits(Index, &VarValue);
    if (VarValue != a_VarDataTab[Index-1]) {Error_Handler();}
    if (ee_status != EE_OK) {Error_Handler();}
  }

  /* Store 1000 values of Variable1,2,3 in EEPROM */
  for (VarValue = 1; VarValue <= 1000; VarValue++)
  {
    while (ErasingOnGoing == 1) { }

    ee_status = EE_WriteVariable32bits(1, VarValue);    
    ee_status|= EE_ReadVariable32bits(1, &a_VarDataTab[0]);
    if (VarValue != a_VarDataTab[0]) {Error_Handler();}

    ee_status |= EE_WriteVariable32bits(2, ~VarValue);
    ee_status|= EE_ReadVariable32bits(2, &a_VarDataTab[1]);
    if (~VarValue != a_VarDataTab[1]) {Error_Handler();}

    ee_status |= EE_WriteVariable32bits(3, VarValue << 1);
    ee_status |= EE_ReadVariable32bits(3, &a_VarDataTab[2]);
    if ((VarValue << 1) != a_VarDataTab[2]) {Error_Handler();}

    /* Start cleanup polling mode, if cleanup is needed */
    if ((ee_status & EE_STATUSMASK_CLEANUP) == EE_STATUSMASK_CLEANUP) {ErasingOnGoing = 0;ee_status|= EE_CleanUp();}
    if ((ee_status & EE_STATUSMASK_ERROR) == EE_STATUSMASK_ERROR) {Error_Handler();}
  }

  /* Read all the variables */
  for (Index = 1; Index < NB_OF_VARIABLES+1; Index++)
  {
    ee_status = EE_ReadVariable32bits(Index, &VarValue);
    if (VarValue != a_VarDataTab[Index-1]) {Error_Handler();}
    if (ee_status != EE_OK) {Error_Handler();}
  }
  
  /* Test is completed successfully */
  /* Lock the Flash Program Erase controller */
  HAL_FLASH_Lock();
  
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    /* Turn LED_OK on for 3sec before entering standby mode */
    BSP_LED_Toggle(LED_OK);
    HAL_Delay(3000);
    
    /* Configure EXTI12 to wakeup from Standby (WKUP3) */
    EXTI12_IRQHandler_Config();
    
    /* Wait for any cleanup to complete before entering standby/shutdown mode */
    while (ErasingOnGoing == 1) { }
    
    /* Check if the system was resumed from StandBy mode */
    /* Note: On STM32WB, both CPU1 and CPU2 must be in standby mode to set the entire system in standby mode */
    if(   (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
       && (__HAL_PWR_GET_FLAG(PWR_FLAG_C2SB) != RESET)
    )
    {
      /* Clear Standby flag */
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB); 
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_C2SB);
    }
    
    /* The Following Wakeup sequence is highly recommended prior to each Standby mode entry
      mainly  when using more than one wakeup source this is to not miss any wakeup event.
      - Disable all used wakeup sources,
      - Clear all related wakeup flags, 
      - Re-enable all used wakeup sources,
      - Enter the Standby mode.
    */
    /* Disable all used wakeup sources*/
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN3);
    /* Clear all related wakeup flags */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    /* Enable all used wakeup sources*/
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3);
    
    /* Specific procedure on STM32WB, in case of initial power-up and RF stack no started */
    /* Note: This procedure is required when user application wants to request  */
    /*       a low-power mode in the particular case:                           */
    /*       - RF stack not started: On STM32WB, system low-power mode is fixed */
    /*         by the deepest low-power modes of each sub-system (CPU1,         */
    /*         CPU2, RF).                                                       */
    /*         Standard case is RF stack started and managing low-power modes   */
    /*         of CPU2 and RF.                                                  */
    /*         In case of RF stack not started, CPU2 low-power mode must be     */
    /*         forced to the lowest level. This allows to require all system    */
    /*         low-power modes using only PWR for CPU1.                         */
    /*         low-power mode.                                                  */
    /*       - Initial power-up: In case of power-on reset, CPU2 low-power mode */
    /*         has its reset value and must be set.                             */
    /*         In case of system is resumed from low-power mode standby         */
    /*         or shutdown, configuration of PWR parameters related to CPU2 are */
    /*         retained and must not modified (This check is required in case   */
    /*         of RF stack started afterwards and not to overwritte its         */
    /*         low-power configuration).                                        */
    if(   (LL_PWR_IsActiveFlag_C1SB() == 0)
     || (LL_PWR_IsActiveFlag_C2SB() == 0)
    )
    {
      /* Set the lowest low-power mode for CPU2: shutdown mode */
      LL_C2_PWR_SetPowerMode(LL_PWR_MODE_SHUTDOWN);
    }
    
    /* Enter low-power mode */
    HAL_PWR_EnterSTANDBYMode();

    /* This code should never go beyond this point. Reset on Standby wakeup */
    Error_Handler();    
    
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE1;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  FLASH end of operation interrupt callback.
  * @param  ReturnValue: The value saved in this parameter depends on the ongoing procedure
  *                  Mass Erase: Bank number which has been requested to erase
  *                  Page Erase: Page which has been erased
  *                    (if 0xFFFFFFFF, it means that all the selected pages have been erased)
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
  ErasingOnGoing = 0;
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
  * @brief  Configures EXTI line 12 (connected to Wake Up Pin 3) in interrupt mode
  * @param  None
  * @retval None
  */
static void EXTI12_IRQHandler_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOC clock */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /* Configure PC.12 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = GPIO_PIN_12;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

  /* Enable and set EXTI line 12 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  while(1)
  {
    /* Toggle LED_KO (Red) fast */
    BSP_LED_Toggle(LED_KO);
    HAL_Delay(40);
  }
  
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
  
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
