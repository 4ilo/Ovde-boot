/*
 * oli_bootloader.h
 *
 *  Created on: 27-dec.-2016
 *      Author: Olivier
 */

#ifndef OLI_OLI_BOOTLOADER_H_
#define OLI_OLI_BOOTLOADER_H_

#include "oli/oli_discover.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_it.h"
#include "lwip.h"

void vObootloaderJumpToApplication( uint32_t ulAppAdress );

#endif /* OLI_OLI_BOOTLOADER_H_ */
