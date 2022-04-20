#pragma once
#include "D3D.h"

//Forward Declarations
class EntityInterface;

/*
	It can be useful to direct the flow of a mode based on what outcome we want (like restarting the level, or
	progressing to the next). We can use these flags for that.
*/
struct ModeStateFlags
{
	ModeStateFlags()
		:startNewGame(true), modePaused(false), modeGameOver(false), modeRestart(false),
		modeComplete(false), modeLoaded(false), disableInput(false)
	{}
	bool startNewGame : 1;
	bool modePaused : 1;
	bool modeGameOver : 1;
	bool modeRestart : 1;
	bool modeComplete : 1;
	bool modeLoaded : 1;
	bool disableInput : 1;
};

/*
	Mode interface that any particular mode of a game (main menu, game over etc) will use.
*/
class ModeInterface
{
public:

	//Each Mode MUST be given an ID to be found in the ModeManager (So make sure it is unique via enum)
	ModeInterface(size_t id)
		:m_ModeID(id)
	{}
	virtual ~ModeInterface() {}

	////////////
	/// Core ///
	////////////

	//called on the new mode just once when it first activates
	virtual void Enter() {}
	/*
		Called on the old mode when switching to the new one
		The switch won't complete until we return true, gives time for
		effects like fading out.
	*/
	virtual bool Exit() { return true; }
	//once active this is called repeatedly, contains the logic of the mode
	virtual void Update(const GameTimer& gt) = 0;
	//used by a mode to render itself
	virtual void Render(PassData& data) = 0;
	//Pass WM_CHAR key messages to the mode for it to interpret
	virtual void ProcessKey(char key) { }
	//Release any assets here 
	virtual void Release() = 0;

	
	///////////
	/// Get ///
	///////////

	//Get Mode ID
	const size_t GetModeID() { return m_ModeID; }
	//Get Mode State Flag
	ModeStateFlags& GetModeFlags() { return m_ModeState; }

private:

protected:

	ModeStateFlags m_ModeState;
	size_t m_ModeID;

};