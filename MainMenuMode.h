#pragma once

#include "ModeInterface.h"			//Parent
#include "Game.h"					//Main Game Object

#include "Sprite.h"

class MainMenuMode : public ModeInterface
{
public:

	MainMenuMode(size_t id);
	~MainMenuMode();

	/////////////////
	/// Overrides ///
	/////////////////

	void Enter() override;
	bool Exit() override { return true; }
	void Update(const GameTimer& gt) override;
	void Render(PassData& data) override;
	void ProcessKey(char key) override;
	void Release() override;

private:

	enum class STATE
	{
		MAIN_MENU,
		TUTORIAL
	};

	void ProcessKeyMainMenuState(char key);
	void ProcessKeyTutorialState(char key);
	void InitModeAudio();
	//Main Background
	Sprite m_Background;
	//Tutorial Images
	Sprite m_Tutorials;
	//Internal State
	STATE m_State;
	//Main Menu
	NavigationalMenu* m_MainMenu = nullptr;
	//Control reminder
	NonNavigationUI* m_ContextControl = nullptr;
	//Tutorial Frame Management
	int m_FrameMax = 5;
	int m_CurrentFrame = 0;

};