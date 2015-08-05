#include <iostream>
#include "ChatListener.h"
#ifdef _EG_PS4_PLATFORM
#	include "Controller.h"
#	include "Keyboard.h"
#endif

using namespace ExitGames;
using namespace ExitGames::Common;

static bool gUseTcp = false;

static const JString appId = L"<no-app-id>"; // set your app id here
static const JString appVersion = L"1.0";

#if defined _EG_WINDOWS_PLATFORM
#	define SLEEP(ms) Sleep(ms)
#else
#	define SLEEP(ms) usleep(ms*1000)
#endif

static const int G_USER_COUNT = 512;

static JVector<JString> gFriends;

static bool typeMode = false;

#ifdef _EG_PS4_PLATFORM
static Keyboard* gpKeyboard;
#endif

static void usage();
static int handleKbd(Chat::Client& chatClient);

static void subscribe(Chat::Client& chatClient)
{
	JVector<JString> chs;
	chs.addElement("a");
	chs.addElement("b");
	chs.addElement("v");
	if(chatClient.opSubscribe(chs))
		Console::get().writeLine(L"Subscribing...");
	if(chatClient.opAddFriends(gFriends))
		Console::get().writeLine(JString(L"Adding friends: ") + gFriends.getFirstElement() + L" ... " + gFriends.getLastElement());
}

static void unsubscribe(Chat::Client& chatClient)
{
	JVector<JString> chs;
	chs.addElement("a");
	chs.addElement("nonexisting");
	chs.addElement("b");
	chs.addElement("v");
	if(chatClient.opUnsubscribe(chs))
	Console::get().writeLine(L"Unsubscribing...");
	if(chatClient.opRemoveFriends(gFriends))
		Console::get().writeLine(JString(L"Clearing friends: ") + gFriends.getFirstElement() + L" ... " + gFriends.getLastElement());
}

int main(int argc, const char* argv[])
{
	usage();
	ChatListener chatListener;
	JString usernamePrefix(L"user");
	int t = GETTIMEMS();
	int localIndex = (t > 0 ? t : -t) % G_USER_COUNT;
	JString username;
	for(int i=0; i<G_USER_COUNT; ++i)
	{
		if(i == localIndex)
			username = usernamePrefix + i;
		else
			gFriends.addElement(usernamePrefix + i);
	}
	Chat::Client chatClient(chatListener, appId, appVersion, username, gUseTcp ? Photon::ConnectionProtocol::TCP:Photon::ConnectionProtocol::UDP);
	chatClient.setDebugOutputLevel(DebugLevel::INFO);

	Console::get().writeLine(L"Connecting to nameserver as user " + username);
	Console::get().writeLine();

	chatClient.connect();
	while(true)
	{
		chatClient.service();
		Console::get().update();
#ifdef _EG_PS4_PLATFORM
		static Controller controller;
		if(controller.readInput() == 'h')
		{
			gpKeyboard = MemoryManagement::allocate<Keyboard>();
			typeMode = true;
			Console::get().write(L" <<< ");
		}
		if(controller.readInput() == '0')
			break;
		if(controller.readInput() == '1')
		{
			gpKeyboard = MemoryManagement::allocate<Keyboard>();
			typeMode = false;
		}
#endif
		if(handleKbd(chatClient))
			break;
	}

	return 0;
}

static const JString statusNames[] = {L"OFFLINE", L"INVISIBLE", L"ONLINE", L"AWAY", L"DND", L"LFG", L"PLAYING"};

static void usage()
{
	Console::get().writeLine(L"Photon Chat Demo");
	Console::get().writeLine();
#ifdef _EG_PS4_PLATFORM
	Console::get().writeLine(L"Cross - Open the on screen keyboard for command input");
	Console::get().writeLine(L"Triangle - Open the on screen keyboard for chat message input");
	Console::get().writeLine(L"Circle - Close the application");
	Console::get().writeLine();
	Console::get().writeLine(L"use ':' and '@' to specify a channel or a user as receiver for a message:");
	Console::get().writeLine(L"bla - publish message to random channel");
	Console::get().writeLine(L"x:bla - publish message to channel 'x'");
	Console::get().writeLine(L"x@bla - send private message to user 'x'");
#else
	Console::get().writeLine(L"Esc - close the application");
	Console::get().writeLine();
	Console::get().writeLine(L"<enter>bla<enter> - publish message to random channel");
	Console::get().writeLine(L"<enter>x:bla<enter> - publish message to channel 'x'");
	Console::get().writeLine(L"<enter>x@bla<enter> - send private message to user 'x'");
#endif
	Console::get().writeLine();
	Console::get().writeLine(L"available commands:");
	Console::get().writeLine(L"h - print this help");
	Console::get().writeLine(L"s - subscribe");
	Console::get().writeLine(L"u - unsubscribe");
	Console::get().writeLine(L"d - disconnect");
	Console::get().writeLine(L"c - connect");
	Console::get().writeLine(L"l - list all channels and messages");
	for(int i=0; i<sizeof(statusNames)/sizeof(JString); ++i)
		Console::get().writeLine(JString('1') + i + L" - " + statusNames[i]);
	Console::get().writeLine();
}

