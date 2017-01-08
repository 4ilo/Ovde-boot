/*
 * oli_tcp.c
 *
 *  Created on: 21-nov.-2016
 *      Author: Olivier
 */

#include "oli/oli_tcp.h"

/*-----------------------------------------------------------*/

/* Begin Static function prototypes */

// De callback functies die opgeroepen worden bij menu commando's
static void prvOvdeReturnFirmwareVersion( struct tcp_pcb * pxTpcb );
static void prvOvdeReturnBootloaderVersion( struct tcp_pcb * pxTpcb );
static void prvOvdeSaveFile( struct tcp_pcb * pxTpcb );
static void prvOvdeSetFlashProgramName( struct tcp_pcb * pxTpcb );
static void prvOvdeReturnFlashData( struct tcp_pcb * pxTpcb );
static void prvOvdeResetThisDevice( struct tcp_pcb * pxTpcb );

/* End Static function prototypes */

/* Begin menu items */

commando xSetFileName = { 'n', NULL, &prvOvdeSetFlashProgramName};
commando xResetDevice = { 'r', &xSetFileName, &prvOvdeResetThisDevice };
commando xVerifyFile = { 'v', &xResetDevice, &prvOvdeReturnFlashData };
commando xUploadFile = { 'u', &xVerifyFile, &prvOvdeSaveFile };
commando xFirmwareVersie = { 'f', &xUploadFile, &prvOvdeReturnFirmwareVersion };
commando xBootloaderVersie = { 'b', &xFirmwareVersie, &prvOvdeReturnBootloaderVersion };

/* End menu items */

/* Begin static global variables */

static uint64_t ullFileSize = 0;

/* End static global variables */

/* Begin extern variables */

/* End extern variables */

/*-----------------------------------------------------------*/

/* Begin public callback functions */

/**
 * @brief      Hier komen we als het upload programma commando's kan sturen
 *
 * @param      pvArg   Extra argumenten van lwip
 * @param      pxTpcb  De tcp-connectie
 * @param      pxBuf   De buffer met ontvangen data
 * @param[in]  xErr    De errors van lwip
 *
 * @return     return aan lwip of er een error was
 */
err_t xOtcpReadMenuCommando( void * pvArg, struct tcp_pcb * pxTpcb, struct pbuf * pxBuf, err_t xErr )
{

commando * pxTcpCommando = &xBootloaderVersie;		// De eerste node in de gelinkte lijst
uint8_t ucGevonden = 0;

	if( pxBuf == NULL )
	{
		tcp_close( pxTpcb );
	}

	if( pxBuf->len == 1 )
	{
		// We hebben nu een commando gekregen
		tcp_recved( pxTpcb, pxBuf->len );

		// We lopen de gelinkte lijst af en kijken of we een match hebben voor het commando
		while( ( pxTcpCommando != NULL ) && ( ucGevonden == 0 ) )
		{
			if( *( (char*) pxBuf->payload ) == pxTcpCommando->cAfkorting )
			{
				pxTcpCommando->functie( pxTpcb );
				ucGevonden = 1;
			}
			else
			{
				pxTcpCommando = pxTcpCommando->pxNext;
			}
		}
	}

	pbuf_free( pxBuf );
	return ERR_OK;
}
/*-----------------------------------------------------------*/


/**
 * @brief      Alle data die nu krijgen moet opgeslagen worden
 *
 * @param      pvArg   Extra argumenten van lwip
 * @param      pxTpcb  De tcp-connectie
 * @param      pxBuf   De buffer met ontvangen data
 * @param[in]  xErr    De errors van lwip
 *
 * @return     return aan lwip of er een error was
 */
