/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32wbxx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
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
#include "main.h"
#include "stm32wbxx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifdef DUALCORE_FLASH_SHARING
#define HSEM_PROCESS_1 12U /* Number taken randomly to identify the process locking a semaphore in the driver context */
#endif
    
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* During the cleanup phase in EE_Init, AddressRead is the address being read */ 
extern __IO uint32_t AddressRead;
/* Flag equal to 1 when the cleanup phase is in progress, 0 if not */
extern __IO uint8_t CleanupPhase;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_lpuart1_tx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern UART_HandleTypeDef hlpuart1;
extern UART_HandleTypeDef huart1;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

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
    /* Toggle RED LED fast */
    BSP_LED_Toggle(LED_RED);
    HAL_Delay(40); 
  }
  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while(1)
  {
    /* Toggle LED_RED */
    BSP_LED_Toggle(LED_RED);
    HAL_Delay(500);
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32WBxx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32wbxx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */

  /* USER CODE END DMA1_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_lpuart1_tx);
  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles LPUART1 global interrupt.
  */
void LPUART1_IRQHandler(void)
{
  /* USER CODE BEGIN LPUART1_IRQn 0 */

  /* USER CODE END LPUART1_IRQn 0 */
  HAL_UART_IRQHandler(&hlpuart1);
  /* USER CODE BEGIN LPUART1_IRQn 1 */

  /* USER CODE END LPUART1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel4 global interrupt.
  */
void DMA2_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel4_IRQn 0 */

  /* USER CODE END DMA2_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
  /* USER CODE BEGIN DMA2_Channel4_IRQn 1 */

  /* USER CODE END DMA2_Channel4_IRQn 1 */
}

/* USER CODE BEGIN 1 */
/**
 * @brief  This function handles External line
 *         interrupt request.
 * @param  None
 * @retval None
 */
void PUSH_BUTTON_SW1_EXTI_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(BUTTON_SW1_PIN);
}

/**
 * @brief  This function handles External line
 *         interrupt request.
 * @param  None
 * @retval None
 */
void PUSH_BUTTON_SW2_EXTI_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(BUTTON_SW2_PIN);
}

/**
 * @brief  This function handles External line
 *         interrupt request.
 * @param  None
 * @retval None
 */
void PUSH_BUTTON_SW3_EXTI_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(BUTTON_SW3_PIN);
}

void RTC_WKUP_IRQHandler(void)
{
  HW_TS_RTC_Wakeup_Handler();
}

void IPCC_C1_TX_IRQHandler(void)
{
  HW_IPCC_Tx_Handler();

  return;
}

void IPCC_C1_RX_IRQHandler(void)
{
  HW_IPCC_Rx_Handler();
  return;
}

/**
  * @brief  This function handles Flash interrupt request.
  * @param  None
  * @retval None
  */
void FLASH_IRQHandler(void)
{
#ifdef DUALCORE_FLASH_SHARING
  
  /* We enter a critical section */
  UTILS_ENTER_CRITICAL_SECTION();
  
  /*  Try to take the HW flash protection semaphore 7 until it is released */ 
  while(HAL_HSEM_Take(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1) != HAL_OK)
  {
    while( HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) ) ;
  }
#endif
    
  HAL_FLASH_IRQHandler();

#ifdef DUALCORE_FLASH_SHARING
  /* Release the HW Semaphore */  
  HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1); 
  /* We exit the critical section */
  UTILS_EXIT_CRITICAL_SECTION();
#endif
}

#ifdef DUALCORE_FLASH_SHARING
/**
  * @brief This function handles HSEM global interrupt.
  */
void HSEM_IRQHandler(void)
{
  /* USER CODE BEGIN HSEM_IRQn 0 */

  /* USER CODE END HSEM_IRQn 0 */
  HAL_HSEM_IRQHandler();
  /* USER CODE BEGIN HSEM_IRQn 1 */

  /* USER CODE END HSEM_IRQn 1 */
}
#endif
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
