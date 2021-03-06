/*
	main.c - eris's wai ossum dual sprite lib
		http://blea.ch/wiki/
		Kallisti (K) 2008-01-26 All rights reversed.

	v0.0 - Original
		This is insanely not yet ready for mass consumption... 
*/

// Includes
#include <nds.h>
#include <stdio.h>
#include <nitrofs.h>
#include <fat.h>
#include <stdlib.h>
#include <nds/arm9/console.h>   //basic print funcionality
#include <fcntl.h>
#include <unistd.h>


//YOU PROLLY WANNA CHANGE THIS TO WHATEVER YOUR .nds FILE IS CALLED AND SET CORRECT PATH
#define FILENAME "thsprite.nds"    //Need some way of finding the file, i know this is horrible but its better than searching every file for a magic key imo anyways... ~_~

// Function: main()
int main(int argc, char ** argv)
{
	touchPosition touchXY;
	bool imfat;	//indicate fat driver loaded properly
	int c;
	int a=0;
	int scnt=0;
	int dly=0;
        // initialise the irq dispatcher
        irqInit();
        powerON( POWER_LCD | POWER_2D_B );
        irqSet(IRQ_VBLANK, NULL);
        irqEnable(IRQ_VBLANK);

	// Use the touch screen for output
	videoSetMode(MODE_FB0);
//	vramSetBankA(VRAM_A_LCD);
//	videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);

	//set mode (these may best be done in thspriteinit!!)
	videoSetMode(MODE_0_2D|DISPLAY_SPR_ACTIVE|DISPLAY_BG0_ACTIVE|DISPLAY_SPR_1D|DISPLAY_SPR_EXT_PALETTE|DISPLAY_SPR_1D_BMP|DISPLAY_BG_EXT_PALETTE|DISPLAY_SPR_1D_SIZE_128);

//Set sub mode
	videoSetModeSub(MODE_0_2D|DISPLAY_SPR_ACTIVE|DISPLAY_BG0_ACTIVE|DISPLAY_SPR_1D|DISPLAY_SPR_EXT_PALETTE|DISPLAY_SPR_1D_BMP|DISPLAY_BG_EXT_PALETTE|DISPLAY_SPR_1D_SIZE_128);	//sub bg 0 will be used to print 

	vramSetBankC(VRAM_C_SUB_BG);
	SUB_BG0_CR = BG_MAP_BASE(31);

	// Set the colour of the font to White.
	BG_PALETTE_SUB[255] = RGB15(31,31,31);
	consoleInitDefault((u16*)SCREEN_BASE_BLOCK_SUB(31), (u16*)CHAR_BASE_BLOCK_SUB(0), 16);

	printf("hello world!...\ninitalizing fat\n");
/*	if(fatInitDefault()) {
		imfat=true;
		printf("Success...\n");
	} else {
		imfat=false;
		printf("Failed... (normal for some emulators. Dont worry, yet :p)\n");
	}
*/	printf("initalizing nitrofs\n");
	//pass argument of what filename to use (i cant think of a clean way to find the file so leaving it up to end users to do choose)
	if(nitroFSInit(FILENAME)!=0) {

		elfStart("nitro:/arm-eabi-test.o");
		while(1) {
			touchXY=touchReadXY();
			swiWaitForVBlank();

		};

	} else {
		printf("Failed...\n");
		if(imfat) { //error if fat was initialized...
			printf("Cannot open %s please check filename and path are correct.\n",FILENAME);
		} else { //error if fat/dldi failed
			printf("FAT/DLDI error, please to ensure your card supports DLDI.\n");
		}
	}
	// Infinite loop to keep the program running
	while (1)
	{
		swiWaitForVBlank();
	}
	return 0;
} // End of main()
