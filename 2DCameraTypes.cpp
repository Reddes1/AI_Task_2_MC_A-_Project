#include "2DCameraTypes.h"

using namespace DirectX;

void FixedDist2DCamera::SetupCamera(DirectX::XMINT2& cursorScreenOffset, DirectX::XMINT2& mapMin, DirectX::XMINT2& mapMax, DirectX::XMINT2& startingCoords, DirectX::XMINT2& tileSize)
{
	//Setup map Min/Max coordinates
	m_CoordinateMin.x = (0 + cursorScreenOffset.x);
	m_CoordinateMin.y = (0 + cursorScreenOffset.y);
	m_CoordinateMax.x = -mapMax.x + cursorScreenOffset.x;
	m_CoordinateMax.y = -mapMax.y + cursorScreenOffset.y;
	//Grab offset for later math
	m_CursorOffset = cursorScreenOffset;
	//Sync the map coordinate using minimum value (top left coordinate) - the starting value
	m_Coordinates.x = (m_CoordinateMin.x - startingCoords.x);
	m_Coordinates.y = (m_CoordinateMin.y - startingCoords.y);
	//Set tilesize for calculations
	m_TileSize = tileSize;

	//Push update
	UpdateSceneTransform();
}

void FixedDist2DCamera::UpdateSceneTransform()
{
	m_Translation = { m_TileSize.x * m_Coordinates.x, m_TileSize.y * m_Coordinates.y };
	m_SceneTransform = XMMatrixTranslation(m_Translation.x, m_Translation.y, 0.0f);
	m_UpdateFlag = false;
}

void FixedDist2DCamera::UpdateCamera()
{
	if (m_UpdateFlag)
		UpdateSceneTransform();
}

void FixedDist2DCamera::MoveSceneUp()
{
	//Update value
	--m_Coordinates.y;
	//Check if out of range
	if (m_Coordinates.y < m_CoordinateMax.y)
		m_Coordinates.y = m_CoordinateMax.y;
	//Flag for update
	m_UpdateFlag = true;
}

void FixedDist2DCamera::MoveSceneDown()
{
	//Update value
	++m_Coordinates.y;
	//Check if out of range
	if (m_Coordinates.y > m_CoordinateMin.y)
		m_Coordinates.y = m_CoordinateMin.y;
	//Flag for update
	m_UpdateFlag = true;
}

void FixedDist2DCamera::MoveSceneLeft()
{
	//Update value
	--m_Coordinates.x;
	//Check if out of range
	if (m_Coordinates.x < m_CoordinateMax.x)
		m_Coordinates.x = m_CoordinateMax.x;
	//Flag for update
	m_UpdateFlag = true;
}

void FixedDist2DCamera::MoveSceneRight()
{
	//Update value
	++m_Coordinates.x;
	//Check if out of range
	if (m_Coordinates.x > m_CoordinateMin.x)
		m_Coordinates.x = m_CoordinateMin.x;
	//Flag for update
	m_UpdateFlag = true;
}

void FixedDist2DCamera::MoveCameraToCoordinates(DirectX::XMINT2& newCoords)
{
	m_Coordinates.x = m_CoordinateMin.x - newCoords.x;
	m_Coordinates.y = m_CoordinateMin.y - newCoords.y;
	//Flag for update
	m_UpdateFlag = true;
}
