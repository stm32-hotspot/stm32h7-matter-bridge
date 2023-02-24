/**
  ******************************************************************************
  * @file    flash_h7.c
  * @author  MCD Application Team
  * @brief   Management of the H7 internal flash memory.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "flash.h"
#include "stm32h7xx_hal.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "FreeRTOS.h"

//#include "STM32Config.h" // for define related to Matter key management
#define SECTOR_ID_FOR_MATTER_KEY 15 // the flash sector dedicated to the Key management
#define MATTER_KEY_NAME_MAX_LENGTH (30 * 2) // Max key name string size is 30 "keyType...;KeyName..."
								 // ^ STM32STORE_MAX_KEY_SIZE

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
#define ROUND_DOWN(a,b) (((a) / (b)) * (b))
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#define MAX(a,b)        (((a) > (b)) ? (a) : (b))

/* Private variables ----------------------------------------------------------*/

/* STM32H743ZI / STM32H755ZI: 2 banks * 8 sectors * 128 Kbytes (4 K Flash memory words by sector) */
const uint32_t l_sector_map[] = {
  0x08000000, 0x08020000, 0x08040000, 0x08060000, 0x08080000, 0x080A0000, 0x080C0000, 0x080E0000,
  0x08100000, 0x08120000, 0x08140000, 0x08160000, 0x08180000, 0x081A0000, 0x081C0000, 0x081E0000,
  0x08200000 /* End of range */
};

/* Private function prototypes -----------------------------------------------*/
static int32_t GetSector(uint32_t Address);
static void FLASH_clean(void);
uint32_t FLASH_get_bank(uint32_t addr);

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Erase FLASH memory sector(s) at address. They must belong to the same Flash bank.
  * @param  In: address     Start address to erase from.
  * @param  In: len_bytes   Length to be erased.
  * @retval  0:  Success.
            -1:  Failure.
  */
int FLASH_Erase_Size(uint32_t address, uint32_t len_bytes)
{
  int rc = -1;
  uint32_t SectorError = 0;
  FLASH_EraseInitTypeDef EraseInit;

  /* WARN: ABW. If the passed address and size are not aligned on the sectors geometry,
   * the start of the first sector and the end of the last sector are erased anyway.
   * After erase, the flash is left in unlocked state.
   */
  EraseInit.TypeErase     = FLASH_TYPEERASE_SECTORS;
  EraseInit.Banks         = FLASH_get_bank(address);
  EraseInit.Sector        = GetSector(address);
  EraseInit.NbSectors     = GetSector(address + len_bytes - 1) - EraseInit.Sector + 1;
  EraseInit.VoltageRange  = FLASH_VOLTAGE_RANGE_4;

  if (HAL_FLASH_Unlock() != HAL_OK)
  {
   /* msg_info("Flash was already unlocked!\n"); */
  }

  if (HAL_FLASHEx_Erase(&EraseInit, &SectorError) == HAL_OK)
  {
    rc = 0;
  }
  else
  {
    /* msg_info("Error %lu erasing at 0x%08lx\n", SectorError, address); */
  }

  return rc;
}

/**
  * @brief  Write to FLASH memory.  Restriction: The same flash word (32 bytes) may only be written once.
  * @param  In: address     Destination address.
  * @param  In: pData       Data to be programmed.
  * @param  In: len_bytes   Number of bytes to be programmed.
  * @retval  0: Success.
            -1: Failure.
  */
int FLASH_Write(uint32_t address, uint32_t *pData, uint32_t len_bytes)
{
  int i;
  int ret = 0;

  if ( ( address % (FLASH_NB_32BITWORD_IN_FLASHWORD * sizeof(uint32_t)) != 0 )
    || ( len_bytes % (FLASH_NB_32BITWORD_IN_FLASHWORD * sizeof(uint32_t)) != 0 ) )
  {
    /* The destination address range is not aligned on flash word boundaries.
       This can result in ECC errors if the same flash word gets written again before the Flash sector is erased.
     */
    //vLoggingPrintf("Warning: FLASH_Write(). Should avoid writing to addresses not aligned on Flash words boundaries.\n");
  }

  for (i = 0; i < len_bytes; i += FLASH_NB_32BITWORD_IN_FLASHWORD * sizeof(uint32_t))
  {
    if ( HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
        address + i,
        ((uint32_t) pData) + i ) != HAL_OK)
    {
      ret = -1;
      break;
    }
  }

  /* Memory check */
  if (ret == 0)
  {
    for (i = 0; i < (len_bytes / sizeof(uint32_t)); i++ )
    {
      uint32_t *dst = ((uint32_t *) address) + i;
      uint32_t *src = ((uint32_t *) pData) + i;

      if ( *dst != *src )
      {
        ret = -1;
        break;
      }
    }
  }

  FLASH_clean();

  return ret;
}


