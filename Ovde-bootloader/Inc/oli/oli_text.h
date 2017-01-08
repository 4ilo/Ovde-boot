/*
 * oli_text.h
 *
 *  Created on: 16-nov.-2016
 *      Author: Olivier
 */

#ifndef OLI_TEXT_H_
#define OLI_TEXT_H_

#include "stm32f7xx_hal.h"
#include "string.h"

/* Het font dat gebruikt gaat worden */
#define otextFONT VERDANAFONT_DATA

//	Extra info over het gebruikte font
#define otextFONT_HEIGHT 12
#define otextFONT_WIDTH 8
#define otextFONT_BITMAP_SIZE 128

//	Extra info over het scherm waarop de data komt
#define otextSCREENSIZE_X 480
#define otextSCREENSIZE_Y 272

// De functies die te gebruiken zijn
void vOtextInitLcd( LTDC_HandleTypeDef* pxLtdc, DMA2D_HandleTypeDef * pxDma2d, uint32_t ulFrameBufferAdress );
void vOtextWriteChar( uint16_t usXpos, uint16_t usYpox, const char cKarakter );
void vOtextWriteString( uint16_t usXpos, uint16_t usYpos, char cString[] );
void vOtextClearBuffer( uint32_t ulFramebufferAdress );


#endif /* OLI_TEXT_H_ */
