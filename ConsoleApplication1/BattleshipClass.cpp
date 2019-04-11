#include "stdafx.h"
#include "BattleshipClass.h"

Battleship::Battleship() : xSize(4), ySize(4)
{
	playersMaps.push_back(std::vector<std::vector<unsigned int>>());
	playersMaps.push_back(std::vector<std::vector<unsigned int>>());
	playersHp.push_back(0);
	playersHp.push_back(0);
}

/*void Battleship::setShips(const std::vector<unsigned int> & _ships)
{
	ships = _ships;
}*/

void Battleship::setPlayerMap(unsigned int iPlayer, const std::vector<std::vector<unsigned int>>& map)
{
	playersMaps[iPlayer] = map;
	int sum = 0;
	for (int i = 0; i < xSize; i++)
	{
		for (int j = 0; j < ySize; j++)
		{
			if (map[i][j] == 1) sum++;
		}
	}
	playersHp[iPlayer] = sum;
}

const std::vector<std::vector<unsigned int>>& Battleship::getPlayerMap(unsigned int iPlayer)
{
	return playersMaps[iPlayer];
}

/*void Battleship::buildMaps(unsigned int nPlayers)
{
	for (int p = 0; p < nPlayers; p++)
	{
		std::vector<std::vector<unsigned int>> newPlayerMap;
		for (int i = 0; i < xSize; i++)
		{
			newPlayerMap.push_back(std::vector<unsigned int>(ySize, 0));
		}
		playersMaps.push_back(newPlayerMap);
	}
}*/

bool Battleship::haveLost(unsigned int i)
{
	if (playersHp[i] == 0)
		return true;
	return false;
}

bool Battleship::manageShot(unsigned int n, unsigned int m, unsigned int player)
{
	if (playersMaps[player][n][m] == 1)
	{
		playersMaps[player][n][m] = 2;
		playersHp[player]--;
		return true;
	}
	playersMaps[player][n][m] = 3;
	return false;
}

std::ostream & operator<<(std::ostream & os, const Battleship & bs)
{
	os << "\n";
	for (int i = 0; i < bs.xSize; i++)
	{
		for (int p = 0; p < bs.playersMaps.size(); p++)
		{
			for (int j = 0; j < bs.ySize; j++)
			{
				os << bs.playersMaps[p][i][j] << " ";
			}
			if(p<bs.playersMaps.size()-1) os << "  |   ";
		}
		os << std::endl;
	}
	return os;
}
