#include "MapTile.h"

MapTile::MapTile(std::shared_ptr<SpriteTexture> mainTex)
	:EntityInterface(mainTex)
{
	m_GridSprite.SetTexturePtr(mainTex);
}

MapTile::MapTile(std::shared_ptr<SpriteTexture> mainTex, std::shared_ptr<SpriteTexture> gridTex)
	:EntityInterface(mainTex)
{
	m_GridSprite.SetTexturePtr(gridTex);
}

void MapTile::Update(const GameTimer& gt)
{
	//If the grid animates, nest in here
	if (m_EnableGridDraw)
	{

	}
}

void MapTile::Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap)
{
	//Render main sprite
	EntityInterface::Render(batch, heap);
	//Conditional draw for grid (if enabled)
	if (m_EnableGridDraw)
		m_GridSprite.Draw(batch, heap);
}

std::string MapTile::GetTileTypeAsString()
{
	return std::string();
}

void MapTile::MirrorTileToGridData()
{
	m_GridSprite.SetPosition(m_PrimarySprite.GetPosition());
}

MapTile* MapTile::FindTileInArray(std::vector<MapTile*>& container, DirectX::XMINT2& coords, int rowSize)
{
	return container.at((coords.x + coords.y) + coords.y * rowSize);
}

void MapTile::CalculateCosts(DirectX::XMINT2& origin, DirectX::XMINT2& target)
{
	//Calculate distance from origin

	//Get the X and Y distance difference
	float dist1 = origin.x - m_MapCoordinates.x;
	float dist2 = origin.y - m_MapCoordinates.y;

	//If any of these numbers are negative, flip them to postive for the algorithm
	IsNumberNegative(dist1);
	IsNumberNegative(dist2);

	m_GCost = (dist1 + dist2) * m_Properties.moveCost;


	//Calculate distance from target
	dist1 = target.x - m_MapCoordinates.x;
	dist2 = target.y - m_MapCoordinates.y;

	//If any of these numbers are negative, flip them to postive for the algorithm
	IsNumberNegative(dist1);
	IsNumberNegative(dist2);

	m_HCost = dist1 + dist2;


	//Calculate total cost
	m_FCost = m_GCost + m_HCost;
}