err_t xOtcpSaveFileToFlash( void * pvArg, struct tcp_pcb * pxTpcb, struct pbuf * pxBuf, err_t xErr)
{

static uint16_t usBlockCounter = 0;
static uint64_t ullDataSavedToFlash = 0;

char cTemp[10];
uint8_t ucTeller;

FLASH_EraseInitTypeDef xEraseInitStruct;
uint32_t ulSectorError;

uint32_t ulAdress;

	if( pxBuf == NULL )
	{
		tcp_close( pxTpcb );
	}


	if( ullFileSize == 0 )
	{
		tcp_recved( pxTpcb, pxBuf->len );		// We sturen een ack
		// Eerst krijgen we de grootte van de file door, deze slagen we op
		for( ucTeller = 0; ucTeller < pxBuf->len; ucTeller++ )
		{
			cTemp[ ucTeller ] = ((char*)pxBuf->payload)[ ucTeller ];
		}
		cTemp[ ucTeller + 1 ] = '\0';	// Nog een nulbyte
		ullFileSize = atoi( cTemp );

		// We maken de flash klaar om in te schrijven
		xEraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		xEraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		xEraseInitStruct.Sector = FLASH_SECTOR_4;
		xEraseInitStruct.NbSectors = 4;

		// We maken de flash sector leeg
		HAL_FLASH_Unlock();
		if( HAL_FLASHEx_Erase( &xEraseInitStruct, &ulSectorError ) == HAL_OK )
		{
			// Het is gelukt! we sturen ok en gaan door
			strcpy( cTemp, "ok2" );
			tcp_write( pxTpcb, &cTemp, 3, 1);
		}
		HAL_FLASH_Lock();
		pbuf_free( pxBuf );
		return ERR_OK;
	}

	// Als we hier komen is er data ontvangen om in de flash te steken
	tcp_recved( pxTpcb, pxBuf->len );		// We sturen een ack

	HAL_FLASH_Unlock();
	ulAdress = otcpAPPLICATION_ADDRESS;	// Een var met het te schrijven addr

	// We programmeren de flash byte per byte
	for( uint16_t usTeller = 0; usTeller < pxBuf->len; usTeller++ )
	{
		ulAdress = (uint32_t)( otcpAPPLICATION_ADDRESS + ( usBlockCounter * 512 ) + usTeller );
		HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, ulAdress, ( (uint8_t*) pxBuf->payload )[ usTeller ] );
	}
	usBlockCounter++;		// We hebben weer een block van 512bytes geschreven
	ullDataSavedToFlash = ullDataSavedToFlash + pxBuf->len;		// De hoeveelheid die we effectief geschreven hebben opslaan
	HAL_FLASH_Lock();

	// We maken de buffer leeg want we hebben alles opgeslagen
	pbuf_free( pxBuf );

	if(ullDataSavedToFlash == ullFileSize)
	{
		// We hebben nu alle data ontvangen en gaan terug over naar wachten op commando's
		tcp_recv( pxTpcb, xOtcpReadMenuCommando );		// Terug de commando callback
		strcpy( cTemp, "okEnd" );
		tcp_write( pxTpcb, &cTemp, 5, 1 );
	}
	else
	{
		// We hebben nog niet alle data ontvangen, we sturen dat we het verwerkt hebben
		//	en wachten op meer data
		strcpy( cTemp, "ok2" );
		tcp_write( pxTpcb, &cTemp, 3, 1);
	}

	return ERR_OK;
}
/*-----------------------------------------------------------*/


/**
 * @brief      Het uploader prog heeft een naam gestuurt, deze steken we in flash
 *
 * @param      pvArg   Extra argumenten van lwip
 * @param      pxTpcb  De tcp-connectie
 * @param      pxBuf   De buffer met ontvangen data
 * @param[in]  xErr    De eventuele errors
 *
 * @return     We returnen een eventuele error
 */
err_t xOtcpSaveFileName( void * pvArg, struct tcp_pcb * pxTpcb, struct pbuf * pxBuf, err_t xErr)
{
FLASH_EraseInitTypeDef xEraseInitStruct;
uint32_t ulSectorError;
char cData[] = { "okEnd" };
uint8_t ucTeller = 0;

		tcp_recved( pxTpcb, pxBuf->len );

		// We maken de flash klaar om in te schrijven
		xEraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
		xEraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
		xEraseInitStruct.Sector = FLASH_SECTOR_3;
		xEraseInitStruct.NbSectors = 1;

		// We maken de flash sector leeg
		HAL_FLASH_Unlock();
		HAL_FLASHEx_Erase( &xEraseInitStruct, &ulSectorError );

		// We zetten een magic number om te zeggen of er effectief een titel is
		HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, otcpAPPLICATION_NAME_ADDRESS + ucTeller, 14 );

		for( ucTeller = 0; ucTeller < pxBuf->len; ucTeller++ )
		{
			HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, otcpAPPLICATION_NAME_ADDRESS + ucTeller+1, ( (char*)( pxBuf->payload ) )[ ucTeller ] );
		}

		HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, otcpAPPLICATION_NAME_ADDRESS + ucTeller+1, '\0' );

		HAL_FLASH_Lock();

		tcp_recv( pxTpcb, xOtcpReadMenuCommando );
		tcp_write( pxTpcb, &cData, 5, 1 );

		pbuf_free( pxBuf );
		return ERR_OK;
}
/*-----------------------------------------------------------*/

/* End public callback functions */

/* Begin static Callback functies voor de menu commando's */

