#include <stdarg.h>
#include <stdio.h>

#include "ConsoleView.h"
#include "Console.h"

#ifdef _EG_ANDROID_PLATFORM
#	include <android/log.h>
#endif

void ConsoleView::info(const char* format, ...) 
{
	static char str[1024];
	sprintf(str, "INFO: ");
	va_list argptr;
    va_start(argptr, format);
    vsprintf(str, format, argptr);
    va_end(argptr);
	Console::get().writeLine(str);
#ifdef _EG_ANDROID_PLATFORM
	__android_log_vprint(ANDROID_LOG_INFO, "DemoParticle", format, argptr);
#endif
}

void ConsoleView::warn(const char* format, ...) 
{
	static char str[1024];
	sprintf(str, "WARN: ");
	va_list argptr;
	va_start(argptr, format);
	vsprintf(str, format, argptr);
	va_end(argptr);
	Console::get().writeLine(str);
#ifdef _EG_ANDROID_PLATFORM
	__android_log_vprint(ANDROID_LOG_INFO, "DemoParticle", format, argptr);
#endif
}

void ConsoleView::error(const char* format, ...) 
{
	static char str[1024];
	sprintf(str, "ERROR: ");
	va_list argptr;
	va_start(argptr, format);
	vsprintf(str, format, argptr);
	va_end(argptr);
	Console::get().writeLine(str);
#ifdef _EG_ANDROID_PLATFORM
	__android_log_vprint(ANDROID_LOG_INFO, "DemoParticle", format, argptr);
#endif
}

void ConsoleView::updateState(int state, const char* stateStr, const char* joinedRoomName) 
{
	info("state: %d/%s", state, stateStr);
}

void ConsoleView::addPlayer(int playerNr, const char* playerName, bool local) 
{
	info("player added: %s / %d ", playerName, playerNr);
}

void ConsoleView::removePlayer(int playerNr) 
{
	info("player removed: %d", playerNr);
}

void ConsoleView::changePlayerColor(int playerNr, int color) 
{
	info("player %d color: %d", playerNr, color);
}

void ConsoleView::changePlayerPos(int playerNr, int x, int y) 
{
	info("player %d pos: %d, %d", playerNr, x, y);
}

void ConsoleView::setupScene(int gridSize)
{
	info("setup scene with gridSize=%d", gridSize);
}

void ConsoleView::updateRoomList(const char** roomNames, unsigned int size) 
{
	info("room list:");
	for(unsigned int i=0; i<size; ++i) 
		info("  %s", roomNames[i]);
}