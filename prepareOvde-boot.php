<?php

$save = TRUE;

$LinkerscriptNAME = "STM32F746NGHx_FLASH.ld";
$systemFile = "Src/system_stm32f7xx.c";
$buildFOLDER = "Debug";

$colorGreen = "\033[32m";
$colorRed = "\033[31m";
$colorBlue = "\033[36m";
$colorWhite = "\033[0m";

/* 
 * We passen 4 dingen aan in het linkerscript: 
 *	
 *	- De groote van de stack
 *	- Het begin van de stack
 *	- De grootte, en het begin van de ram
 *	- De gtootte en het begin van de flash
 *
 */
echo $colorGreen . "\n\nOpenen van het linkerscript.\n";
echo $colorBlue . "/* 
 * We passen 4 dingen aan in het linkerscript: 
 *	
 *	- De groote van de stack
 *	- Het begin van de stack
 *	- De grootte, en het begin van de ram
 *	- De gtootte en het begin van de flash
 *
 */\n";

if(!file_exists($LinkerscriptNAME))
{
	echo $colorRed . "Linkerscript " . $LinkerscriptNAME . " is niet gevonden.\n";
	die($colorRed . "Gelieve dit script in de root van uw project te plaatsen.\n");
}

$lines = file($LinkerscriptNAME, FILE_IGNORE_NEW_LINES);

$allesGevonden = 0;

for ($teller=0; $teller < count($lines); $teller++) 
{
	// We zoeken naar het begin van de stack
	if(strpos($lines[$teller], "_estack = 0x20050000;    /* end of RAM */") !== FALSE)
	{
		$lines[$teller] = "_estack = 0x2004E200;    /* end of RAM */";
		echo $colorWhite . "Begin van stack\t". $colorGreen ."Ok\n";
		$allesGevonden++;
	}

	// We zoeken naar de stack size
	if(strpos($lines[$teller], "_Min_Stack_Size = 0x400; /* required amount of stack */") !== FALSE)
	{
		$lines[$teller] = "_Min_Stack_Size = 0x200; /* required amount of stack */";
		echo $colorWhite . "Groote van de stack\t". $colorGreen ."Ok\n";
		$allesGevonden++;
	}

	// We zoeken naar het begin en grootte van de flash
	if(strpos($lines[$teller], "FLASH (rx)      : ORIGIN = 0x8000000, LENGTH = 1024K") !== FALSE)
	{
		$lines[$teller] = "FLASH (rx)      : ORIGIN = 0x8020000, LENGTH = 896K";
		echo $colorWhite . "Flashsize en plaats\t". $colorGreen ."Ok\n";
		$allesGevonden++;
	}

	// We zoeken naar het begin en grootte van de ram
	if(strpos($lines[$teller], "RAM (xrw)      : ORIGIN = 0x20000000, LENGTH = 320K") !== FALSE)
	{
		$lines[$teller] = "RAM (xrw)      : ORIGIN = 0x20027110, LENGTH = 160K";
		echo $colorWhite . "Ramsize en plaats\t". $colorGreen ."Ok\n";
		$allesGevonden++;
	}
}

if($allesGevonden != 4)
{
	echo $colorRed . "Er is een fout opgetreden bij het aanpassen van het linkerscript.\n";
	die($colorRed . "Het zou kunnen da dit script al eens uitgevoerd is.\n");
}

if($save)
	file_put_contents( $LinkerscriptNAME , implode( "\n", $lines ) );


/**
 * In de system-file worden 2 dingen aangepast:
 *  - We zorgen ervoor dat de interupts die al aanstaan niet uitgezet worden
 *  - De vectortabel niet overschreven wordt
 */
echo $colorGreen . "\n\nOpenen van system file.\n";
echo $colorBlue . "/**
 * In de system-file worden 2 dingen aangepast:
 *  - We zorgen ervoor dat de interupts die al aanstaan niet uitgezet worden
 *  - De vectortabel niet overschreven wordt
 */\n";

if(!file_exists($systemFile))
{
	echo $colorRed . "System-file " . $systemFile . " is niet gevonden.\n";
	die($colorRed . "Gelieve na te kijken of deze file bestaat.\n");
}

$lines = 0;
$lines = file($systemFile, FILE_IGNORE_NEW_LINES);
$allesGevonden = 0;
$teller = 0;

for($teller = 0; $teller < count($lines); $teller++)
{
	if(strpos($lines[$teller],"  RCC->CIR = 0x00000000;") !== FALSE)
	{
		echo $colorWhite . "Interupt disable verwijderen\t". $colorGreen ."Ok\n";
		$lines[$teller] = "";
		$allesGevonden++;
	}

	if(strpos($lines[$teller],"  SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH */") !== FALSE)
	{
		echo $colorWhite . "Init vectortabel verwijderen\t". $colorGreen ."Ok\n";
		$lines[$teller] = "";
		$allesGevonden++;
	}
}

if($save)
	file_put_contents( $systemFile , implode( "\n", $lines ) );



/**
 *	We verwijderen de Debug folder omdat eclipse geforceerd ALLES 
 *	moet herbuilden door de aanpassingen in het linkerscript
 */
echo $colorGreen . "\n\nVerwijderen van vorige builds\n";
echo $colorBlue . "/**
 *	We verwijderen de Debug folder omdat eclipse geforceerd ALLES 
 *	moet herbuilden door de aanpassingen in het linkerscript
 */\n";

if(is_dir($buildFOLDER))
{
	shell_exec("cd ". $buildFOLDER . " && make clean");
	shell_exec("rm -rf ".$buildFOLDER);
	echo $colorWhite . "Verwijderen\t" . $colorGreen . "Ok\n";
}

echo $colorBlue . "\nVergeet niet een make clean te doen voor je opnieuw kan builden.\n";
echo $colorGreen . "Alles is succesvol voorbereid voor het gebruik van Ovde-boot.\n";

?>