/**
  * @brief  Get the bank of a given address.
  * @param  In: addr      Address in the FLASH Memory.
  * @retval Bank identifier.
  *           FLASH_BANK_1  - Also when running in single-bank mode.
  *           FLASH_BANK_2
  */
uint32_t FLASH_get_bank(uint32_t addr)
{
  return (addr < (FLASH_BASE + FLASH_BANK_SIZE)) ? FLASH_BANK_1 : FLASH_BANK_2;
}


/**
  * @brief  Update a chunk of the FLASH memory.
  * @note   The FLASH chunk must no cross a FLASH bank boundary.
  * @note   The source and destination buffers have no specific alignment constraints.
  * @param  In: dst_addr    Destination address in the FLASH memory.
  * @param  In: data        Source address.
  * @param  In: size        Number of bytes to update.
  * @retval  0:  Success.
  *         <0:  Failure (at malloc, erase, or write).
  */
int FLASH_update(uint32_t dst_addr, const void *data, uint32_t size)
{
  int rc = -1;
  int ret = 0;
  int remaining = size;
  uint8_t * src_addr = (uint8_t *) data;
  uint32_t * sector_cache = NULL;
  uint8_t * sector_cache_buffer = NULL;

  /* Allocate and word-align. The HAL implementation does not require that the source buffer is aligned on flashword boundaries. */
  sector_cache_buffer = pvPortMalloc (FLASH_SECTOR_SIZE + sizeof(uint32_t));	//heap freertos
  //sector_cache_buffer = malloc(FLASH_SECTOR_SIZE + sizeof(uint32_t));			//Notes : malloc in Heap instead of pvPortMalloc in FreeRTOS ucHeap ..


  if (sector_cache_buffer != NULL)
  {
    sector_cache = ( (uint32_t *) ( (uint32_t)sector_cache_buffer & ~(sizeof(uint32_t) - 1)) )+ 1;

    do {
      uint32_t sector = GetSector(dst_addr);
      uint32_t sector_size = l_sector_map[sector + 1] - l_sector_map[sector];
      uint32_t fl_addr = l_sector_map[sector];
      int fl_offset = dst_addr - fl_addr;
      int len = MIN(sector_size - fl_offset, remaining);

      #ifdef __CC_ARM
      /* Clear potential residual errors from the flash loader (MDK-ARM) or from ECC. */
      FLASH_clean();
      #endif

      /* Load from the flash into the cache */
      memcpy(sector_cache, (void *) fl_addr, sector_size);
      /* Update the cache from the source */
      memcpy((uint8_t *)sector_cache + fl_offset, src_addr, len);
      /* Erase the sector, and write the cache */
      ret = FLASH_Erase_Size(fl_addr, sector_size);
      if (ret != 0)
      {
        /* msg_error("Could not erase at 0x%08lx\n", fl_addr); */
      }
      else
      {
        ret = FLASH_Write(fl_addr, sector_cache, sector_size);
        if((ret != 0) && (memcmp((void*)fl_addr, sector_cache, sector_size)))
        {
          /* msg_error("Could not write %lu bytes at 0x%08lx (err %d)\n", sector_size, fl_addr, ret); */
        }
        else
        {
          dst_addr += len;
          src_addr += len;
          remaining -= len;
        }
      }
    } while ((ret == 0) && (remaining > 0));
    if (ret == 0)
    {
      rc = 0;
    }

    vPortFree(sector_cache_buffer);
  }
  else
  {
    /* msg_error("Could not allocate %d bytes for Flash update.\n", copy_buffer_size + sizeof(uint32_t)); */
  }

  FLASH_clean();

  return rc;
}

/**
  * @brief  Clean the flash controller errors.
  */
static void FLASH_clean(void)
{
  __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_ALL_BANK1 );
  __HAL_FLASH_CLEAR_FLAG( FLASH_FLAG_ALL_BANK2 );
}

/**
  * @brief  Gets the sector number of a given address.
  * @param  In: address
  * @retval >=0 Sector number.
  *         -1  Error: Invalid address.
  */
static int32_t GetSector(uint32_t address)
{
  int32_t sector = -1;

  if ( (l_sector_map[0] <= address) && (address < l_sector_map[sizeof(l_sector_map)/sizeof(uint32_t) - 1]) )
  { /* The address is within the range of the internal flash. */
    for (int i = 0; i < (sizeof(l_sector_map) / sizeof(uint32_t) - 1); i++)
    {
      if (address < l_sector_map[i+1])
      { /* Matching sector found. */
        sector = i;
        break;
      }
    }
  }

  return sector;
}


