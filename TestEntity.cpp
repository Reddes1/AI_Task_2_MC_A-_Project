#include "TestEntity.h"

#include "Game.h"			//Manager Access


using namespace DirectX;

TestEntity::TestEntity(std::shared_ptr<SpriteTexture> texture)
	:EntityInterface(texture)
{

}

void TestEntity::Update(const GameTimer& gt)
{

	//Reset Flags for Frame
	memset(&m_InputFlags, 0, sizeof(m_InputFlags));

	//Process inputs InputManager
	ProcessInputs();

	//Update position with flags
	UpdateMovement(gt.DeltaTime());
}

void TestEntity::ProcessInputs()
{
	//Get Game Handle
	Game* game = Game::GetGame();

	////Check Directional Inputs
	//m_InputFlags.up = game->GetKBMManager().IsPressed(VK_W) || game->GetKBMManager().IsPressed(VK_UP);
	//m_InputFlags.down = game->GetKBMManager().IsPressed(VK_S) || game->GetKBMManager().IsPressed(VK_DOWN);
	//m_InputFlags.left = game->GetKBMManager().IsPressed(VK_A) || game->GetKBMManager().IsPressed(VK_LEFT);
	//m_InputFlags.right = game->GetKBMManager().IsPressed(VK_D) || game->GetKBMManager().IsPressed(VK_RIGHT);

}

void TestEntity::UpdateMovement(float dt)
{
	//Capture current position
	XMFLOAT2 pos = GetPrimarySprite().GetPosition();
	//Hold empty container
	XMFLOAT2 newPos = { 0.f, 0.f };

	if (m_InputFlags.up)
		newPos.y -= 300.f * dt;
	else if(m_InputFlags.down)
		newPos.y += 300.f * dt;
	if(m_InputFlags.left)
		newPos.x -= 300.f * dt;
	else if(m_InputFlags.right)
		newPos.x += 300.f * dt;


	//Set new position
	newPos.x += pos.x;
	newPos.y += pos.y;

	GetPrimarySprite().SetPosition(newPos);

	GetPrimarySprite().IncrementRotation((PI * 2) * dt);

}
