#pragma once


#ifndef __MODMAIN_H
#define __MODMAIN_H

#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS 1
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS 1
#define AddChatMessages       SF->getSAMP()->getChat()->AddChatMessage
#include <windows.h>
#include <string>
#include <assert.h>
#include <process.h>
#include <string>
#include <chrono>
#include <thread>
#include <WinInet.h>
#include "SAMPFUNCS_API.h"
#include "game_api\game_api.h"

#pragma comment (lib, "WinInet.lib")


//function
#include "Setting.h"
#include "CMD.h"

extern SAMPFUNCS *SF;
#endif
