#pragma once

#include "D3DUtils.h"

/*
	Class manages the the effect of a 2D camera by managing a matrix transformation to move a scene.
	Using tilesize to align with fixed cursor movement, maintain an x/y coordinate system for the world
	and calculate a transformation for the scene to move.
*/
class FixedDist2DCamera
{
public:

	FixedDist2DCamera() {}
	~FixedDist2DCamera() {}

	//Setup camera initial values and starting position
	void SetupCamera(DirectX::XMINT2& cursorScreenOffset, DirectX::XMINT2& mapMin,
		DirectX::XMINT2& mapMax, DirectX::XMINT2& startingCoords, DirectX::XMINT2& tileSize);

	//If bool flagged, will update transform for next call
	void UpdateCamera();

	void MoveSceneUp();
	void MoveSceneDown();
	void MoveSceneLeft();
	void MoveSceneRight();

	void MoveCameraToCoordinates(DirectX::XMINT2& newCoords);

	DirectX::XMMATRIX GetSceneTransform() { return m_SceneTransform; }

private:

	//Force update scene transform (is naturally called on each coordinate change regardless)
	void UpdateSceneTransform();

	//Scene Manipulation Matrix
	DirectX::XMMATRIX m_SceneTransform = DirectX::XMMatrixIdentity();
	DirectX::XMINT2 m_Translation = { 0, 0 };

	DirectX::XMINT2 m_Coordinates = { 0, 0 };
	DirectX::XMINT2 m_CoordinateMin = { 0, 0 };
	DirectX::XMINT2 m_CoordinateMax = { 0, 0 };
	DirectX::XMINT2 m_TileSize = { 0, 0 };
	DirectX::XMINT2 m_CursorOffset = { 0, 0 };
	bool m_UpdateFlag = false;
};