#pragma once

#include "D3DUtils.h"
#include "MapTile.h"
#include <set>

/*
	Diamond style radius targetting system. For use with the MapTile object
*/
class DiamondRadiusTargeting
{
public:

	DiamondRadiusTargeting();
	~DiamondRadiusTargeting();
	
	///////////
	/// Set ///
	///////////

	void SetTileOverlayFrameIndex(int index) { m_FrameIndex = index; }
	void SetTileMapLength(int mapRowLength) { m_MapRowLength = mapRowLength; }

	///////////
	/// Get ///
	///////////

	
	//////////////////
	/// Operations ///
	//////////////////

	//Target grid operations

	void GenerateTargetGrid(const std::vector<MapTile*>& tiles, const DirectX::XMINT2& startCoords, int range);
	//Disables tile effect off on each member of the manifest and releases them
	void DisableGrid();
	//Check by coordinate if the target is inside of the grid
	bool IsTargetInGrid(const DirectX::XMINT2& unitCoords);

	//AoE grid operations
	void GenerateAoEGrid(const std::vector<MapTile*>& tiles, const DirectX::XMINT2& cursorCoords, int radius);
	void DisableAoEGrid();
	bool IsUnitInAoEGrid(const DirectX::XMINT2& unitCoords);
private:
	//Track the currently stored coordinates
	std::set<MapTile*> m_TileManifest;
	std::set<MapTile*> m_AoeTileManifest;

	//Hold tile overlay frame index here
	int m_FrameIndex = 0;
	//Hold on to the tile map length for container searches
	int m_MapRowLength = 0;
};