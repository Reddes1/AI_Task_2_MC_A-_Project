#pragma once
#include "Sprite.h"
#include "GameTimer.h"

/*
	Parent class for any would be game entity. Create any children through this.
*/

class EntityInterface
{
public:

	EntityInterface() {}
	//Optional pre-load of texture
	EntityInterface(std::shared_ptr<SpriteTexture> texture);

	//////////////////
	/// Structures ///
	//////////////////

	struct EntityData
	{
		EntityData()
			:boundaryLimit(0.f, 0.f, 1920.f, 1080.f),
			objectID(-1),
			isActive(true),
			isInputEnabled(false),
			isBoundaryEnabled(false)
		{}

		DirectX::SimpleMath::Vector4 boundaryLimit;
		int objectID;
		bool isActive : 1;
		bool isInputEnabled : 1;
		bool isBoundaryEnabled : 1;
	};

	struct BoundingBoxData
	{
		BoundingBoxData()
			:boxCollider(50, 50, 50),
			isCollisionEnabled(false)
		{}

		DirectX::SimpleMath::Vector3 boxCollider;
		bool isCollisionEnabled : 1;
	};


	////////////////
	/// Virtuals ///
	////////////////
	
	//
	//	Core Virtuals
	//

	//Update entity logic
	virtual void Update(const GameTimer& gt) = 0;
	//Render entity in the game (default provided)
	virtual void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap) { m_PrimarySprite.Draw(batch, heap); }


	//
	// Support/Util Virtuals
	//

	//Supporting function that should typically be used to normalise an entities values (defined by child)
	virtual void ResetEntityToDefaults() { }

	///////////
	/// Set ///
	///////////

	void SetMapCoordinates(DirectX::XMINT2& newCoords) { m_MapCoordinates = newCoords; }
	void SetMapCoordinates(int32_t x, int32_t y) { m_MapCoordinates.x = x; m_MapCoordinates.y = y; }
	void SetMapCoordinateMax(DirectX::XMINT2& newMax) { m_MapCoordinateLimit = newMax; }
	void SetMapCoordinateMax(int32_t x, int32_t y) { m_MapCoordinateLimit.x = x; m_MapCoordinateLimit.y; }

	///////////
	/// Get ///
	///////////

	Sprite& GetPrimarySprite() { return m_PrimarySprite; }
	BoundingBoxData& GetBoundingBoxData() { return m_BBData; }
	EntityData& GetEntityData() { return m_EntityData; }
	DirectX::XMINT2& GetMapCoordinates()  { return m_MapCoordinates; }
	DirectX::XMINT2& GetMapCoordinateLimit() { return m_MapCoordinateLimit; }


	//////////////////
	/// Operations ///
	//////////////////

	void DecrementXCoordinate(bool clampToZero = true);
	void DecrementYCoordinate(bool clampToZero = true);
	void IncrementXCoordinate(bool clampToLimit = true);
	void IncrementYCoordinate(bool clampToLimit = true);
	int GetMaxCoordDifferenceX() { return m_MapCoordinateLimit.x - m_MapCoordinates.x; }
	int GetMaxCoordDifferenceY() { return m_MapCoordinateLimit.y - m_MapCoordinates.y; }

	//
	// Utilities
	//

	//Add EI or Derived class to a EI container (Pointers)
	static void Add(std::vector<EntityInterface*>& container, EntityInterface* object);
	//Release a container		(Note: Could be generalised really)
	static void ReleaseContainer(std::vector<EntityInterface*>& container);

private:


protected:

	Sprite m_PrimarySprite;
	BoundingBoxData m_BBData;
	EntityData m_EntityData;
	DirectX::XMINT2 m_MapCoordinates = { -1, -1 };
	DirectX::XMINT2 m_MapCoordinateLimit = { -1, -1 };
};

/*
-----------------------------------------------------------------------------------
	Copy Paste Me Into Any Derived Class (Starter Class, Change Name)
-----------------------------------------------------------------------------------

class DerivedEntity : public EntityInterface
{
public:

	/////////////////
	/// Overrides ///
	/////////////////

	void Update(const GameTimer& gt) { }		//Create in .cpp
	void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap)
	{
		EntityInterface::Render(batch, heap);
	}

	//Optional Overides (Uncomment as needed)
	//void ResetEntityToDefault() { }


private:

	//Data Here


};
*/