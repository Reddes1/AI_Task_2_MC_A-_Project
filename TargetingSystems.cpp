#include "TargetingSystems.h"

using namespace DirectX;

DiamondRadiusTargeting::DiamondRadiusTargeting()
{
}

DiamondRadiusTargeting::~DiamondRadiusTargeting()
{

}

void DiamondRadiusTargeting::GenerateTargetGrid(const std::vector<MapTile*>& tiles, const XMINT2& startCoords, int range)
{
	
	for (int i(0); i < range + 1; ++i)
	{
		//Topside + Origin row
		for (int j = -i - 1; j < i; ++j)
		{
			//Grab tile at index
			int xAdj = (startCoords.x) - j - 1;
			int yAdj = startCoords.y - range + i;
			int index = (xAdj + yAdj) + (yAdj * m_MapRowLength);

			//Prevent invalid index access and drawing on tiles that would be drawn due to screen wrapping.
			if (!(index < 0 || index > tiles.size()) && !(xAdj < 0 || xAdj > m_MapRowLength))
			{
				MapTile* tile = tiles.at(index);
				tile->SetDrawGridFlag(true);
				tile->GetGridSprite().SetFrame(m_FrameIndex);
				m_TileManifest.insert(tile);
			}
		}
		//Botside
		for (int j = range - 1 - i; j > -range + i; --j)
		{
			//Grab tile at index
			int xAdj = startCoords.x - j;
			int yAdj = startCoords.y + i + 1;
			int index = (xAdj + yAdj) + (yAdj * m_MapRowLength);

			//Prevent invalid index access and drawing on tiles that would be drawn due to screen wrapping.
			if (!(index < 0 || index > tiles.size()) && !(xAdj < 0 || xAdj > m_MapRowLength))
			{
				MapTile* tile = tiles.at(index);
				tile->SetDrawGridFlag(true);
				tile->GetGridSprite().SetFrame(m_FrameIndex);
				m_TileManifest.insert(tile);
			}
		}
	}

}

void DiamondRadiusTargeting::DisableGrid()
{
	for (auto& a : m_TileManifest)
	{
		a->SetDrawGridFlag(false);
	}

	m_TileManifest.clear();
}

bool DiamondRadiusTargeting::IsTargetInGrid(const XMINT2& unitCoords)
{
	//Check each member in the manifest and search for coordinate match
	for (auto& a : m_TileManifest)
	{
		if (a->GetMapCoordinates().x == unitCoords.x && a->GetMapCoordinates().y == unitCoords.y)
			return true;
	}	
	return false;
}

void DiamondRadiusTargeting::GenerateAoEGrid(const std::vector<MapTile*>& tiles, const DirectX::XMINT2& cursorCoords, int radius)
{
	for (int i(0); i < radius + 1; ++i)
	{
		//Topside + Origin row
		for (int j = -i - 1; j < i; ++j)
		{
			//Grab tile at index
			int xAdj = (cursorCoords.x) - j - 1;
			int yAdj = cursorCoords.y - radius + i;
			int index = (xAdj + yAdj) + (yAdj * m_MapRowLength);

			//Prevent invalid index access and drawing on tiles that would be drawn due to screen wrapping.
			if (!(index < 0 || index > tiles.size()) && !(xAdj < 0 || xAdj > m_MapRowLength))
			{
				MapTile* tile = tiles.at(index);
				tile->SetDrawGridFlag(false);
				tile->GetGridSprite().SetFrame(m_FrameIndex);
				m_AoeTileManifest.insert(tile);
			}
		}
		//Botside
		for (int j = radius - 1 - i; j > -radius + i; --j)
		{
			//Grab tile at index
			int xAdj = cursorCoords.x - j;
			int yAdj = cursorCoords.y + i + 1;
			int index = (xAdj + yAdj) + (yAdj * m_MapRowLength);

			//Prevent invalid index access and drawing on tiles that would be drawn due to screen wrapping.
			if (!(index < 0 || index > tiles.size()) && !(xAdj < 0 || xAdj > m_MapRowLength))
			{
				MapTile* tile = tiles.at(index);
				tile->SetDrawGridFlag(false);
				tile->GetGridSprite().SetFrame(m_FrameIndex);
				m_AoeTileManifest.insert(tile);
			}
		}
	}
}

void DiamondRadiusTargeting::DisableAoEGrid()
{
	m_AoeTileManifest.clear();
}

bool DiamondRadiusTargeting::IsUnitInAoEGrid(const DirectX::XMINT2& unitCoords)
{
	//Check each member in the manifest and search for coordinate match
	for (auto& a : m_AoeTileManifest)
	{
		if (a->GetMapCoordinates().x == unitCoords.x && a->GetMapCoordinates().y == unitCoords.y)
			return true;
	}
	return false;
}
