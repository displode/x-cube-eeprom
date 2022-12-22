/**
  ******************************************************************************
  * @file    EEPROM_Emul/Porting/STM32WB/flash_interface.c
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation flash interface functions.
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
#include "eeprom_emul.h"
#include "flash_interface.h"
/* Displode Override: Library should not be hardcoded against nucleo board
#include "stm32wbxx_nucleo.h"
*/

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
#ifdef DUALCORE_FLASH_SHARING
#define HSEM_PROCESS_1 12U /* Number taken randomly to identify the process locking a semaphore in the driver context */
#endif
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Functions
  * @{
  */

#ifdef DUALCORE_FLASH_SHARING
/**
  * @brief  Write a double word at the given address in Flash
  * @param  Address Where to write
  * @param  Data What to write
  * @param  Write_type Type of writing on going (see EE_Write_type)
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE_WRITE_ERROR: if an error occurs
  *           - EE_FLASH_USED: flash currently used by CPU2
  */
EE_Status FI_WriteDoubleWord(uint32_t Address, uint64_t Data, EE_Write_type Write_type)
{
  EE_Status ee_status = EE_OK;
  
  /* We enter a critical section */
  UTILS_ENTER_CRITICAL_SECTION();
  
  /* When the ongoing writing operation is a direct one (no transfer is required,
      we are not in init process, and we do not write the state of a page) */
  if(Write_type == EE_SIMPLE_WRITE)
  {
    /* Wait for the semaphore to be free */
    while( HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) );
    
    /*  Take the HW 7 semaphore */    
    if(HAL_HSEM_Take(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1) == HAL_OK)
    {  
      if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, Data) != HAL_OK)
      {
        HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1);
        ee_status = EE_WRITE_ERROR;
      }    
      /* Release the HW Semaphore */  
      HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1);
      ee_status =  EE_OK;
    }
    else
    {    
      /* If flash is used by CPU2, the semaphore release interrupt is activated so as to raise a notification when
          the semaphore will be unlocked (user can do other operations while waiting) */
      HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID));
      ee_status =  EE_FLASH_USED;
    }
  }
  /* This is when the function call comes from a writing operation other than a direct one */
  else 
  {    
    /* Wait for the semaphore 7 to be free and take it when it is */
    while(HAL_HSEM_Take(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1) != HAL_OK)
    {
      while( HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) ) ;
    }

    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, Data) != HAL_OK)
    {
      HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1);
      ee_status = EE_WRITE_ERROR;
    }    
    /* Release the HW Semaphore */  
    HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1);
    ee_status =  EE_OK;
  }
  
  /* We exit the critical section */
  UTILS_EXIT_CRITICAL_SECTION();
  return ee_status;
}
#else
/**
  * @brief  Write a double word at the given address in Flash
  * @param  Address Where to write
  * @param  Data What to write
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE_WRITE_ERROR: if an error occurs
  */
HAL_StatusTypeDef FI_WriteDoubleWord(uint32_t Address, uint64_t Data)
{
  return HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, Address, Data); 
}
#endif

/**
  * @brief  Erase a page in polling mode
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_PageErase(uint32_t Page, uint16_t NbPages)
{
  EE_Status status = EE_OK;
  
#ifdef DUALCORE_FLASH_SHARING
    
  /* Wait for last operation to be completed */
  while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) ;
  
  /* Because we want to share flash between CPU1 and 2, we erase each page individually
  * and we take then release the associated semaphore for each page erasings.
  * By doing this, we allow CPU2 to do urgent works between page erasings. */
  for (uint32_t index = Page; index < (Page + NbPages); index++)
  {
    /* We enter a critical section */
    UTILS_ENTER_CRITICAL_SECTION();
    
    /* Wait for the semaphore 7 to be free and take it when it is */
    while(HAL_HSEM_Take(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1) != HAL_OK)
    {
      while( HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) ) ;
    }
   
    /* Start erase page */
    FLASH_PageErase(index);
    /* Release the HW Semaphore */  
    HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1); 
    /* We exit the critical section */
    UTILS_EXIT_CRITICAL_SECTION();
    
    /* Wait for last operation to be completed */
    while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) ;
  }
    
  /* If operation is completed or interrupted, disable the Page Erase Bit */
  CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));
  
  /* Flush the caches to be sure of the data consistency */
  /* Flush instruction cache  */
  if (READ_BIT(FLASH->ACR, FLASH_ACR_ICEN) == FLASH_ACR_ICEN)
  {
    /* Disable instruction cache  */
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    /* Reset instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_RESET();
    /* Enable instruction cache */
    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  }

  /* Flush data cache */
  if (READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) == FLASH_ACR_DCEN)
  {
    /* Disable data cache  */
    __HAL_FLASH_DATA_CACHE_DISABLE();
    /* Reset data cache */
    __HAL_FLASH_DATA_CACHE_RESET();
    /* Enable data cache */
    __HAL_FLASH_DATA_CACHE_ENABLE();
  }

