#include "CursorEntity.h"

using namespace DirectX;

CursorEntity::CursorEntity(std::shared_ptr<SpriteTexture> texture)
	:EntityInterface(texture)
{

}

void CursorEntity::Update(const GameTimer& gt)
{

}

void CursorEntity::Move(CURSOR_DIR dir)
{
	switch (dir)
	{
	case CURSOR_DIR::UP:
		--m_MapCoordinates.y;
		if (m_MapCoordinates.y < 0)
		{
			m_MapCoordinates.y = 0;
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		else
		{
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		break;

	case CURSOR_DIR::DOWN:
		++m_MapCoordinates.y;
		if (m_MapCoordinates.y > m_MapCoordinateLimit.y)
		{
			m_MapCoordinates.y = m_MapCoordinateLimit.y;
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		else
		{
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		break;
	case CURSOR_DIR::LEFT:
		--m_MapCoordinates.x;
		if (m_MapCoordinates.x < 0)
		{
			m_MapCoordinates.x = 0;
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		else
		{
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		break;
	case CURSOR_DIR::RIGHT:
		++m_MapCoordinates.x;
		if (m_MapCoordinates.x > m_MapCoordinateLimit.x)
		{
			m_MapCoordinates.x = m_MapCoordinateLimit.x;
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		else
		{
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		break;
	}
}

bool CursorEntity::SearchForUnitObject(std::vector<EntityInterface*>& container, int rowLength)
{
	for (auto& a : container)
	{
		XMINT2& coords = a->GetMapCoordinates();
		//If cursor matches coordinates of object
		if ((coords.x == m_MapCoordinates.x) && (coords.y == m_MapCoordinates.y))
		{
			m_Object = a;
			return true;
		}
	}
	//No matches so cursor is likely not hovering a unit anymore so clear pointer.
	m_Object = nullptr;
	return false;
}

bool CursorEntity::SearchForSecondUnitObject(std::vector<EntityInterface*>& container, int rowLength)
{
	for (auto& a : container)
	{
		XMINT2& coords = a->GetMapCoordinates();
		//If cursor matches coordinates of object
		if ((coords.x == m_MapCoordinates.x) && (coords.y == m_MapCoordinates.y))
		{
			m_2ndTarget = a;
			return true;
		}
	}
	//No matches so cursor is likely not hovering a unit anymore so clear pointer.
	m_2ndTarget = nullptr;
	return false;
}

bool CursorEntity::SearchForTileObject(std::vector<MapTile*>& container, int rowLength)
{
	//Find tile using map coordinates (tile position needs to be accurate to its position in the container, i.e 0,5 = index 4 in array)
	m_TileObject = container.at(static_cast<size_t>((m_MapCoordinates.x + m_MapCoordinates.y) + (m_MapCoordinates.y * rowLength)));

	//Extra errorchecking
	DBOUT(std::to_string(m_TileObject->GetMapCoordinates().x) << ", " << std::to_string(m_TileObject->GetMapCoordinates().y));
	//If coordinates match, all good
	if ((m_TileObject->GetMapCoordinates().x == m_MapCoordinates.x) && (m_TileObject->GetMapCoordinates().y == m_MapCoordinates.y))
		return true;
	else
		return false;
}

void CursorEntity::Init()
{

}