/************************ //Dedicated to Matter Key Management ****************************************************/

//Architecture of keys in the flash from sector address at "SECTOR_ID_FOR_MATTER_KEY" dedicated to store all Matter keys
//uint32_t : The number of keys stored in flash (GetNbKey function to get it)
//uint32_t : The total size of keys (including this uint32_t) stored in flash (GetTotalKeySize function to get it)
//String of MATTER_KEY_NAME_MAX_LENGTH char: The Name of the key
//uint32_t : The size of this key = KeySize (GetKeySize function to get it from start address of the name of the key)
//KeySize number of char to store the KeyValue (GetKeyValue to get it from start address of the name of the key OR the name of the key)


/**
  * @brief  Get the nb of key saved in flah
  * @param  In: Sector address. First uint32_t is the nb of key / Second uint32_t is the total size
  * @retval Nb of key
  */
uint32_t GetNbKey(uint32_t fl_addr)
{
	uint32_t NbKey = 0;
	// Get nb of keys in flash
	memcpy((char*)&NbKey, (char*)fl_addr, sizeof(uint32_t));
	//Test if flash erased
	if(NbKey == 0xFFFFFFFF)
		NbKey = 0;
	return(NbKey);
}

/**
  * @brief  Get the size of all keys saved in flash
  * @param  In: Sector address. First uint32_t is the nb of key / Second uint32_t is the total size
  * @retval Total key size
  */
uint32_t GetTotalKeySize(uint32_t fl_addr)
{
	uint32_t KeySize = 0;
	// Get nb of keys in flash
	memcpy((char*)&KeySize, (char*)(fl_addr + sizeof(uint32_t)), sizeof(uint32_t));
	if(KeySize == 0xFFFFFFFF || KeySize == 0)
		KeySize = 8; //Nb and size only. No key
	return(KeySize);
}

/**
  * @brief  Get the size of one key
  * @param  In: Address of the Key
  * @retval Key size
  */
uint32_t GetKeySize(void* fl_addr)
{
	uint32_t KeySize = 0;
	if (fl_addr != NULL)
	{
		// Get the key size
		memcpy((char*)&KeySize, (char*)(fl_addr + MATTER_KEY_NAME_MAX_LENGTH), sizeof(uint32_t));
	}
	if(KeySize == 0xFFFFFFFF)
		KeySize = 0;
	return(KeySize);

}

/**
  * @brief  Search for a key - Recursive function
  * @param  In: Name of the key to found / address of the current key / ID of key to test => 1 is the last one
  * @retval NULL : Key not found
  *         address of the key found
  */
void* SearchKey(const char* KeyName, const char* AddKey, uint32_t KeyID)
{
	// Size of the key
	uint32_t KeySize = 0;
	// pointer to the next key to test
	char* PtNextKey = NULL;
	// Next key = decrease keyID
	uint32_t NextKeyID = KeyID - 1;
	// Name of the current Key tested
	char CurrentKeyName[MATTER_KEY_NAME_MAX_LENGTH];
	

	if(KeyName == NULL || AddKey == NULL || KeyID == 0)
		return (NULL);
	// Get current key name
	memcpy((char*)CurrentKeyName, (char*)AddKey, MATTER_KEY_NAME_MAX_LENGTH);
	if(strcmp(KeyName, CurrentKeyName) != 0) // KeyName and AddKey not equal
	{
		//Test if last key to test
		if(KeyID <= 1)
		{
			return (NULL);
		}
		// Get size of the key
		KeySize = GetKeySize(AddKey);
		// Jump to the next key to test
		PtNextKey = AddKey + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t) + KeySize; //Current key address + name of the key + sizeof(uint32_t) for the lenght of the key value + lenght of the key
		return(SearchKey(KeyName, PtNextKey, NextKeyID));		
	} else {
		return(AddKey);
	}	
}


/**
  * @brief  Delete the Key knowing key Name
  * @param  In: Name of the key to delete
  *         Out: KeyValue This pointer must have been created (pvPortMalloc) by the caller function and will contain copied value
  * @retval KeyName if deleted, NULL if not (not found, no keys ...)
  *
  */
