#include "AnimTestEntity.h"

#include "Game.h"			//Manager Access

using namespace DirectX;

AnimTestEntity::AnimTestEntity(std::shared_ptr<SpriteTexture> texture)
	:EntityInterface(texture)
{

}

void AnimTestEntity::Update(const GameTimer& gt)
{
	//Get Game Handle
	Game* game = Game::GetGame();

	//Check anim flags
	m_InputFlags.action1 = game->GetKBMManager().IsPressed(VK_G);
	m_InputFlags.action2 = game->GetKBMManager().IsPressed(VK_H);
	m_InputFlags.action3 = game->GetKBMManager().IsPressed(VK_J);
	m_InputFlags.action4 = game->GetKBMManager().IsPressed(VK_K);

	////Check Directional Inputs
	//m_InputFlags.up = game->GetKBMManager().IsPressed(VK_W) || game->GetKBMManager().IsPressed(VK_UP);
	//m_InputFlags.down = game->GetKBMManager().IsPressed(VK_S) || game->GetKBMManager().IsPressed(VK_DOWN);
	//m_InputFlags.left = game->GetKBMManager().IsPressed(VK_A) || game->GetKBMManager().IsPressed(VK_LEFT);
	//m_InputFlags.right = game->GetKBMManager().IsPressed(VK_D) || game->GetKBMManager().IsPressed(VK_RIGHT);


	if (m_InputFlags.action1)
		GetPrimarySprite().GetAnimator().SetAnimation(0, true, true, false);
	else if (m_InputFlags.action2)
	 	GetPrimarySprite().GetAnimator().SetAnimation(1, true, true, false);
	else if (m_InputFlags.action3)
	 	GetPrimarySprite().GetAnimator().SetAnimation(2, true, true, false);
	else if (m_InputFlags.action4)
		GetPrimarySprite().GetAnimator().SetAnimation(3, true, true, false);



	//Capture current position
	XMFLOAT2 pos = GetPrimarySprite().GetPosition();
	//Hold empty container
	XMFLOAT2 newPos = { 0.f, 0.f };

	if (m_InputFlags.up)
		newPos.y -= 400.f * gt.DeltaTime();
	else if (m_InputFlags.down)
		newPos.y += 400.f * gt.DeltaTime();
	if (m_InputFlags.left)
		newPos.x -= 400.f * gt.DeltaTime();
	else if (m_InputFlags.right)
		newPos.x += 400.f * gt.DeltaTime();


	//Set new position
	newPos.x += pos.x;
	newPos.y += pos.y;

	GetPrimarySprite().SetPosition(newPos);

	//Update Animation
	GetPrimarySprite().GetAnimator().Update(gt.DeltaTime());


	//Done with flags, so clear for next frame
	memset(&m_InputFlags, 0, sizeof(m_InputFlags));
}

void AnimTestEntity::Move(int dir)
{
	switch(dir)
	{
	case 0:
		m_InputFlags.up = true;
		break;
	case 1:
		m_InputFlags.down = true;
		break;
	case 2:
		m_InputFlags.left = true;
		break;
	case 3:
		m_InputFlags.right = true;
		break;
	}
}
