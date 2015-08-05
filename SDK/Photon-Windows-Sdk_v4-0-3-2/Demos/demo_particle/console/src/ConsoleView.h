#pragma once

#include "../../src/BaseView.h"

class ConsoleView : public BaseView
{
public:
	virtual ~ConsoleView(void) {}
	virtual void info(const char* format, ...);
	virtual void warn(const char* format, ...);
	virtual void error(const char* format, ...);
	virtual void updateState(int state, const char* stateStr, const char* joinedRoomName);
	virtual void addPlayer(int playerNr, const char* playerName, bool local);
	virtual void removePlayer(int playerNr);
	virtual void changePlayerColor(int playerNr, int color);
	virtual void changePlayerPos(int playerNr, int x, int y);
    virtual void setupScene(int gridSize);
	virtual void updateRoomList(const char* roomNames[], unsigned int size);
};