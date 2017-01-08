/*
 * oli_text.c
 *
 *  Created on: 16-nov.-2016
 *      Author: Olivier
 */

#include "oli_text.h"
#include "verdanaFont_data.h"

/* Begin Static function prototypes */

static void prvOtextWriteCharToBuffer( uint32_t ulFramebuffer, const char cKarakter );
static void prvOtextInitDma2d( DMA2D_HandleTypeDef * pxDma2d );
static void prvOtextInitLayer( LTDC_HandleTypeDef * pxLtdc, uint32_t ulAdress );

/* End Static function prototypes */

/* Begin static and extern variables */

uint32_t ulFrameBuffer;
static DMA2D_HandleTypeDef * pxHdma2dPointer;

/* End static and extern variables */

/*-----------------------------------------------------------*/

/* Begin pubic functions */

/**
 * @brief      Initialisatie van het lcd-scherm
 * 				De initialisatie kan enkel slagen als:
 * 				- De ltdc juist geconfigureerd is via cube(alle pinnen juist + lcdclk op 9,6Mhz
 *				- Dma 2d aangezet in cube
 *
 * @param      pxLtdc   De ltdc variable voor initialisatie
 * @param      pxDma2d  De dma variable voor initialisatie
 */
void vOtextInitLcd( LTDC_HandleTypeDef* pxLtdc, DMA2D_HandleTypeDef * pxDma2d, uint32_t ulFrameBufferAdress )
{
	// We slagen de framebuffer op
	ulFrameBuffer = ulFrameBufferAdress;

	// We maken de sd-rambuffer leeg
	vOtextClearBuffer( ulFrameBuffer );

	// We maken een layer op het scherm
	prvOtextInitLayer( pxLtdc, ulFrameBuffer );

	// We configureren de dma om het font te renderen
	prvOtextInitDma2d( pxDma2d );
}
/*-----------------------------------------------------------*/


/**
 * @brief      Schrijft een karakter naar een bepaald coordinaat op het scherm
 *
 * @param[in]  usXpos     Het x-coordinaat
 * @param[in]  usYpox     Het y-coordinaat
 * @param[in]  cKarakter  Het karakter
 */
void vOtextWriteChar( uint16_t usXpos, uint16_t usYpox, const char cKarakter )
{
uint16_t * pusScreenPointer;

	pusScreenPointer = ulFrameBuffer;
	pusScreenPointer = pusScreenPointer + ( usYpox * otextSCREENSIZE_X + usXpos);

	prvOtextWriteCharToBuffer( (uint32_t) pusScreenPointer, cKarakter );
}
/*-----------------------------------------------------------*/


/**
 * @brief      Schrijf een string naar het scherm op een bepaald coordinaat
 *
 * @param[in]  usXpos     Het x-coordinaat
 * @param[in]  usYpox     Het y-coordinaat
 * @param[in]  cString	  De string
 */
void vOtextWriteString( uint16_t usXpos, uint16_t usYpos, char cString[] )
{
uint16_t usStrlen;
uint16_t usXOffset = 0;

	usStrlen = strlen( cString );

	for( uint16_t teller = 0; teller < usStrlen; teller++ )
	{
		if( cString[ teller ] == '\n' )
		{
			// De user wilt een \n dus gaan we 10pixels naar beneden en het begin v/d lijn
			usYpos = usYpos + 10;
			usXOffset = 0;
		}
		else
		{
			vOtextWriteChar( usXpos + usXOffset * otextFONT_WIDTH, usYpos, cString[ teller ] );
			usXOffset++;
		}
	}
}
/*-----------------------------------------------------------*/

/**
 * @brief      Maak de buffer helemaal leeg, alle pixels op 0
 *
 * @param[in]  ulFramebufferAdress  Het adress van de framebuffer
 */
void vOtextClearBuffer( uint32_t ulFramebufferAdress )
{
uint16_t usX,usY;
uint16_t * pusFramePointer;

	pusFramePointer = ulFramebufferAdress;

	for( usY = 0; usY < otextSCREENSIZE_Y; usY++ )
	{
		for( usX = 0; usX < otextSCREENSIZE_X; usX++)
		{
			pusFramePointer[ usX + ( usY * otextSCREENSIZE_X ) ] = 0x0000;
		}
	}
}

/*-----------------------------------------------------------*/

/* End public functions */

/* Begin Static functions */


/**
 * @brief      Schrijf een char naar de screenbuffer
 *
 * @param[in]  ulFramebuffer  De framebuffer
 * @param[in]  cKarakter      Het te schrijven karakter
 */
