#pragma once
#include "EntityInterface.h"

#include "GameUtilities.h"

class TestEntity : public EntityInterface
{
public:

	TestEntity() {}
	//Call and pass texture to parent
	TestEntity(std::shared_ptr<SpriteTexture> texture);

	/////////////////
	/// Overrides ///
	/////////////////

	void Update(const GameTimer& gt);
	void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap)
	{
		EntityInterface::Render(batch, heap);
	}

	//Optional Overides (Uncomment as needed)
	//void ResetEntityToDefault() { }


private:
	
	//Manage inputs
	void ProcessInputs();

	//Move entity using inputflags
	void UpdateMovement(float dt);

	//Local input flag container
	InputFlags16 m_InputFlags;
	

};


