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
