#include "stdafx.h"
#include "ServerClass.h"

void ServerClass::startNetcode(const char* port)
{
	SocketHelper sh;

	try
	{
		sh.startup(wData);
		struct addrinfo init;
		ZeroMemory(&init, sizeof(init));
		init.ai_family = AF_INET;
		init.ai_socktype = SOCK_STREAM;
		init.ai_protocol = IPPROTO_TCP;
		init.ai_flags = AI_PASSIVE;
		struct addrinfo * addrinfoList = sh.hatchAddrinfo(NULL, port, init);
		//sh.printAddrinfo(addrinfoList);
		serverSocket = sh.bindSocket(addrinfoList);
		freeaddrinfo(addrinfoList);
		sh.listenOnSocket(serverSocket);
	}
	catch (std::exception & ex)
	{
		printf("\n%s", ex.what());
	}
}

void ServerClass::threadManageConnections(ServerClass * servClass)
{
	servClass->manageConnections();
}

void ServerClass::manageConnections()
{
	SocketHelper sh;
	int i = 1;
	for (;;)
	{
		SOCKET clientSocket = sh.acceptSocket(serverSocket);
		printf("\nClient %d connected!", i);
		std::shared_ptr<User> user = std::shared_ptr<User>(new User);
		user->socket = clientSocket;
		user->id = i;
		user->room = NULL;
		user->left = false;

		// starting thread which will serve connected player
		std::thread userThread(&ServerClass::threadPlayer, *this, this, user);
		userThread.detach(); // we are not waiting for thread to end

		i++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////
// HANDLING CLIENT

void ServerClass::threadPlayer(ServerClass * servClass, std::shared_ptr<User> player)
{
	servClass->managePlayer(player);
}

void ServerClass::managePlayer(std::shared_ptr<User> player)
{
	SocketHelper sh;
	char recvbuff[DEFAULT_BUFLEN];

	for (;;)
	{
		if (player->left) break;
		memset(recvbuff, '\0', sizeof(char) * DEFAULT_BUFLEN);
		try
		{
			sh.recvFromSocket(player->socket, recvbuff, DEFAULT_BUFLEN);
		}
		catch (std::exception & ex)
		{
			printf("\nCLIENT %i: %s", player->id ,ex.what());
			eraseUser(player);
			closesocket(player->socket);
			break;
		}
		std::string recvstr(recvbuff);
		readMsgAndAct(player, recvstr);
	}
}

void ServerClass::readMsgAndAct(std::shared_ptr<User> player, const std::string & msg)
{
	// we assume msg passed to this function is formatted with protocol rules

	// extracting code
	std::string code = msg.substr(1,3);
	std::string objects = msg.substr(4,msg.size()-5);
	//printf("\nClient %i send: %s", player->id, msg.c_str());
	if(code == "000") // login
	{
		if (player->serverStatus == 0)
		{
			std::string pom;
			std::string nick;
			HelpFunctions::getObjectAt(objects, 0, pom);
			HelpFunctions::getMemberAt(pom, 0, nick);
			player->nick = nick;
			printf("\nClient %i authenticated - nick: %s", player->id, player->nick.c_str());
			player->serverStatus = 1;
			userLock.lock();
			users.push_back(player);
			userLock.unlock();
		}
	}
	else if (code == "001") // map 
	{
		if (player->serverStatus == 2)
		{
			auto room = player->room;
			if (room->player1 == player.get())
			{
				std::string obj;
				HelpFunctions::getObjectAt(objects, 0, obj);
				std::string member1;
				HelpFunctions::getMemberAt(obj, 0, member1);
				std::vector<std::vector<unsigned int>> map = HelpFunctions::strToMap(member1, 4, 4);
				if (room->gameStatus == 1)
				{
					room->battle.setPlayerMap(0, map);
					room->gameStatus = 3;
					printf("\nReceived map from Client %i - player1", player->id);
				}
				else if(room->gameStatus == 2)
				{
					room->battle.setPlayerMap(0, map);
					room->gameStatus = 4; // game start
					printf("\nReceived map from Client %i - player1", player->id);
					std::cout << room->battle;
				}
			}
			else if (room->player2 == player.get())
			{
				std::string obj;
				HelpFunctions::getObjectAt(objects, 0, obj);
				std::string member1;
				HelpFunctions::getMemberAt(obj, 0, member1);
				std::vector<std::vector<unsigned int>> map = HelpFunctions::strToMap(member1, 4, 4);
				if (room->gameStatus == 1)
				{
					room->battle.setPlayerMap(1, map);
					room->gameStatus = 2;
					printf("\nReceived map from Client %i - player2", player->id);
				}
				else if (room->gameStatus == 3)
				{
					room->battle.setPlayerMap(1, map);
					room->gameStatus = 4; // game start
					printf("\nReceived map from Client %i - player2", player->id);
					std::cout << room->battle;
				}
			}
		}
	}
	else if (code == "002") // ???
	{

	}
	else if (code == "003") // chat
	{
		if (player->serverStatus == 2 || player->serverStatus == 3)
		{
			player->room->sendMsgToAll(msg);
		}
	}
	else if (code == "004") // from client - commands
	{
		int i = 0;
		std::string obj;
		while (HelpFunctions::getObjectAt(objects, i, obj))
		{
			std::string member1;
			if (HelpFunctions::getMemberAt(obj, 0, member1))
			{
				if (member1 == "0")
				{
					if (player->serverStatus == 2)
					{
						auto room = player->room;
						std::string member2;
						HelpFunctions::getMemberAt(obj, 1, member2);
						std::string member3;
						HelpFunctions::getMemberAt(obj, 2, member3);
						int n = std::stoi(member2);
						int m = std::stoi(member3);
						bool changed = false;
						if (room->player1 == player.get() && room->gameStatus == 4)
						{
							changed = true;
							if (room->battle.manageShot(n, m, 1))
							{
								printf("\nClient %i issued shot on %i-%i and HIT", player->id, n, m);
								if (room->battle.haveLost(1))
								{
									printf("\nClient %i has won the game!", player->id);
									room->gameStatus = 1;
								}
							}
							else
								printf("\nClient %i issued shot on %i-%i and MISSED", player->id, n, m);
							room->gameStatus = 5;
						}
						else if (room->player2 == player.get() && room->gameStatus == 5)
						{
							changed = true;
							if (room->battle.manageShot(n,m,0))
							{
								printf("\nClient %i issued shot on %i-%i and HIT", player->id, n, m);
								if (room->battle.haveLost(0))
								{
									printf("\nClient %i has won the game!", player->id);
									room->gameStatus = 1;
								}
							}
							else
								printf("\nClient %i issued shot on %i-%i and MISSED", player->id, n, m);
							room->gameStatus = 4;
						}
						if (changed) // send new state of maps to clients
						{
							std::cout << room->battle;
							std::string msgMaps = "[001{" + HelpFunctions::mapToStr(room->battle.getPlayerMap(0)) + "}" +
												  "{" + HelpFunctions::mapToStr(room->battle.getPlayerMap(1)) + "}]";
							room->sendMsgToAll(msgMaps);
						}
					}
				}
				else if (member1 == "1") // sending players/rooms to client
				{
					
				}
				else if (member1 == "2") // creating new room
				{
					if (player->serverStatus == 1)
					{
						std::string member2;
						HelpFunctions::getMemberAt(obj, 1, member2);
						std::shared_ptr<GameRoom> newRoom = std::shared_ptr<GameRoom>(new GameRoom);
						roomAmount++;
						newRoom->battle = Battleship();
						newRoom->isPublic = std::stoi(member2);
						newRoom->player1 = player.get();
						newRoom->player2 = NULL;
						player->room = newRoom;
						player->serverStatus = 2;
						printf("\nClient %i created new room! amount of rooms: %i", player->id, roomAmount);
					}
				}
				else if (member1 == "3") // {joinroom id player/observator permission}
				{
					std::string member2;
					HelpFunctions::getMemberAt(obj, 1, member2);
					std::string member3;
					HelpFunctions::getMemberAt(obj, 2, member3);
					std::string member4;
					HelpFunctions::getMemberAt(obj, 3, member4);
					printf("\n%s %s %s",member2.c_str(),member3.c_str(),member4.c_str());
					int id = std::stoi(member2);
					int mode = std::stoi(member3);
					int permission = std::stoi(member4);
					int i = 0;
					bool found = false;
					while (i<users.size() && !found)
					{
						if (users[i]->id == id)
						{
							found = true;
							if (users[i]->room != NULL)
							{
								if (users[i]->room->isPublic == 1 || (users[i]->room->isPublic == 0 && permission == 1))
								{
									if (mode == 0)
									{
										if (users[i]->room->player2 == NULL)
										{
											users[i]->room->player2 = player.get();
											users[i]->room->gameStatus = 1;
											player->room = users[i]->room;
											player->serverStatus = 2;
											printf("\nClient %i joined to room as player2",player->id);
										}
									}
									else // as observer
									{
										obsLock.lock();
										users[i]->room->observators.push_back(player.get());
										obsLock.unlock();
										player->room = users[i]->room;
										player->serverStatus = 3;
										printf("\nClient %i joined room as observer",player->id);
									}
								}
							}
						}
					}
				}
				else if (member1 == "4") // client logout
				{
					printf("\nClient %i logged off!", player->id);
					eraseUser(player);
					player->left = true;
				}
				else if (member1 == "5") // client left room
				{
					printf("\nClient %i left room", player->id);
					clearUser(player);
				}
			}
			i++;
		}
	}
}

void ServerClass::clearRoom(std::shared_ptr<GameRoom> room)
{
	for (User* user : room->observators)
	{
		user->serverStatus = 1;
		user->room = NULL;
	}
	room->observators.clear();
	if (room->player2 != NULL)
	{
		room->player2->serverStatus = 1;
		room->player2->room = NULL;
	}
	room->player2 = NULL;
	if (room->player1 != NULL)
	{
		room->player1->serverStatus = 1;
		room->player1->room = NULL;
	}
	room->player1 = NULL;
}

void ServerClass::clearUser(std::shared_ptr<User> player)
{
	auto room = player->room;
	if (room != NULL)
	{
		if (room->player1 == player.get()) // player1
		{
			if (room->player2 != NULL)
			{
				room->gameStatus = 1;
				room->player1 = room->player2;
				room->player2 = NULL;
			}
			else // room will be destroyed
			{
				clearRoom(player->room);
			}
		}
		else if (room->player2 == player.get()) // player2
		{
			room->gameStatus = 1;
			room->player2 = NULL;
		}
		else // observator
		{
			int i = 0;

			//room->obsLock.lock();
			for (auto user : room->observators)
			{
				if (user == player.get())
				{
					room->observators.erase(room->observators.begin() + i);
					break;
				}
				i++;
			}
			//room->obsLock.unlock();
		}
		player->room = NULL;
		player->serverStatus = 1;
	}
}

void ServerClass::eraseUser(std::shared_ptr<User> player)
{
	// first clear the user (rearrange/close room if needed)
	clearUser(player);
	userLock.lock();
	for (int i = 0; i < users.size(); i++)
	{
		if (player == users[i])
		{
			users.erase(users.begin() + i);
			break;
		}
	}
	userLock.unlock();
}

//////////////////////////////////////////////////////////////////////////////////////
//

void ServerClass::init()
{
	startNetcode("27015");
	printf("SERVER WORKING...");
	std::thread connectionThread(&ServerClass::threadManageConnections,*this,this);
	connectionThread.join();
}

std::string HelpFunctions::mapToStr(const std::vector<std::vector<unsigned int>>& map)
{
	std::string str = "";
	for (int i = 0; i < map.size(); i++)
	{
		for (int j = 0; j < map[i].size(); j++)
		{
			str += std::to_string(map[i][j]);
		}
	}
	return str;
}

std::vector<std::vector<unsigned int>> HelpFunctions::strToMap(const std::string & str, int sizeX, int sizeY)
{
	std::vector<std::vector<unsigned int>> map;

	for (int i = 0; i < sizeX; i++)
	{
		std::vector<unsigned int> row;
		for (int j = 0; j < sizeY; j++)
		{
			row.push_back(std::stoi(str.substr(i*sizeY+j,1)));
		}
		map.push_back(row);
	}

	return map;
}

bool HelpFunctions::getObjectAt(const std::string & str, unsigned int pos, std::string & object)
{
	if (str.size() == 0)
		return false;
	object = "";

	int count = 0;
	int j = 1;
	int i = 1;
	while (i <= str.size() - 1)
	{
		j = i;
		while (str[i] != '}')
		{
			i++;
		}
		if (pos == count)
		{
			object = str.substr(j,i-j);
			return true;
		}
		count++;
		i += 2;
	}
	return false;
}

bool HelpFunctions::getMemberAt(const std::string & object, unsigned int pos, std::string & member)
{
	if (object.size() == 0)
		return false;
	member = "";

	int count = 0;
	int j = 0;
	int i = 0;
	while (i <= object.size() - 1)
	{
		j = i;
		while (object[i] != ',' && i<=object.size()-1)
		{
			i++;
		}
		if (pos == count)
		{
			member = object.substr(j, i - j);
			return true;
		}
		count++;
		i += 1;
	}
	return false;
}

/*std::string HelpFunctions::userChatToStringMsg(const std::string & chatmsg, User* user)
{
	std::string str = "[003";
}

std::string User::toString()
{
	std::string str;
	str += "{"+std::to_string(id)+","+nick+"}";
}*/

void User::sendMsg(const std::string & msg)
{
	char sendbuff[DEFAULT_BUFLEN];
	memset(sendbuff, '\0', sizeof(char) * DEFAULT_BUFLEN);
	strcpy(sendbuff, msg.c_str());

	SocketHelper sh;
	sh.sendToSocket(socket,sendbuff);
}

void GameRoom::sendMsgToAll(const std::string & msg)
{
	if (player1 != NULL)
		player1->sendMsg(msg);
	if (player2 != NULL)
		player2->sendMsg(msg);
	for (auto var : observators)
	{
		var->sendMsg(msg);
	}
}
