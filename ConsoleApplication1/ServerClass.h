#pragma once
#include "SocketHelper.h"
#include <vector>
//#include <memory>
#include <string>
#include "BattleshipClass.h"

#define DEFAULT_BUFLEN 1024

static int roomAmount = 0;
static std::mutex obsLock;
static std::mutex userLock;

class GameRoom;
class User;

struct HelpFunctions
{
	static std::string mapToStr(const std::vector<std::vector<unsigned int>> & map);
	static std::vector<std::vector<unsigned int>> strToMap(const std::string & str, int sizeX, int sizeY);
	static bool getObjectAt(const std::string & str, unsigned int pos, std::string & object);
	static bool getMemberAt(const std::string & object, unsigned int pos, std::string & member);

	//static std::string userChatToStringMsg(const std::string & chatmsg, User* user);
};

class GameRoom;

class User
{
public:
	SOCKET socket;

	int id;
	bool left;
	std::string nick;
	int serverStatus; // 0 - just connected (not authenticated)
	                  // 1 - ready to join/create
					  // 2 - ingame - player
	                  // 3 - ingame - observator

	std::shared_ptr<GameRoom> room;

	void sendMsg(const std::string & msg);
};

class GameRoom
{
public:
	Battleship battle;

	bool isPublic;

	int gameStatus; // 0 - waiting for second player to join
	                // 1 - waiting for both maps
	                // 2 - waiting for player1 map
	                // 3 - waiting for player2 map
	                // 4 - player1 turn
	                // 5 - player2 turn
	                // 6 - game ended - player1 won
	                // 7 - game ended - player2 won
	User* player1;
	User* player2;
	std::vector<User*> observators;
	std::vector<std::string> chat;

	void sendMsgToAll(const std::string & msg); // maps and chat

	~GameRoom() { roomAmount--; };
};

class ServerClass
{
private:
	WSADATA wData;
	SOCKET serverSocket;

	std::vector<std::shared_ptr<User>> users;
	//std::vector<std::shared_ptr<GameRoom>> rooms;

	void startNetcode(const char* port);

	void threadManageConnections(ServerClass * servClass);
	void threadPlayer(ServerClass* servClass, std::shared_ptr<User> player);

	void readMsgAndAct(std::shared_ptr<User> player, const std::string & msg);

	// nulling all the pointers which will result in
	// invoking room destructor
	void clearRoom(std::shared_ptr<GameRoom> room);
	void clearUser(std::shared_ptr<User> player);
	void eraseUser(std::shared_ptr<User> player);
public:
	void init();
	void manageConnections();
	void managePlayer(std::shared_ptr<User> player);
};