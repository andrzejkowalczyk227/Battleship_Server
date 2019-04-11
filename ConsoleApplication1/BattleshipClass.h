#pragma once

#include <vector>
#include <iostream>

class Battleship
{
// 0 - water, 1 - boat, 2 - hit, 3 - miss
private:
	std::vector<std::vector<std::vector<unsigned int>>> playersMaps;
	//std::vector<unsigned int> ships;
	int xSize, ySize;
	std::vector<int> playersHp;

public:
	Battleship();

	void setPlayerMap(unsigned int iPlayer, const std::vector<std::vector<unsigned int>> & map);
	const std::vector<std::vector<unsigned int>> & getPlayerMap(unsigned int iPlayer);

	bool haveLost(unsigned int i);
	bool manageShot(unsigned int i, unsigned int y, unsigned int player);

	friend std::ostream & operator<<(std::ostream & os, const Battleship & bs);
};