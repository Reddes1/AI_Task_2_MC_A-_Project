#include "TilemapUtils.h"
#include <assert.h>

namespace TiledLoaders
{
	using namespace rapidjson;

	void GenerateFrameData(rapidjson::Document& doc, int tilesetIndex, std::vector<RECT>& container)
	{
		//Calculate number of rows and columns to align object RECTs properly.
		int tileCount = doc["tilesets"][tilesetIndex]["tilecount"].GetInt();
		int texColCount = doc["tilesets"][tilesetIndex]["columns"].GetInt();
		int texRowCount = tileCount / texColCount;

		//Get tile dimensions
		int tileWidth = doc["tilewidth"].GetInt();
		int tileHeight = doc["tileheight"].GetInt();

		//Generate frames (left->right, top->bottom pattern)
		for (int y(0); y < texRowCount; ++y)
		{
			for (int x(0); x < texColCount; ++x)
			{
				//Calc left, top corner
				long left = tileWidth * x;
				long top = tileHeight * y;
				//Calc right, bot corner
				RECT frame = { left, top, left + tileWidth,	top + tileHeight };
				container.push_back(frame);
			}
		}
	}

	float GetTileRotationValue(unsigned int& tileID)
	{
		//Return value
		const float PI = 3.1415926535f;

		//Isolate what transformations have been applied
		bool flipH = tileID & FLIPPED_HORIZONTALLY_FLAG;
		bool flipV = tileID & FLIPPED_VERTICALLY_FLAG;
		bool flipD = tileID & FLIPPED_DIAGONALLY_FLAG;

		//Strip flag values from the tileID
		tileID &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG | FLIPPED_DIAGONALLY_FLAG);

		//Do some checks and determine what rotation needs applying
		//Check if 90 Degree rotation
		if (flipH && flipD)
			return PI * 0.5f;
		//Check if 180 Degree rotation
		else if (flipH && flipV)
			return PI;
		//Check if 270 Degree rotation
		else if (flipV && flipD)
			return PI * 1.5f;

		//No triggers flags, so no rotation
		return 0.f;
	}

	void AssertBasicTilemapMembers(rapidjson::Document& doc)
	{
		//Tile Data
		assert(doc.HasMember("tilewidth"));
		assert(doc.HasMember("tileheight"));
		//Map Size Data
		assert(doc.HasMember("width"));
		assert(doc.HasMember("height"));
		//Sub Arrays
		assert(doc.HasMember("layers"));
		assert(doc.HasMember("tilesets"));

		//Check sub-array members
		//layers
		assert(doc["layers"][0].HasMember("data"));
		assert(doc["layers"][0].HasMember("height"));
		assert(doc["layers"][0].HasMember("id"));
		assert(doc["layers"][0].HasMember("width"));

		//tilesets
		assert(doc["tilesets"][0].HasMember("columns"));
		assert(doc["tilesets"][0].HasMember("name"));
		assert(doc["tilesets"][0].HasMember("tilecount"));
		assert(doc["tilesets"][0].HasMember("tileheight"));
		assert(doc["tilesets"][0].HasMember("tilewidth"));
		//sub array
		assert(doc["tilesets"][0].HasMember("tiles"));

		//tilesets sub-array members
		assert(doc["tilesets"][0]["tiles"][0].HasMember("id"));
		//tiles sub-array
		assert(doc["tilesets"][0]["tiles"][0].HasMember("properties"));
	}
}


//
// CURRENT GAME ADDITIONS
//

namespace TiledLoaders
{
	void LoadTilePropertiesData(rapidjson::Document& doc, int tilesetIndex, std::unordered_map<int, TileProperties>& container)
	{
		//Go through each tile in the array, extract their unique properties into the container and store
		for (auto& a : doc["tilesets"][0]["tiles"].GetArray())
		{

			std::pair<int, TileProperties> newData;
			newData.first = a["id"].GetInt();
			newData.second.tileID = a["id"].GetInt();
			newData.second.impassable = a["properties"][1]["value"].GetBool();
			newData.second.moveCost = a["properties"][2]["value"].GetFloat();
			newData.second.occupied = a["properties"][3]["value"].GetBool();
			newData.second.tileName = a["properties"][4]["value"].GetString();
			newData.second.terrainTypeID = a["properties"][5]["value"].GetInt();

			container.insert(newData);
		}
	}
}
