#include "MainMenuMode.h"

MainMenuMode::MainMenuMode(size_t id)
	:ModeInterface(id)
{
	m_Background.SetTexturePtr(Game::GetGame()->GetAssetManager().GetSpriteTextureData(std::string("Main_Menu_BG_00")));
	m_Tutorials.SetTexturePtr(Game::GetGame()->GetAssetManager().GetSpriteTextureData(std::string("Tutorial_00")));

	m_State = STATE::MAIN_MENU;
}

MainMenuMode::~MainMenuMode()
{
	//Release pointers
	m_MainMenu = nullptr;
	m_ContextControl = nullptr;
}

void MainMenuMode::Enter()
{
	Game::GetGame()->GetUIManager().GetNavigationMenuByTypeID(m_MainMenu, UIElementIDs::MAIN_MENU_00);
	Game::GetGame()->GetUIManager().GetNonNavigationUIByTypeID(m_ContextControl, UIElementIDs::CONTEXT_CONTROLS_01);
	m_MainMenu->GetCursorSprite().SetScale(1.35f, 1.f);
	InitModeAudio();
}

void MainMenuMode::Update(const GameTimer& gt)
{
}

void MainMenuMode::Render(PassData& data)
{
	//Get Command list for draws
	auto commandList = Game::GetGame()->GetCommandList().Get();

	//Begin Scene Draw
	data.sceneSB->Begin(commandList);

	switch (m_State)
	{
	case STATE::MAIN_MENU:
		m_Background.Draw(*data.sceneSB, data.heap);
		m_MainMenu->Render(*data.sceneSB, data.heap);
		break;
	case STATE::TUTORIAL:
		m_Tutorials.Draw(*data.sceneSB, data.heap);
		break;
	}

	m_ContextControl->Render(*data.sceneSB, data.heap);

	//End Scene Draw
	data.sceneSB->End();
}

void MainMenuMode::ProcessKey(char key)
{
	switch (m_State)
	{
	case STATE::MAIN_MENU:
		ProcessKeyMainMenuState(key);
		break;
	case STATE::TUTORIAL:
		ProcessKeyTutorialState(key);
		break;
	}
}

void MainMenuMode::Release()
{
}

void MainMenuMode::ProcessKeyMainMenuState(char key)
{
	switch (key)
	{
	case 'f':
	case 'F':
		//Get Menu Index, execute action based on index
		switch (m_MainMenu->GetMenuIndex().y)
		{
		case 0:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			Game::GetGame()->GetModeManager().SwitchMode(ModeNames::MAIN_GAME_MODE);
			break;
		case 1:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			m_State = STATE::TUTORIAL;

			break;
		case 2:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			PostQuitMessage(0);
			break;
		}

		break;
	case 'w':
	case 'W':
		//Move menu
		m_MainMenu->MoveUp();
		//Play Nav Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		break;

	case 's':
	case 'S':
		//Move menu
		m_MainMenu->MoveDown();
		//Play Nav Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		break;
	}
}

void MainMenuMode::ProcessKeyTutorialState(char key)
{
	switch (key)
	{
	case VK_ESCAPE:

		break;
	case 'f':
	case 'F':
		//Play Nav Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		++m_CurrentFrame;
		if (m_CurrentFrame > m_FrameMax)
		{
			m_CurrentFrame = 0;
			m_Tutorials.SetFrame(m_CurrentFrame);
			m_State = STATE::MAIN_MENU;
		}
		else
		{
			m_Tutorials.SetFrame(m_CurrentFrame);
		}

		break;
	}
}

void MainMenuMode::InitModeAudio()
{
	//Menu Sounds
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0,
		L"data/music/Menu/MenuNav.wav");
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1,
		L"data/music/Menu/SelectionConfirm.wav");
}
