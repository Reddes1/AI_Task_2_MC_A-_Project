#pragma once

/*
	Specific Types for this game.
*/

//Holds specialised information regarding a game tiles properties 
struct TileProperties
{
	TileProperties()
		:tileName("Tile"), tileID(-1), moveCost(-1), terrainTypeID(99),
		impassable(false), occupied(false)
	{}
	std::string tileName;
	int tileID;
	int terrainTypeID;
	float moveCost;
	bool impassable : 1;
	bool occupied : 1;
};