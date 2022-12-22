/**
  ******************************************************************************
  * @file    EEPROM_Emul/STM32G0/Src/main.c
  * @author  MCD Application Team
  * @brief   Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/** @addtogroup STM32G0xx_HAL_Applications
  * @{
  */

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_OK       LED4
#define LED_KO       LED4
#define FLAG_WUF PWR_FLAG_WUF2
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint32_t Index = 1;
__IO uint32_t ErasingOnGoing = 0;
uint32_t a_VarDataTab[NB_OF_VARIABLES] = {0};
uint32_t VarValue = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void PVD_Config(void);
static void EXTI13_IRQHandler_Config(void);
static void Error_Handler(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  EE_Status ee_status = EE_OK;

/* STM32G0xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the system clock to 64 MHz */
  SystemClock_Config();

  /* Enable and set FLASH Interrupt priority */
  /* FLASH interrupt is used for the purpose of pages clean up under interrupt */
  HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(FLASH_IRQn);

  /* Unlock the Flash Program Erase controller */
  HAL_FLASH_Unlock();

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);
 
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
    
  /* Activate NMI generation when two errors are detected */
  __HAL_FLASH_ENABLE_IT(FLASH_IT_ECCC);	

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
    if (__HAL_PWR_GET_FLAG(FLAG_WUF) != RESET)
    {
      __HAL_PWR_CLEAR_FLAG(FLAG_WUF);
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

    ee_status|= EE_WriteVariable32bits(2, ~VarValue);
    ee_status|= EE_ReadVariable32bits(2, &a_VarDataTab[1]);
    if (~VarValue != a_VarDataTab[1]) {Error_Handler();}

    ee_status|= EE_WriteVariable32bits(3, VarValue << 1);
    ee_status|= EE_ReadVariable32bits(3, &a_VarDataTab[2]);
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
   /* Turn LED_OK on for 3sec before entering standby mode */
    BSP_LED_Toggle(LED_OK);
    HAL_Delay(3000);

    /* Configure EXTI13 to wakeup from Standby (WKUP2) */
    EXTI13_IRQHandler_Config();
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2_HIGH);
    
    /* Wait for any cleanup to complete before entering standby/shutdown mode */
    while (ErasingOnGoing == 1) { }
    
    /* Clear all related wakeup flags*/
    __HAL_PWR_CLEAR_FLAG(FLAG_WUF);
    
    /* Enter low-power mode */
    /* NOTE: Care must be taken when using shutdown mode.
             The SBF bit from the PWR_SR1 register used in this example can not
             be used anymore. The system status has to be stored in the
             RTC backup registers. */
    HAL_PWR_EnterSTANDBYMode();

    /* This code should never go beyond this point. Reset on Standby wakeup */
    Error_Handler();
  }
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
  ErasingOnGoing = 0;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks at 64MHz (top STM32G0 performance)
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
  HAL_NVIC_SetPriority(PVD_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(PVD_IRQn);
}

/**
  * @brief  Configures EXTI line 13 (connected to PC.13 pin) in interrupt mode
  * @param  None
  * @retval None
  */
static void EXTI13_IRQHandler_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable GPIOC clock */
  USER_BUTTON_GPIO_CLK_ENABLE();

  /* Configure PC.13 pin as input floating */
  GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
  GPIO_InitStructure.Pin = USER_BUTTON_EXTI_LINE;
  HAL_GPIO_Init(USER_BUTTON_GPIO_PORT, &GPIO_InitStructure);

  /* Enable and set EXTI line 13 Interrupt to the lowest priority */
  HAL_NVIC_SetPriority(USER_BUTTON_EXTI_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(USER_BUTTON_EXTI_IRQn);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void)
{
  while(1)
  {
    /* Toggle LED_KO (Red) fast */
    BSP_LED_Toggle(LED_KO);
    HAL_Delay(100);
  }
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
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
