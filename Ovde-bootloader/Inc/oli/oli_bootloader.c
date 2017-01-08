/*
 * oli_bootloader.c
 *
 *  Created on: 27-dec.-2016
 *      Author: Olivier
 */

#include "oli_bootloader.h"

/*-----------------------------------------------------------*/

/* Begin Static function prototypes */

static void prvObootloaderUserOnderbreking( void );

/* End Static function prototypes */


/* Begin extern variables */

extern TIM_HandleTypeDef htim2;			// De timer die we gebruiken om terug te keren
extern uint8_t ucUserAppIsRunning;		// Het bitje om te zeggen dat de userApp bezig is
extern uint32_t userVectorTable[80];			// De nieuwe dynamische vectorTabel

/* End extern variables */

/*-----------------------------------------------------------*/

/* Begin public functions */

/**
 * @brief      Deze functie handeld alles om juist het nieuwe programma op te starten
 *
 * @param[in]  ulAppAdress  Het startadress van de applicatie in flash
 */
void vObootloaderJumpToApplication( uint32_t ulAppAdress )
{

typedef void ( *pFunction )( void );
uint32_t ulAppStack;
pFunction xAppEntry;

	// Het eerste address in het geflashte programma is de locatie van de stack
	ulAppStack = ( uint32_t ) *( ( volatile uint32_t* ) ulAppAdress );

	// Het 2de adress in flash is de locatie van de reset handler, dus het entery point
	xAppEntry = ( pFunction ) *( volatile uint32_t* ) ( ulAppAdress + 4 );

	// We copieren heel de user vectortabel naar ram
	for(uint8_t i=0; i < 79; i++)
	{
		userVectorTable[i] = *( ( volatile uint32_t* ) ulAppAdress+i );
	}

	// Omdat de vectortabel nu in ram zit kunnen we deze aanpassen
	userVectorTable[44] = (uint32_t) prvObootloaderUserOnderbreking;

	// We initialiseren lwip voor discovery + reset
	HAL_Init();
	MX_LWIP_Init();
	vOdiscoverInitUdpDiscover();

	// We willen dat de geflashte locatie ook interupts kan gebruiken,
	// 	Dus verzetten we de vecor tabel
	SCB->VTOR = (uint32_t) userVectorTable;

	// We starten nog even de timer zodat we periodisch terugkunnen uit de user app
	HAL_TIM_Base_Start_IT( &htim2 );

	// We zetten de stackpointer op het juiste(eerste) adress in ram
	__set_MSP( ulAppStack );

	// We hebben een bitje om te zeggen dat de user app bezig is
	ucUserAppIsRunning = 1;

	// We springen (zetten de Program counter) naar de reset handler van het nieuwe programma
	xAppEntry();
}

/*-----------------------------------------------------------*/

/* End public functions */

/* Begin Static functions */

/**
 * @brief     Met behulp van een timer, en een dynamische vectortabel
 * 				Waarbij we de interupt van deze timer "stelen" van de user-app, kunnen we 
 * 				periodisch terug vanuit de user-app naar de bootloader.
 * 				Zo kunnen we ookal is de user-app aan het runnen reageren op discover-paketten
 */
static void prvObootloaderUserOnderbreking(void)
{
	// We clearen de interupt flag
	__HAL_TIM_CLEAR_IT(&htim2,TIM_IT_UPDATE);

	// We zetten de interupts uit voor de veiligheid
	__disable_irq();

	// De pin hoog en laag maken om lengte te testen
	//HAL_GPIO_WritePin(Led_GPIO_Port,Led_Pin,1);

	// We zorgen dat systick terug voor de bootloader zal tikken
	uint32_t backupSystickHandler = userVectorTable[15];
	userVectorTable[15] = (uint32_t) SysTick_Handler;

	// De eigenlijke uit te voeren code
	// 	We laten lwip 1 cyclus doen op zoek naar udp-paketten
	MX_LWIP_Process();

	// De user app krijgt terug de systick, we hebben hem niet meer nogig
	userVectorTable[15] = backupSystickHandler;

	//HAL_GPIO_WritePin(Led_GPIO_Port,Led_Pin,0);

	__enable_irq();
}
/*-----------------------------------------------------------*/

/* End Static functions */