#else
  FLASH_EraseInitTypeDef s_eraseinit;
  uint32_t page_error = 0U;

  s_eraseinit.TypeErase   = FLASH_TYPEERASE_PAGES;
  s_eraseinit.NbPages     = NbPages;
  s_eraseinit.Page        = Page;

  /* Erase the Page: Set Page status to ERASED status */
  if (HAL_FLASHEx_Erase(&s_eraseinit, &page_error) != HAL_OK)
  {
    status = EE_ERASE_ERROR;
  }
#endif
  return status;
}

/**
  * @brief  Erase a page with interrupt enabled
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_PageErase_IT(uint32_t Page, uint16_t NbPages)
{
  EE_Status status = EE_OK;
  FLASH_EraseInitTypeDef s_eraseinit;

  s_eraseinit.TypeErase   = FLASH_TYPEERASE_PAGES;
  s_eraseinit.NbPages     = NbPages;
  s_eraseinit.Page        = Page;

#ifdef DUALCORE_FLASH_SHARING
  /* We enter a critical section */
  UTILS_ENTER_CRITICAL_SECTION();
  
  /* Wait for the semaphore 7 to be free and take it when it is */
  while(HAL_HSEM_Take(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1) != HAL_OK)
  {
    while( HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) ) ;
  }
#endif    
  
  /* Erase the Page: Set Page status to ERASED status */
  if (HAL_FLASHEx_Erase_IT(&s_eraseinit) != HAL_OK)
  {
    status = EE_ERASE_ERROR;
  }
  
#ifdef DUALCORE_FLASH_SHARING
  /* Release the HW Semaphore */  
  HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1);
  /* We exit the critical section */
  UTILS_EXIT_CRITICAL_SECTION();
#endif

  return status;
}

/**
  * @brief  Flush the caches if needed to keep coherency when the flash content is modified
  */
void FI_CacheFlush()
{
  /* To keep its coherency, flush the D-Cache: its content is not updated after a flash erase. */
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_DATA_CACHE_ENABLE();
}

/**
  * @brief  Delete corrupted Flash address, can be called from NMI. No Timeout.
  * @param  Address Address of the FLASH Memory to delete
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_DeleteCorruptedFlashAddress(uint32_t Address)
{
  EE_Status status = EE_OK;

  /* Set FLASH Programmation bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

#ifdef DUALCORE_FLASH_SHARING  
  /* We enter a critical section */
  UTILS_ENTER_CRITICAL_SECTION();
  
  /* Wait for the semaphore 7 to be free and take it when it is */
  while(HAL_HSEM_Take(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1) != HAL_OK)
  {
    while( HAL_HSEM_IsSemTaken(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID) ) ;
  }
#endif
    
    /* Program double word of value 0 */
    *(__IO uint32_t*)(Address) = (uint32_t)0U;
    *(__IO uint32_t*)(Address+4U) = (uint32_t)0U;

#ifdef DUALCORE_FLASH_SHARING
  /* Release the HW Semaphore */  
  HAL_HSEM_Release(CFG_HW_BLOCK_FLASH_REQ_BY_CPU2_SEMID, HSEM_PROCESS_1); 
  /* We exit the critical section */
  UTILS_EXIT_CRITICAL_SECTION();
#endif

  /* Wait programmation completion */
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) ;

  /* Check if error occured */
  if((__HAL_FLASH_GET_FLAG(FLASH_FLAG_OPERR))  || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PROGERR)) ||
     (__HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR)) || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGAERR))  ||
     (__HAL_FLASH_GET_FLAG(FLASH_FLAG_SIZERR)) || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGSERR)))
  {
    status = EE_DELETE_ERROR;
  }

  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
  }

  /* Clear FLASH Programmation bit */
  CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

  /* Clear FLASH ECCD bit */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ECCD);

  return status;
}

/**
  * @brief  Check if the configuration is 128-bits bank or 2*64-bits bank
  * @param  None
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_CheckBankConfig(void)
{
#if defined (FLASH_OPTR_DBANK)
  FLASH_OBProgramInitTypeDef sOBCfg;
  EE_Status status;

  /* Request the Option Byte configuration :
     - User and RDP level are always returned
     - WRP and PCROP are not requested */
  sOBCfg.WRPArea     = 0xFF;
  sOBCfg.PCROPConfig = 0xFF;
  HAL_FLASHEx_OBGetConfig(&sOBCfg);

  /* Check the value of the DBANK user option byte */
  if ((sOBCfg.USERConfig & OB_DBANK_64_BITS) != 0)
  {
    status = EE_OK;
  }
  else
  {
    status = EE_INVALID_BANK_CFG;
  }

  return status;
#else
  /* No feature 128-bits single bank, so always 64-bits dual bank */
  return EE_OK;
#endif
}


/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