void* DeleteKey(const char* KeyName)
{
	void * ptKeyFound = NULL;
	void * ptNextKey = NULL;
	void * ptEnd = NULL;
	// Size of the key to delete
	uint32_t KeySize = 0;
	// Length of the Key to delete
	uint32_t KeyLength = 0;
	//Total size already in Flash
	uint32_t TotalKeyLength = GetTotalKeySize(l_sector_map[SECTOR_ID_FOR_MATTER_KEY]);
	//Nb of key
	uint32_t NbKey = 0;
	// Buffer for key recipient updated
	uint8_t * PtKey_buffer = NULL;
	//Size of sector to copy
	uint32_t FirstSectorSize = 0;
	uint32_t SecondSectorSize = 0;
	//ResultFlashUpdate
	int ResultFlashUpdate = 0;


	if(KeyName != NULL)
	{
		NbKey = GetNbKey(l_sector_map[SECTOR_ID_FOR_MATTER_KEY]);
		if(NbKey)
		{
			ptKeyFound = SearchKey(KeyName, (const char*) (l_sector_map[SECTOR_ID_FOR_MATTER_KEY] + 2*sizeof(uint32_t)), NbKey);
			if(ptKeyFound != NULL)
			{
				//Delete the Key from the array
				KeySize = GetKeySize(ptKeyFound);
				//Length of the Key to delete
				KeyLength = KeySize + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t);
				//Update total key length if possible
				if (TotalKeyLength >= KeyLength) {
					TotalKeyLength = TotalKeyLength - KeyLength;
					// Buffer of keys without key to remove
					PtKey_buffer = pvPortMalloc (TotalKeyLength);	// heap freertos
					if(PtKey_buffer != NULL) {
						ptNextKey = ptKeyFound + KeyLength;
						ptEnd = l_sector_map[SECTOR_ID_FOR_MATTER_KEY] + TotalKeyLength + KeyLength; //ptEnd is the last address of the arry WITH the key to delete
						FirstSectorSize = ptKeyFound - l_sector_map[SECTOR_ID_FOR_MATTER_KEY];
						SecondSectorSize = ptEnd - ptNextKey;
						//Copy the array of key without the key to delete
						memcpy((char*)PtKey_buffer, (char*)l_sector_map[SECTOR_ID_FOR_MATTER_KEY], FirstSectorSize);
						memcpy((char*)(PtKey_buffer + FirstSectorSize), (char*)ptNextKey, SecondSectorSize);
						//Update the flash
						ResultFlashUpdate = FLASH_update(l_sector_map[SECTOR_ID_FOR_MATTER_KEY], PtKey_buffer, TotalKeyLength);
						//Free memory
						vPortFree(PtKey_buffer);	//heap freeRTOS
						return (l_sector_map[SECTOR_ID_FOR_MATTER_KEY]);
					} else {
						//Not enought heap ram
						return (NULL);
					}

				} else {
					//Error the total key lenght can not be less than the lenght of the key found
					return (NULL);
				}
			} else {
				//Key not found
				return (NULL);
			}
		} else {
			//No key registered
			return (NULL);
		}
	} else {
		//Key not found - No name given
		return (NULL);
	}
}


/**
  * @brief  Get the Key value from the key Name or the Key addr
  * @param  In: Name of the key to found / address of the key / Size in byte of the key
  *         Out: KeyValue This pointer must have been created (pvPortMalloc) by the caller function and will contain copied value 
  * @retval None
  *         
  */
void* GetKeyValue(void* KeyValue, const char* KeyName, void* KeyAddr, uint32_t KeySize, size_t * read_bytes_size)
{
	
	void * ptKeyFound = NULL;
	uint32_t NbKey = 0;
	uint32_t KeyLength = KeySize;

	
	if(KeyAddr == NULL)		
	{
		if(KeyName == NULL)
		{
			return(NULL);
		} else {
			NbKey = GetNbKey(l_sector_map[SECTOR_ID_FOR_MATTER_KEY]);
			if(NbKey)
			{
				ptKeyFound = SearchKey(KeyName, (const char*) (l_sector_map[SECTOR_ID_FOR_MATTER_KEY] + 2*sizeof(uint32_t)), NbKey);
				if(ptKeyFound != NULL)
				{
					if (KeyLength <= 0)
					{
						//Get Key length
						KeyLength = GetKeySize(ptKeyFound);
					}

					KeyLength = GetKeySize(ptKeyFound);
					*read_bytes_size = KeyLength;

					if (KeyLength > 0)
					{	
						if(KeyValue == NULL) //Test key exist
						{
							//Return Key found
							return(ptKeyFound);
						}
						// Get value of the key
						memcpy((char*)KeyValue, (char*)(ptKeyFound + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t)), KeyLength);
						return(KeyValue);
					} else {
						return(NULL);
					}
				} else {
					return(NULL);
				}
			} else {
				return(NULL);
			}
		}
	} else {
		if (KeyLength <= 0)
		{	
			//Get Key length
			KeyLength = GetKeySize(KeyAddr);
		} 
		if (KeyLength > 0)
		{	
			if(KeyValue == NULL) //Test key exist
			{
				//Return Key found
				return(KeyAddr);
			}
			// Get value of the key
			memcpy((char*)KeyValue, (char*)(KeyAddr + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t)), KeyLength);
			return(KeyValue);
		} else {
			return(NULL);
		}
	}
	
}