#ifdef _EG_WINDOWS_PLATFORM
#include <conio.h>
static int getcharIfKbhit(void)
{
	int res = _kbhit();
	if(res)
		res = _getch();
	return res;
}
#elif defined _EG_PS4_PLATFORM
wchar_t* getcharIfKbhit(void)
{
	return gpKeyboard?gpKeyboard->readInput():NULL;
}
#else
#include <termios.h>
#include <fcntl.h>
int getcharIfKbhit(void);
int getcharIfKbhit(void)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	return ch;
}
#endif

static void listChannels(Chat::Client& chatClient);

static int handleKbd(Chat::Client& chatClient)
{
	static JString typeBuf;

#ifdef _EG_PS4_PLATFORM
	wchar_t* pK = getcharIfKbhit();
	size_t len = pK?wcslen(pK):0;
	int k = pK?pK[len?len-1:0]:0;
	if(!pK)
#else
	int k = getcharIfKbhit();
	if(!k)
#endif
		return 0;

#ifdef _EG_PS4_PLATFORM
	MemoryManagement::deallocate(gpKeyboard);
	gpKeyboard = NULL;

	typeBuf = pK;
	if(typeMode)
	{
		Console::get().write(pK);
		k = 10;
	}
#endif
	if(k == 27)
		return 1;
	else if(k == 10 || k == 13)
	{
		Console::get().writeLine();
		typeMode = !typeMode;
		if(!typeMode)
		{
			int sep = typeBuf.indexOf(':');
			if(sep > 0)
				chatClient.opPublishMessage(typeBuf.substring(0, sep), ValueObject<JString>(typeBuf.substring(sep+1)));
			else
			{
				sep = typeBuf.indexOf('@');
				if(sep > 0)
					chatClient.opSendPrivateMessage(typeBuf.substring(0, sep), ValueObject<JString>(typeBuf.substring(sep+1)));
				else
				{
					static int cnt = 0;
					const JVector<Chat::Channel*>& chs = chatClient.getPublicChannels();
					if(chs.getSize())
						chatClient.opPublishMessage(chs[cnt++ % chs.getSize()]->getName(), ValueObject<JString>(typeBuf));
				}
			}
		}
		else
			Console::get().write(L" <<< ");
		typeBuf = JString();
	}
	else
	{
		if(typeMode)
		{
#ifndef _EG_PS4_PLATFORM
			typeBuf += static_cast<EG_CHAR>(k);
			Console::get().write(JString() + static_cast<EG_CHAR>(k));
#endif
		}
		else
		{
			k = tolower(k);
			if(k == 'h')
				usage();
			if(k == 's')
				subscribe(chatClient);
			if(k == 'u')
				unsubscribe(chatClient);
			if(k == 'c')
			{
				Console::get().writeLine(L"connecting");
				chatClient.connect();
			}
			if(k == 'd')
			{
				Console::get().writeLine(L"disconnecting");
				chatClient.disconnect();
			}
			if(k == 'l')
				listChannels(chatClient);
			if(k >= '0' && k <= '6')
			{
				int s = k - '0';
				Console::get().writeLine(JString(L"Changing online status to ") + s);
				chatClient.opSetOnlineStatus(s, ValueObject<JString>(JString("My status changed to ") + statusNames[s]));
			}
		}
	}
	return 0;
}

static void listChannels(Chat::Client& chatClient)
{
	Console::get().writeLine(L"Channels:");
	for(unsigned int i=0; i<chatClient.getPublicChannels().getSize(); ++i)
	{
		// test
		const JString& name = chatClient.getPublicChannels()[i]->getName();
		const Chat::Channel* ch = chatClient.getPublicChannel(name);
		Console::get().writeLine(ch->getName() + L": " + ch->getMessageCount() + L" messages");
	}
	Console::get().writeLine(L"--------");
	Console::get().writeLine(L"Private Channels:");
	for(unsigned int i=0; i<chatClient.getPrivateChannels().getSize() ;++i)
	{
		// test
		const JString& name = chatClient.getPrivateChannels()[i]->getName();
		const Chat::Channel* ch = chatClient.getPrivateChannel(name);
		Console::get().writeLine(ch->getName() + L": " + ch->getMessageCount() + L" messages");
	}
	Console::get().writeLine(L"--------");
	Console::get().writeLine(L"Messages:");
	const JVector<Chat::Channel*>& chs = chatClient.getPublicChannels();
	for(unsigned int i=0; i<chs.getSize(); ++i)
	{
		const Chat::Channel* ch = chs[i];
		for(unsigned int i=0; i<ch->getMessageCount(); ++i)
			Console::get().writeLine(L"[" + ch->getName() + L": " + ch->getSenders()[i] + L"] >>> " + ch->getMessages()[i].toString());
	}
	const JVector<Chat::Channel*>& pchs = chatClient.getPrivateChannels();
	for(unsigned int i=0; i<pchs.getSize(); ++i)
	{
		const Chat::Channel* ch = pchs[i];
		for(unsigned int i=0; i<ch->getMessageCount(); ++i)
			Console::get().writeLine(L"[Private: " + ch->getName() + L": " + ch->getSenders()[i] + L"] >>> " + ch->getMessages()[i].toString());
	}
	Console::get().writeLine(L"--------");
}