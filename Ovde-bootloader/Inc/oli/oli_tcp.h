/*
 * oli_commands.h
 *
 *  Created on: 17-nov.-2016
 *      Author: Olivier
 */

#ifndef OLI_COMMANDS_H_
#define OLI_COMMANDS_H_

#include "stm32f7xx_hal.h"
#include "lwip.h"
#include "tcp.h"
#include "string.h"

#define otcpWELCOME "Ovde-boot V2.0"
#define otcpWELCOME_LEN 14
#define otcpAPPLICATION_ADDRESS ( (uint32_t) 0x08020000 )
#define otcpAPPLICATION_NAME_ADDRESS ( (uint32_t) 0x08018000 )
#define otcpTCP_BLOCKSIZE 512
#define otcpBOOTLOADER_SERVER_PORT 5555

/**
 * Deze wordt gebruikt om een gelinkte lijst op te bouwen met de commando's 
 * die door deze functies kunnen uitgevoerd worden
 */
typedef struct Commando
{
	char cAfkorting;
	struct Commando * pxNext;
	void ( *functie )( struct tcp_pcb * pxTpcb );
} commando;

// De callbackfuncties die opgeroepen worden als er tcpdata onvangen is
err_t xOtcpReadMenuCommando( void * pvArg, struct tcp_pcb * pxTpcb, struct pbuf * pxBuf, err_t xErr );
err_t xOtcpSaveFileToFlash( void * pvArg, struct tcp_pcb * pxTpcb, struct pbuf * pxBuf, err_t xErr);
err_t xOtcpSaveFileName( void * pvArg, struct tcp_pcb * pxTpcb, struct pbuf * pxBuf, err_t xErr);


#endif /* OLI_COMMANDS_H_ */
