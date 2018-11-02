#include "main.h"

using namespace std;

SAMPFUNCS *SF = new SAMPFUNCS();

bool CALLBACK Present(CONST RECT *pSourceRect, CONST RECT *pDestRect, HWND hDestWindowOverride, CONST RGNDATA *pDirtyRegion)
{
	if (SF->getGame()->isGTAMenuActive())
	{
		Set.dwLastPauseTick = GetTickCount();
		return true;
	}
	if (SUCCEEDED(SF->getRender()->BeginRender()))
	{

	}
	return true;
}

void CALLBACK mainloop()
{
	static bool init = false;
	if (!init)
	{
		if (GAME == nullptr)
			return;
		if (GAME->GetSystemState() != eSystemState::GS_PLAYING_GAME)
			return;
		if (!SF->getSAMP()->IsInitialized())
			return;

		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 255), "{01b729}[LVa Homecoming by M.Friedmann]{ffffff} - loaded.");
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 255), "{ffffff}Версия:{9607c1} Beta 0.6");
		SF->getRender()->registerD3DCallback(D3DMETHOD_PRESENT, Present);
		CMD();
		init = true;
	}
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved)
{
	switch (dwReasonForCall)
	{
		case DLL_PROCESS_ATTACH:
			SF->initPlugin(mainloop, hModule);
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
