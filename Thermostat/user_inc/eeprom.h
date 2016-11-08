/**
 *******************************************************************************
 * @file    eeprom.h
 * @author  JASoN
 * @version V1.0
 * @date    2014-03-20
 * @since   2014-03-20
 * @ref     STM32F10x_AN2594_FW_V3.1.0 - MCD Application Team
 * @brief   This file contains all the functions prototypes for the EEPROM
 *          emulation firmware library.
 *******************************************************************************
 */

#ifndef __EEPROM_H
#define __EEPROM_H

#include "stm32f10x.h"

/**
 * STM32F10Xxx flash page size
 */
#if defined (STM32F10X_LD) || defined (STM32F10X_MD)
  #define PAGE_SIZE  (uint16_t)0x400                      // page size = 1KByte
#elif defined (STM32F10X_HD) || defined (STM32F10X_CL)
  #define PAGE_SIZE  (uint16_t)0x800                      // page size = 2KByte
#endif

/**
 * Used flash pages for EEPROM emulation
 */
#define PAGE_0                  ((uint16_t)0x00)
#define PAGE_1                  ((uint16_t)0x01)

/**
 * EEPROM start address in flash
 * As for STM32F103VE (512KB flash), 0x0807F000 is the begining of the last 2 pages.
 * Check the *.map file (in listing folder) to find out the used ROM size.
 */
#define EEPROM_START_ADDRESS    ((uint32_t)0x0807F000)

/**
 * Page base and end addresses
 */
#define PAGE_0_BASE_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS))
#define PAGE_0_END_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE - 1))

#define PAGE_1_BASE_ADDRESS     ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE))
#define PAGE_1_END_ADDRESS      ((uint32_t)(EEPROM_START_ADDRESS + PAGE_SIZE * 2 - 1))

/**
 * Page status
 */
#define ERASED                  ((uint16_t)0xFFFF) // page is empty
#define RECEIVE_DATA            ((uint16_t)0xEEEE) // page is marked to receive data
#define VALID_PAGE              ((uint16_t)0x0000) // page containing valid data

/**
 * Block status
 */
#define VALID_BLOCK             ((uint16_t)0xAAAA)
#define EXPIRED_BLOCK           ((uint16_t)0x0000)

/**
 * Data flag
 */
#define DATA_FLAG               ((uint16_t)0xCCCC)

/**
 * Page operation
 */
#define READ_FROM_VALID_PAGE    ((uint8_t)0x00)
#define WRITE_IN_VALID_PAGE     ((uint8_t)0x01)

/**
 * Error defines
 */
#define EE_NO_VALID_PAGE        ((uint16_t)0x00AB) // no valid page
#define EE_PAGE_FULL            ((uint16_t)0x0080) // page full
#define EE_DATA_SIZE_ERROR      ((uint16_t)0x0081) // data size error
#define EE_DATA_CHECKSUM_ERROR  ((uint16_t)0x0082) // data checksum error


/**
 * Exported functions
 */
uint16_t EE_Init(void);
uint16_t EE_ReadData(uint8_t key, uint8_t *pdata, uint16_t size);
uint16_t EE_WriteData(uint8_t key, const uint8_t *pdata, uint16_t size);

/**
 * Extensions
 */
uint16_t memcpy_to_eeprom_with_checksum(uint8_t key, const void *pdata, uint16_t size);
uint16_t memcpy_from_eeprom_with_checksum(void *pdata, uint8_t key, uint16_t size);

#endif /* __EEPROM_H */

