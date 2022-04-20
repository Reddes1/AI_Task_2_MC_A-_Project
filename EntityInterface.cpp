#include "EntityInterface.h"

EntityInterface::EntityInterface(std::shared_ptr<SpriteTexture> texture)
{
	assert(texture);
	
	//Set Sprite with texture
	m_PrimarySprite.SetTexturePtr(texture, true);
}

void EntityInterface::DecrementXCoordinate(bool clampToZero)
{
	--m_MapCoordinates.x;
	if (clampToZero && m_MapCoordinates.x < 0)
		m_MapCoordinates.x = 0;
}

void EntityInterface::DecrementYCoordinate(bool clampToZero)
{
	--m_MapCoordinates.y;
	if (clampToZero && m_MapCoordinates.y < 0)
		m_MapCoordinates.y = 0;
}

void EntityInterface::IncrementXCoordinate(bool clampToLimit)
{
	++m_MapCoordinates.x;
	if (clampToLimit && m_MapCoordinates.x > m_MapCoordinateLimit.x)
		m_MapCoordinates.x = m_MapCoordinateLimit.x;

}

void EntityInterface::IncrementYCoordinate(bool clampToLimit)
{
	++m_MapCoordinates.y;
	if (clampToLimit && m_MapCoordinates.y > m_MapCoordinateLimit.y)
		m_MapCoordinates.y = m_MapCoordinateLimit.y;

}

void EntityInterface::Add(std::vector<EntityInterface*>& container, EntityInterface* object)
{
	assert(&container);
	assert(object);
	container.push_back(object);
}

void EntityInterface::ReleaseContainer(std::vector<EntityInterface*>& container)
{
	for (auto& a : container) 
	{
		delete a;
		a = nullptr;
	}
	container.clear();
}
