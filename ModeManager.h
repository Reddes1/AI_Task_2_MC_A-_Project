#pragma once

#include "ModeInterface.h"    //For Mode Containers
#include "D3D.h"

/*
	Load, store and manage all the modes/states of the game (stages, menus, loading etc etc).
	Manages the transition between states
*/
class ModeManager
{
public:
	ModeManager();
	~ModeManager() { Release(); }

	//////////////
	/// Update ///
	//////////////

	//Update current mode
	void Update(const GameTimer& gt);
	//Render current mode
	void Render(PassData& data);
	//Handle WH_CHAR key messages
	void ProcessKey(char key);

	//////////////////
	/// Operations ///
	//////////////////

	//Add a new mode via dynamic allocation
	void AddMode(ModeInterface* p);
	//Change current mode via ID
	bool SwitchMode(size_t id);


private:
	std::vector<ModeInterface*> m_Modes;	//container of modes
	int m_CurrentModeIndex = -1;			//the one that is active
	int m_NextModeIndex = -1;				//one that wants to be active

	//Release mode resources, called in destructor
	void Release();
};