static void prvOtextWriteCharToBuffer( uint32_t ulFramebuffer, const char cKarakter )
{
uint8_t * pucFont;
uint8_t ucCharRijInBitmap;
uint16_t usCharOffset;

	// We kijken of vorige transfer al afgelopen was
	HAL_DMA2D_PollForTransfer( pxHdma2dPointer, 10 );

	pucFont = (uint8_t *) &otextFONT;		// Het font
	usCharOffset = ( cKarakter - 32 ) * otextFONT_WIDTH;	 // De plaats waar het karakter staat
	ucCharRijInBitmap = usCharOffset / otextFONT_BITMAP_SIZE;

	if( ucCharRijInBitmap > 0 )
	{
		// We passen de ofset aan en negeren de rij
		usCharOffset = usCharOffset - ucCharRijInBitmap * otextFONT_BITMAP_SIZE;
		// De rij voegen we ineens toe aan de pointer
		// Een rij is 128px en 1 char is 12px hoog dus moeten we 12 rijen verder zijn
		pucFont = pucFont + ucCharRijInBitmap * ( otextFONT_BITMAP_SIZE * otextFONT_HEIGHT );
	}

	pucFont = pucFont + usCharOffset;

	// Laat de dma-transfer starten
	HAL_DMA2D_Start( pxHdma2dPointer, (uint32_t)pucFont, ulFramebuffer, otextFONT_WIDTH, otextFONT_HEIGHT );
}
/*-----------------------------------------------------------*/


/**
 * @brief      We initialiseren de dma2d om een tekst in een framebuffer te kunnen steken
 *
 * @param      pxDma2d  Een dma init typedef
 */
static void prvOtextInitDma2d( DMA2D_HandleTypeDef * pxDma2d )
{
	pxDma2d->Instance = DMA2D;
	pxDma2d->Init.Mode = DMA2D_M2M_PFC;
	pxDma2d->Init.ColorMode = DMA2D_OUTPUT_ARGB1555;		// Schermbuffer is in ARGB1555 formaat
	pxDma2d->Init.OutputOffset = otextSCREENSIZE_X - otextFONT_WIDTH;		// Het scherm is 480px breed en 1 char is 8px; 480-8=473

	// De font bitmap is 128px breed en 1char is 8 px breed
	pxDma2d->LayerCfg[1].InputOffset = otextFONT_BITMAP_SIZE - otextFONT_WIDTH;	// 128-8px
	pxDma2d->LayerCfg[1].InputColorMode = DMA2D_INPUT_A8;		// Het font is een A8-bitmap
	pxDma2d->LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	pxDma2d->LayerCfg[1].InputAlpha = 0xFFFFFFFF;		// De tekst moet wit worden

	HAL_DMA2D_Init( pxDma2d );
	HAL_DMA2D_ConfigLayer( pxDma2d, 1 );

	pxHdma2dPointer = pxDma2d;		// We slagen de dma config op
}
/*-----------------------------------------------------------*/


/**
 * @brief      We maken een layer op het scherm die wijst naar een framebuffer in ARGB1555
 *
 * @param      pxLtdc    Een ltdc init-typedef
 * @param[in]  ulAdress  Het startadress van de screenbuffer
 */
static void prvOtextInitLayer( LTDC_HandleTypeDef * pxLtdc, uint32_t ulAdress )
{
LTDC_LayerCfgTypeDef xLaag0;

	xLaag0.WindowX0 = 0;
	xLaag0.WindowX1 = 480;
	xLaag0.WindowY0 = 0;
	xLaag0.WindowY1 = 272;

	xLaag0.PixelFormat = LTDC_PIXEL_FORMAT_ARGB1555;
	xLaag0.FBStartAdress = ulAdress;
	xLaag0.ImageWidth = 480;
	xLaag0.ImageHeight = 272;

	xLaag0.Alpha = 255;
	xLaag0.Alpha0 = 0;
	xLaag0.Backcolor.Blue = 0;
	xLaag0.Backcolor.Green = 0;
	xLaag0.Backcolor.Red = 0;
	xLaag0.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	xLaag0.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;

	HAL_LTDC_ConfigLayer( pxLtdc, &xLaag0, 0 );

	HAL_GPIO_WritePin( LCD_DISP_GPIO_Port, LCD_DISP_Pin, 1 );
	HAL_GPIO_WritePin( LCD_BL_GPIO_Port, LCD_BL_Pin, 1 );
}
/*-----------------------------------------------------------*/

/* End Static functions */
