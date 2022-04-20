#include "StartScreenMode.h"

StartScreenMode::StartScreenMode(size_t id)
	:ModeInterface(id)
{
	m_Background.SetTexturePtr(Game::GetGame()->GetAssetManager().GetSpriteTextureData(std::string("Start_Screen_BG_01")));
}

StartScreenMode::~StartScreenMode()
{
	Release();
}

void StartScreenMode::Update(const GameTimer& gt)
{
}

void StartScreenMode::Render(PassData& data)
{
	//Get Command list for draws
	auto commandList = Game::GetGame()->GetCommandList().Get();

	//Begin Scene Draw
	data.sceneSB->Begin(commandList);

	m_Background.Draw(*data.sceneSB, data.heap);

	//End Scene Draw
	data.sceneSB->End();
}

void StartScreenMode::ProcessKey(char key)
{
	switch (key)
	{
	case(VK_SPACE):
		Game::GetGame()->GetModeManager().SwitchMode(ModeNames::MAIN_MENU);
		break;
	}
}

void StartScreenMode::Release()
{

}