/**
 * @brief      De user wilt dat we de firmwareversie terugsturen
 *
 * @param      pxTpcb  De tcp-connectie
 */
static void prvOvdeReturnFirmwareVersion( struct tcp_pcb * pxTpcb )
{
char cData[50];
char * pcPointer = otcpAPPLICATION_NAME_ADDRESS;

	if( *pcPointer == 14 )			// Als eerste data 14 is, is er een titel ingesteld
	{
		strcpy( cData, pcPointer + 1 );
	}
	else
	{
		strcpy( cData, "leeg" );
	}

	tcp_write( pxTpcb, &cData, strlen(cData), 1 );
}
/*-----------------------------------------------------------*/


/**
 * @brief      De user wilt dat we de bootloaderversie terugsturen
 *
 * @param      pxTpcb  De tcp-connectie
 */
static void prvOvdeReturnBootloaderVersion( struct tcp_pcb * pxTpcb )
{
char cWelkomMessage[] = { otcpWELCOME };

	tcp_write( pxTpcb, &cWelkomMessage, otcpWELCOME_LEN, 1 );
}
/*-----------------------------------------------------------*/


/**
 * @brief      We hebben het commando gekregen om een file te uploaden dus veranderen we
 * 			  	de receive callback
 *
 * @param      pxTpcb  De tcp-connectie
 */
static void prvOvdeSaveFile( struct tcp_pcb * pxTpcb )
{
char cData[] = { "ok" };

	tcp_recv( pxTpcb, xOtcpSaveFileToFlash );		// We veranderen de callback
	tcp_write( pxTpcb, &cData, 2, 1 );				// We sturen ok terug
}
/*-----------------------------------------------------------*/


/**
 * @brief      Het upload-prog vraagt om de naam van de user-app te mogen sturen
 *
 * @param      pxTpcb  De tcp-connectie
 */
static void prvOvdeSetFlashProgramName( struct tcp_pcb * pxTpcb )
{
char cData[] = { "ok" };
	tcp_recv( pxTpcb, xOtcpSaveFileName );			// We veranderen de callback
	tcp_write( pxTpcb, &cData, 2, 1 );				// We sturen ok terug
}
/*-----------------------------------------------------------*/


/**
 * @brief      Alle data is aangekomen en in flash geladen, nu sturen we alles block per block
 * 				terug voor verificatie
 *
 * @param      pxTpcb  De tcp-connectie
 */
static void prvOvdeReturnFlashData( struct tcp_pcb * pxTpcb )
{
static uint16_t usVerificationCounter = 0;
static uint8_t ucFiniched = 0;

char cData[10];

	if( ucFiniched == 1 )
	{
		// We sturen data het gedaan is!
		strcpy( cData, "end" );
		tcp_write( pxTpcb, &cData, 3, 1 );
		return;
	}

	// Checken of we al op het einde van de file in flash zitten
	if( ( usVerificationCounter * otcpTCP_BLOCKSIZE + otcpTCP_BLOCKSIZE ) > ullFileSize)
	{
		// Er is nog data maar minder dan 512-bytes
		tcp_write
		(
			pxTpcb,
			( otcpAPPLICATION_ADDRESS ) + usVerificationCounter * otcpTCP_BLOCKSIZE,
			ullFileSize - usVerificationCounter * otcpTCP_BLOCKSIZE,
			0
		);

		// Alle data is nu terug gestuurd
		ucFiniched = 1;
	}
	else if( ( usVerificationCounter * 512 ) == ullFileSize )
	{
		// Het is gewoon gedaan, het kwam mooi uit op een veelvoud van 512
		ucFiniched = 1;
	}
	else
	{
		// We zijn nog lang niet op het einde dus sturen 512-bytes
		tcp_write( pxTpcb, ( otcpAPPLICATION_ADDRESS ) + usVerificationCounter * otcpTCP_BLOCKSIZE, otcpTCP_BLOCKSIZE, 0 );
		usVerificationCounter ++;
	}
}
/*-----------------------------------------------------------*/


/**
 * @brief      De user wilt dit device resetten
 *
 * @param      pxTpcb  De tcp-connectie
 */
static void prvOvdeResetThisDevice( struct tcp_pcb * pxTpcb )
{
char cData[] = { "ok" };

	// We sturen even dat het ok is
	tcp_write( pxTpcb, &cData, 2, 1 );

	if( tcp_close( pxTpcb ) == ERR_OK )
	{
		// We resetten het device
		NVIC_SystemReset();
	}
}
/*-----------------------------------------------------------*/

/* End static Callback functies voor de menu commando's */
