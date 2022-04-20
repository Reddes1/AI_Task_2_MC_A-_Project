#pragma once

#include "EntityInterface.h"
#include "GameUtilities.h"

#include "MapTile.h"		//For search algorithm
#include "UnitEntity.h"

/*
	Specialised class for representing a game unit. Setup for specific navigation and interact with the world space.
*/

enum CURSOR_DIR
{
	UP,	DOWN, LEFT,	RIGHT
};
enum class UNIT_HOVER_TYPE
{
	NONE, FRIENDLY, ENEMY
};


class CursorEntity : public EntityInterface
{
public:

	CursorEntity() { Init(); }
	CursorEntity(std::shared_ptr<SpriteTexture> texture);
	~CursorEntity() {}

	/////////////////
	/// Overrides ///
	/////////////////

	void Update(const GameTimer& gt);
	void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap)
	{
		EntityInterface::Render(batch, heap);
	}

	///////////
	/// Set ///
	///////////

	void SetTileSize(DirectX::XMINT2 dist) { m_TileSize = dist; }
	void SetTileSize(int32_t x, int32_t y) { m_TileSize.x = x; m_TileSize.y = y; }

	///////////
	/// Get ///
	///////////

	EntityInterface* const GetCurrentObject() { return m_Object; }
	MapTile* const GetCurrentTileObject() { return m_TileObject; }
	UNIT_HOVER_TYPE& const GetUnitIDType() { return m_HoverTargetID; }
	EntityInterface* const GetSecondObject() { return m_2ndTarget; }

	//////////////////
	/// Operations ///
	//////////////////

	//Signal what direction to move the cursor
	void Move(CURSOR_DIR dir);
	//Move cursor coordinates (Doesn't move the visual cursor so sync with camera)
	void MoveFixedCursorCoords(DirectX::XMINT2& newCoords) { m_MapCoordinates = newCoords; }

	//Search a given container for coordinate matching object and store pointer to it
	bool SearchForUnitObject(std::vector<EntityInterface*>& container, int rowLength);
	//Search for a given container for coordinate matching enemy object and store pointer to it
	bool SearchForSecondUnitObject(std::vector<EntityInterface*>& container, int rowLength);
	//Search a given container for matching tile object and store pointer to it
	bool SearchForTileObject(std::vector<MapTile*>& container, int rowLength);

	//Optional Overides (Uncomment as needed)
	//void ResetEntityToDefault() { }

private:

	void Init();

	//Dictates how much the cursor moves on each move event
	DirectX::XMINT2 m_TileSize = { 0, 0 };
	//Hold information if the cursor is hovering a unit
	UNIT_HOVER_TYPE m_HoverTargetID;
	//Convenient pointer to currently hovered object
	EntityInterface* m_Object = nullptr;
	EntityInterface* m_2ndTarget = nullptr;
	//Convenient pointer to currently hovered tile
	MapTile* m_TileObject = nullptr;

};