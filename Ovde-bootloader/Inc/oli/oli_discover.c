/*
 * oli_discover.c
 *
 *  Created on: 2-dec.-2016
 *      Author: Olivier
 */

#include "oli_discover.h"

/* Begin extern variables */

extern uint8_t ucUserAppIsRunning;

/* End extern variables */

/*-----------------------------------------------------------*/

/* Begin Static function prototypes */

static void prvOdiscoverUdpPackageReceived( void * pvArg, struct udp_pcb * pxPcb, struct pbuf * pxBuf, const ip4_addr_t * pxAddr, u16_t usPort );
static void prvOdiscoverSendUserAppName( struct udp_pcb * pxPcb, const ip4_addr_t * pxAddr );
static void prvOdiscoverStartBootloaderMode( void );

/* End Static function prototypes */

/*-----------------------------------------------------------*/

/* Begin pubic functions */

/**
 * @brief      Deze functie initialiseerd het discover protocol over udp.
 * 				Zo kan de applicatie reageren op een discover-commando van het upload programma
 */
void vOdiscoverInitUdpDiscover(void)
{
struct udp_pcb * pxUdpConnectie;

	pxUdpConnectie = udp_new();
	udp_bind( pxUdpConnectie, IP_ADDR_ANY, 65001 );
	udp_recv( pxUdpConnectie, prvOdiscoverUdpPackageReceived, 0 );
}

/*-----------------------------------------------------------*/

/* End pubic functions */

/* Begin Static functions */

/**
 * @brief      Hier komen we als er een udp-paket binnenkomt op de discover-poort
 *
 * @param      pvArg   Extra argumenten van lwip
 * @param      pxPcb   De udp-connectie van lwip
 * @param      pxBuf   De buffer met de ontvangen data
 * @param[in]  pxAddr  Het ip-adress vanwaar het request gekomen is
 * @param[in]  usPort  De udp-poort
 */
static void prvOdiscoverUdpPackageReceived( void * pvArg, struct udp_pcb * pxPcb, struct pbuf * pxBuf, const ip4_addr_t * pxAddr, u16_t usPort )
{
	// We kijken of het een vraag is om ons te zoeken
	if( strcmp( (char*)pxBuf->payload, "discoverOvde-boot" ) == 0 )
	{
		prvOdiscoverSendUserAppName(pxPcb,pxAddr);
	}

	// We kijken of we niet moeten herstarten in bootloader mode
	if( strcmp( (char*)pxBuf->payload, "restartOvde-boot" ) == 0  )
	{
		prvOdiscoverStartBootloaderMode();
	}

	// We maken de ontvangstbuffer nog leeg
	pbuf_free( pxBuf );
}

/*-----------------------------------------------------------*/

/**
 * @brief      We sturen de naam van het ingeladen programma terug naar de zender van het request
 *
 * @param      pxPcb   De udp connectie
 * @param[in]  pxAddr  Het ip-adress vanwaar het request gekomen is
 */
static void prvOdiscoverSendUserAppName( struct udp_pcb * pxPcb, const ip4_addr_t * pxAddr )
{

char cData[100];
char cLeegText[] = { "leeg" };
volatile char * pcPointer = otcpAPPLICATION_NAME_ADDRESS;
struct pbuf * pxSendData;


	// Als eerste data 14 is, is er een titel ingesteld
	if( *pcPointer == 14 )
	{
		pcPointer++;		// Er is een titel ingesteld, 1byte verder begint de titel
	}
	else
	{
		pcPointer = cLeegText;		// Er is geen titel, we sturen "leeg" terug
	}

	// We antwoorden dat we gevonden zijn
	sprintf( cData, "targetFound-%s", pcPointer );

	if( ucUserAppIsRunning )	// Als de user-app aan het runnen is, zeggen we dit
	{
		sprintf( cData, "%s running", cData );
	}

	// We maken een buffer aan om data te verzenden
	pxSendData = pbuf_alloc( PBUF_TRANSPORT,100,PBUF_REF );
	pxSendData->payload = cData;
	pxSendData->len = pxSendData->tot_len = strlen( cData );

	// We sturen onze data terug naar de zender
	udp_sendto( pxPcb, pxSendData, pxAddr, 65000 );
	pbuf_free( pxSendData );
}

/*-----------------------------------------------------------*/

/**
 * @brief      We herstarten het device in bootloader mode
 */
static void prvOdiscoverStartBootloaderMode( void )
{

char cData[100];
volatile char * pcPointer = otcpAPPLICATION_NAME_ADDRESS;
uint8_t ucTitelIsProgrammed = 0;

FLASH_EraseInitTypeDef xEraseInitStruct;
uint32_t ulSectorError;
uint8_t ucTeller;

	// Als eerste data 14 is, is er een titel ingesteld
	if( *pcPointer == 14 )
	{
		ucTitelIsProgrammed = 1;	// Er zit een titel in de flash, die moet er terug in
		pcPointer++;		// Er is een titel ingesteld, 1byte verder begint de titel
		strcpy( cData, pcPointer );		// We kopieren de tekst
	}


	// We maken de flash klaar om in te schrijven,
	// 	de magic number zit samen met de titel in sector 3
	xEraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	xEraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	xEraseInitStruct.Sector = FLASH_SECTOR_3;
	xEraseInitStruct.NbSectors = 1;

	// We maken de flash sector leeg
	HAL_FLASH_Unlock();
	if( HAL_FLASHEx_Erase( &xEraseInitStruct, &ulSectorError ) == HAL_OK )
	{
		// We zetten een magic number om te zeggen dat we de bootloader willen starten bij reboot
		HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, otcpAPPLICATION_NAME_ADDRESS, 15 );

		if( ucTitelIsProgrammed )
		{
			// We schrijven de titel terug op de juiste plaats
			for( ucTeller = 0; ucTeller < strlen( cData ) + 1; ucTeller++ )
			{
				HAL_FLASH_Program( FLASH_TYPEPROGRAM_BYTE, otcpAPPLICATION_NAME_ADDRESS + ucTeller + 1, cData[ucTeller] );
			}
		}
	}
	HAL_FLASH_Lock();

	// We herstarten het device, de bootloader weet door de magic number wat er moet gebeuren
	NVIC_SystemReset();
}

/*-----------------------------------------------------------*/

/* End Static functions */


