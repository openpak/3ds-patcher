#include "sysmodules/acta.hpp"
#include "sheet.h"
#include "sheet_t3x.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.hpp"
#include "states/LumaValidation.hpp"
#include "states/MainUI.hpp"

MainStruct mainStruct = MainStruct();

static void sceneInit(void)
{
	C2D_SpriteSheet spriteSheet = C2D_SpriteSheetLoadFromMem(sheet_t3x, sheet_t3x_size);
	C2D_SpriteFromSheet(&mainStruct.top, spriteSheet, sheet_top_idx);
	C2D_SpriteSetCenter(&mainStruct.top, 0.0f, 0.0f);
	C2D_SpriteSetPos(&mainStruct.top, 12, 12);

	textBuf = C2D_TextBufNew(4096); // initialize the text buffer with a max glyph count of 4096
}

int main()
{
	// Initialize the libs
	romfsInit();   // the plugin and certificate travel inside the app
	nsInit();
	ndmuInit();
	frdInit(false);
	actInit(false);
	gfxInitDefault();
	
	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();

	// This version or higher is required creating/swapping friend accounts
	FRD_SetClientSdkVersion(0x70000c8);

	// Create screen
	C3D_RenderTarget* top_screen = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	C3D_RenderTarget* bottom_screen = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);

	// Initialize the scene
	sceneInit();

	// set button selected and current account to nasc environment
	u8 nascEnvironment, nfsType, nfsNo;
	FRD_GetServerTypes(&nascEnvironment, &nfsType, &nfsNo);
	
	mainStruct.buttonSelected = static_cast<NascEnvironment>(nascEnvironment);
	mainStruct.currentAccount = mainStruct.buttonSelected;

	// NULL-terminate string
	mainStruct.errorString[0] = 0;

	// Main loop
	while (aptMainLoop()) {
		bool exit = false;
		
		// get any input, and if applicable the location where the screen was touched
		hidScanInput();
		touchPosition touch;
		hidTouchRead(&touch);
		u32 kDown = hidKeysDown();
		u32 kHeld = hidKeysHeld();
		
		// Render the scene
		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		
		C2D_TargetClear(top_screen, COLOR_PAPER);
		C2D_TargetClear(bottom_screen, COLOR_PAPER);
		
		// TODO: change firstRunOfState into stateRunIndex, incrementing every time the state is run
		if (mainStruct.lastState != mainStruct.state) mainStruct.firstRunOfState = true;
		
		if (mainStruct.state == 0) {
			exit = LumaValidation::checkIfLumaOptionsEnabled(&mainStruct, top_screen, bottom_screen, kDown, kHeld, touch);
		} else {
			exit = MainUI::drawUI(&mainStruct, top_screen, bottom_screen, kDown, kHeld, touch);
		}
		
		mainStruct.lastState = mainStruct.state;
		mainStruct.firstRunOfState = false;
		
		C3D_FrameEnd(0);
		
		if (exit) break;
	}

	// Deinitialize the libs
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	actExit();
	frdExit();
	ndmuExit();

	if (mainStruct.needsReboot) {
		NS_RebootSystem();
	}

	nsExit();
	romfsExit();

	return 0;
}