/**
  * @brief  Save the Key value from the key Name in flash
  * @param  In: Name of the key to save/ Key value / Size in byte of the key value
  *         
  * @retval KeyAddr = adress of the key in flash or NULL if not saved
  *         
  */
void* SetKeyValue(void* KeyValue, const char* KeyName, uint32_t KeySize)
{
	// pointer to the key found 
	char* PtKey = NULL;
	// Buffer to update or add the key
	uint8_t * PtKey_buffer = NULL;
	// Size of the Key to add
	uint32_t KeyLength = KeySize + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t);
	//Total size already in Flash
	uint32_t TotalKeyLength = 0;
	//Nb of key
	uint32_t NbKey = 0;
	//ResultFlashUpdate
	int ResultFlashUpdate = 0;

    //Pt to nb key
	uint32_t* PtNbKey = NULL;
	//Pt to total size
	uint32_t* PtTotalKeyLength = NULL;


	TotalKeyLength = GetTotalKeySize(l_sector_map[SECTOR_ID_FOR_MATTER_KEY]);

	PtKey_buffer = pvPortMalloc (TotalKeyLength + KeyLength );	// heap freertos
	//PtKey_buffer = malloc (TotalKeyLength + KeyLength );					//Notes: malloc in Heap instead of pvPortMalloc in FreeRTOS ucHeap ..
	
	if(PtKey_buffer != NULL)
	{
		memcpy((char*)PtKey_buffer, (char*)l_sector_map[SECTOR_ID_FOR_MATTER_KEY], TotalKeyLength);
		//Does Key already exist ?
		PtNbKey = (uint32_t*)PtKey_buffer;
		NbKey = *PtNbKey;
		if(NbKey == 0xFFFFFFFF)
		{
			//Flash erased
			NbKey = 0;
			*PtNbKey = 0;
		}
		PtKey =  SearchKey(KeyName, (const char*) (PtKey_buffer + 2*sizeof(uint32_t)), NbKey);
		if (PtKey == NULL) {
			//Add KEY
			//Name
			memcpy((char*)(PtKey_buffer + TotalKeyLength), (char*)KeyName, MATTER_KEY_NAME_MAX_LENGTH);
			//Size
			memcpy((char*)(PtKey_buffer + TotalKeyLength + MATTER_KEY_NAME_MAX_LENGTH), (char*)&KeySize, sizeof(uint32_t));
			//Value
			memcpy((char*)(PtKey_buffer + TotalKeyLength + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t)), (char*)KeyValue, KeySize);
			//Update total size
			PtTotalKeyLength = (uint32_t*)(PtKey_buffer + sizeof(uint32_t));
			if(*PtTotalKeyLength == 0xFFFFFFFF || *PtTotalKeyLength == 0)
			{
				//Flash erased
				*PtTotalKeyLength  = 8;
			}
			*PtTotalKeyLength += KeyLength;
			//Update NB key
			PtNbKey = (uint32_t*)PtKey_buffer;
			*PtNbKey += 1;
			//Save result
			ResultFlashUpdate = FLASH_update(l_sector_map[SECTOR_ID_FOR_MATTER_KEY], PtKey_buffer, *PtTotalKeyLength);
		} else {
			//Update KEY
			memcpy((char*)(PtKey + MATTER_KEY_NAME_MAX_LENGTH + sizeof(uint32_t)), (char*)KeyValue, KeySize);
			ResultFlashUpdate = FLASH_update(l_sector_map[SECTOR_ID_FOR_MATTER_KEY], PtKey_buffer, TotalKeyLength);
		}

		//Free memory
		vPortFree(PtKey_buffer);	//heap freeRTOS
		//free(PtKey_buffer);			//heap RAM_D2							//Notes: free in Heap instead of pvPortMalloc in FreeRTOS ucHeap ..

		if(ResultFlashUpdate == 0)
			return(l_sector_map[SECTOR_ID_FOR_MATTER_KEY]);
		else
			return(NULL);
	} else {
		return(NULL);
	}
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

