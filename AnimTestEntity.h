#pragma once
#include "EntityInterface.h"

#include "GameUtilities.h"

class AnimTestEntity : public EntityInterface
{
public:

	AnimTestEntity() { }
	AnimTestEntity(std::shared_ptr<SpriteTexture> texture);
	~AnimTestEntity() {}

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

	//Temp function: 0 = N, 1 = D, 2 = L, 3 = R
	void Move(int dir);

private:
	InputFlags16 m_InputFlags;

};
