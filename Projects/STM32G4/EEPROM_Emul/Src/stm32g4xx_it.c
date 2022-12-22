/**
  ******************************************************************************
  * @file    EEPROM_Emul/STM32G4/Src/stm32g4xx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
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
#include "stm32g4xx_it.h"

/** @addtogroup STM32G4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* During the cleanup phase in EE_Init, AddressRead is the address being read */ 
extern __IO uint32_t AddressRead;
/* Flag equal to 1 when the cleanup phase is in progress, 0 if not */
extern __IO uint8_t CleanupPhase;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
    
  /* Check if NMI is due to flash ECCD (error detection) */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_ECCD))
  {
    if(CleanupPhase==1)
    {
      if ((AddressRead >= START_PAGE_ADDRESS) && (AddressRead <= END_EEPROM_ADDRESS))
      {
        /* Delete the corrupted flash address */
        if (EE_DeleteCorruptedFlashAddress((uint32_t)AddressRead) == EE_OK)
        {
          /* Resume execution if deletion succeeds */
          return;
        }
        /* If we do not succeed to delete the corrupted flash address */
        /* This might be because we try to write 0 at a line already considered at 0 which is a forbidden operation */
        /* This problem triggers PROGERR, PGAERR and PGSERR flags */
        else
        {
          /* We check if the flags concerned have been triggered */
          if((__HAL_FLASH_GET_FLAG(FLASH_FLAG_PROGERR)) && (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGAERR))  
             && (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGSERR)))
          {
            /* If yes, we clear them */
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PROGERR);
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGAERR);
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_PGSERR);
            
            /* And we exit from NMI without doing anything */
            /* We do not invalidate that line because it is not programmable at 0 till the next page erase */
            /* The only consequence is that this line will trigger a new NMI later */
            return;
          }
        }
      }
    }
    else
    {
      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ECCD);
      return;
    }
  }

  /* Go to infinite loop when NMI occurs in case:
     - ECCD is raised in eeprom emulation flash pages but corrupted flash address deletion fails (except PROGERR, PGAERR and PGSERR)
     - ECCD is raised out of eeprom emulation flash pages
     - no ECCD is raised */
  
  /* Go to infinite loop when NMI occurs */
  while (1)
  {
    /* Toggle LED2 fast */
    BSP_LED_Toggle(LED2);
    HAL_Delay(40); 
  }
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32G4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32g4xxxx.s).                                             */
/******************************************************************************/

/**
  * @brief  This function handles Flash interrupt request.
  * @param  None
  * @retval None
  */
void FLASH_IRQHandler(void)
{
  HAL_FLASH_IRQHandler();
}

/**
  * @brief  This function handles PVD interrupt request.
  * @param  None
  * @retval None
  */
void PVD_PVM_IRQHandler(void)
{
  /* Loop inside the handler to prevent the Cortex from using the Flash,
     allowing the flash interface to finish any ongoing transfer. */
  while (__HAL_PWR_GET_FLAG(PWR_FLAG_PVDO) != RESET)
  {
  }
}

/**
  * @brief  This function handles external line 10 to 15 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_13);
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
