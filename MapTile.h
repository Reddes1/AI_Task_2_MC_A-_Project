#pragma once

#include "EntityInterface.h"
#include "GameTypes.h"			//Tile Properties Container

const int NUM_OF_NEIGHBOURS = 4;

class MapTile : public EntityInterface
{
public:

	MapTile() {}
	//Setup with 1 texture (applies pointer to both sprites)
	MapTile(std::shared_ptr<SpriteTexture> mainTex);
	//Setup with 2 textures (1st for main tile tex, 2nd for grid texture)
	MapTile(std::shared_ptr<SpriteTexture> mainTex, std::shared_ptr<SpriteTexture> gridTex);
	~MapTile() {}

	/////////////////
	/// Overrides ///
	/////////////////

	void Update(const GameTimer& gt);
	void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap);

	//Optional Overides (Uncomment as needed)
	//void ResetEntityToDefault() { }

	///////////
	/// Set ///
	///////////

	//Set all tile properties at once
	void SetTileProperties(TileProperties& newProperties) { m_Properties = newProperties; }
	//0 = North, 1 = East, 2 = South, 3 = West
	void SetNeighbourAtIndex(int index, MapTile* neighbour) { m_Pointers[index] = neighbour; }
	//Set GridEnabled state
	void SetDrawGridFlag(bool enableGrid) { m_EnableGridDraw = enableGrid; }


	///////////
	/// Get ///
	///////////

	TileProperties& GetTileProperties() { return m_Properties; }
	//0 = North, 1 = East, 2 = South, 3 = West
	MapTile* GetNeighbourAtIndex(int index) { return m_Pointers[index]; }

	Sprite& GetGridSprite() { return m_GridSprite; }

	std::string GetTileTypeAsString();

	//////////////////
	/// Operations ///
	//////////////////

	void SetOccupationStatus(bool status) { m_Properties.occupied = status; }
	//Mirrors relevant data from the tile to the grid for positional requirements
	void MirrorTileToGridData();


	/*
		Wraps up the math required to find a tile in a given container of just tiles
		using coordinates and max row size.
	*/
	static MapTile* FindTileInArray(std::vector<MapTile*>& container, DirectX::XMINT2& coords, int rowSize);


private:

	//Tile Grid Effect Sprite
	Sprite m_GridSprite;
	//Controlling flag for both update and draws for grid sprite
	bool m_EnableGridDraw = false;

	//0 = North, 1 = East, 2 = South, 3 = West
	MapTile* m_Pointers[4] = { nullptr, nullptr, nullptr, nullptr };
	TileProperties m_Properties;

};
