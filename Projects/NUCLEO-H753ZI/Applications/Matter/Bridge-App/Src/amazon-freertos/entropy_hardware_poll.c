/**
  ******************************************************************************
  * @file    entropy_hardware_poll.c
  * @author  MCD Application Team
  * @brief   This file provides code for the entropy collector.
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

#include <string.h>
#include "main.h"
#include "stm32h7xx_hal.h"

int mbedtls_hardware_poll( void *data, unsigned char *output, size_t len, size_t *olen );



int mbedtls_hardware_poll( void *data,
                    unsigned char *output, size_t len, size_t *olen )
{
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t random_number = 0;

  status = HAL_RNG_GenerateRandomNumber(&hrng, &random_number);
  ((void) data);
  *olen = 0;

  if ((len < sizeof(uint32_t)) || (HAL_OK != status))
  {
    return 0;
  }

  memcpy(output, &random_number, sizeof(uint32_t));
  *olen = sizeof(uint32_t);

  return 0;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
