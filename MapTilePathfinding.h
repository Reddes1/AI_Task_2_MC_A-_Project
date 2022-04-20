#pragma once
#include <set>

#include "MapTile.h"		//Object method uses
#include "UnitEntity.h"		//Game Unit

//Supporting type for use in the pathfinding algorithm
struct Node
{
	Node() {}
	~Node() { tile = nullptr; }

	MapTile* tile = nullptr;
	float remainingCost = -1;

	//Comparison operator
	bool operator <(const Node& pt) const
	{
		DirectX::XMINT2 coords = tile->GetMapCoordinates();
		DirectX::XMINT2 lhCoords = pt.tile->GetMapCoordinates();
		return (coords.x < lhCoords.x) || ((!(lhCoords.x < coords.x)) && (coords.y < lhCoords.y));
	}
};

class MapTilePathfinder
{
public:

	MapTilePathfinder();
	~MapTilePathfinder() { m_GridManifest.clear(); }

	///////////
	/// Get ///
	///////////

	std::set<MapTile*>& GetManifest() { return m_GridManifest; }

	//////////////////
	/// Operations ///
	//////////////////

	void GenerateTileGrid(std::vector<MapTile*>& tileContainer, UnitEntity* unit, int mapRowLength);

	//Check to see if this tile is in the grid
	bool IsTileInGrid(const MapTile* tile);
	
	//Releases hold of all tiles currently highlighted.
	//Calls a release policy on the nodes to clear the grid effect before release.
	void ReleaseManifest();
	
private:

	///////////////////////////
	/// Validation Policies ///
	///////////////////////////

	bool MoveValidationPolicy(const Node& node, MapTile* neighbour, UnitEntity* unit);
	void NodeValidation(const Node& node, std::set<Node>& container, int index);


	/////////////////////////////
	/// Apply Effect Policies ///
	/////////////////////////////

	void ApplyGridEffect(MapTile* tile);
	void RemoveGridEffect(MapTile* tile);


	////////////
	/// Data ///
	////////////

	std::set<MapTile*> m_GridManifest;

};
