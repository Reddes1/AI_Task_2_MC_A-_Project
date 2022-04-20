#pragma once

#include "ModeInterface.h"			//Parent
#include "Game.h"					//Main Game Object

#include "2DCameraTypes.h"			//Scene Management
#include "MapTilePathfinding.h"		//Pathfinding algorithm for grid
#include "TargetingSystems.h"		//For mapping unit attack range when called

//Forward Dec
class CursorEntity;

class MainGameMode : public ModeInterface
{
public:

	enum class MODE_STATE
	{
		//Camera Move States
		FREE_NAVIGATION,
		MOVE_MENU_OPEN,
		FIND_SKILL_TARGET,
		//Menu Related States
		UNIT_MENU_OPEN,
		NON_UNIT_MENU_OPEN,
		ACTION_MENU_OPEN,
		ATTACK_MENU_OPEN,
		STATUS_FRAME_OPEN,
		//Setup
		UNIT_SELECTION,
		UNIT_PLACEMENT,
		//Attack
		UNIT_PERFORM_ACTION_STATE,
		//Other,
		GAME_OVER
	};

	enum SkillTarget
	{
		ALL_UNITS,
		FRIENDLY_UNITS,
		ENEMY_UNITS
	};

	////////////////////
	/// Constructors ///
	////////////////////

	MainGameMode(size_t id);
	~MainGameMode();


	/////////////////
	/// Overrides ///
	/////////////////

	void Enter() override;
	bool Exit() override;
	void Update(const GameTimer& gt) override;
	void Render(PassData& data) override;
	void ProcessKey(char key) override;
	void Release() override;


private:


//-----------------------------------//

	//////////////////////////////
	/// Functions & Operations ///
	//////////////////////////////
	
//-----------------------------------//


	/////////////
	/// Setup ///
	/////////////

	//Setup object containers
	void InitObjectContainers();
	//Setup UI & functionality elements
	void InitUIFunctionality();
	//Setup the map
	void InitMapTiles();
	//Unit Selection Testing Init
	void InitUnitSelection();
	//Setup modes audio
	void InitModeAudio();
	//Clears loose ends (one-offs and edge case preventions)
	void FinaliseInit();

	//Setup a unit that a player selected
	bool SetupSelectedUnit();

	//Resets the mode back to starting point for a new game
	void StartFreshGame();
	//Releases all UI Elements held to the manager
	void ReleaseAllUIElements();


	/////////////////
	/// Rendering ///
	/////////////////

	void RenderBackground(PassData& data, ID3D12GraphicsCommandList* cl);
	void RenderScene(PassData& data, ID3D12GraphicsCommandList* cl);
	void RenderUI(PassData& data, ID3D12GraphicsCommandList* cl);


	////////////////////////////
	/// States/State Control ///
	////////////////////////////

	//Change the state BEFORE doing any one-off state operations
	void OnStateEnter(MODE_STATE newState);
	//Change the state AFTER doing any one-off state operations
	void OnStateExit(MODE_STATE newState);

	//Menu States
	void ProcessKeyUnitMenuOpenState(char key);
	void ProcessKeyUnitActionOpenState(char key);
	void ProcessKeyMoveMenuOpenState(char key);
	void ProcessKeyNonUnitMenuOpenState(char key);
	void ProcessKeyStatusFrameOpenState(char key);

	//Camera Move States
	void ProcessKeyFreeMoveState(char key);
	void ProcessKeyFindTargetState(char key);

	//Unit Creation States
	void ProcessKeyUnitSelectionState(char key);
	void ProcessKeyUnitPlacementState(char key);
	
	//Gameplay states
	void ProcessKeyPerformActionState(char key);

	//Other
	void ProcessKeyGameOverState(char key);

	/////////////////////
	/// Team Controls ///
	/////////////////////
	
	//Determines current team ID and flips it over the other
	void ChangeTeamID();
	//Manage the switching of the teams between modes
	void SwitchTeams();
	//Determine if a teams actions are fully spend, and switch the other team if done.
	void CheckIfTeamActionsSpent();
	//Manage shift from action menu to targeting mode
	void SwitchUnitMenuToTargetingMode(UIElementManager*& manager, MODE_STATE state);
	//Runs post action clean up to determine what to do with the unit, UI clean up, team state check etc.
	void PostActionCleanUp();

	//Snaps the camera to the first unit found alive in a given team.
	bool SnapCameraToUnit();


	//////////
	/// UI ///
	//////////

	//Updates the cursor behaviour for FreeNav State
	void UpdateCursorFreeMode();
	//Updates the cursor behaviour for Targeting State(s)
	void UpdateCursorTargetingMode();

	//Updates the tile tooltip with information from current tile highlighted by cursor
	void UpdateTileTooltip(NonNavigationUI* ui, MapTile* tile);
	//Updates the unit tooltip with information from current unit highlighted by cursor
	void UpdateUnitTooltip(NonNavigationUI* ui, UnitEntity* unit);
	//Updates the skills tooltip with currently selected skill
	void UpdateSkillTooltip();
	//Updates the active team frame with current team ID
	void UpdateTeamFrame();
	//Updates Status Frame with unit details
	void UpdateStatusFrame(NonNavigationUI* ui, UnitEntity* unit);

	//Change the displayed unit during Unit Selection State
	void ChangeDisplayUnit();

	//Setup the Menu UI and adjust according to units available actions
	void SetupUnitMenu(NavigationalMenu* menu, UnitEntity* unit);

	//Setup gameover menu
	void SetupGameOverMenu(int teamWinID);

	//Captures tiles within grid range and enable grid effect (pass team id for where to draw grid)
	void EnablePlacementGrid();
	//Release current grid, disabling grid effect before losing pointer
	void DisablePlacementGrid();
	//Check if tile is inside of placement grid
	bool IsTileInPlacementGrid(MapTile* tile);



	///////////////////////
	/// Gameplay/Combat ///
	///////////////////////
	
	//Activates the currently selected skill on the main unit on the 2nd unit.
	void UseSelectedSkillOnTarget(UnitEntity* unit1, UnitEntity* unit2);
	//Status Check the unit
	void UnitStatusCheck(UnitEntity* unit);
	//Check if a team is wiped out and end game accordingly
	void CheckTeamAliveStates();
	//Set Death Animation for unit based on class
	void SetDeathAnim(UnitEntity* unit);
	//AOE
	void AOE();

	/////////////////
	/// Utilities ///
	/////////////////

	//Quick unit kill, for quick testing
	void KillUnit(UnitEntity* unit);

//-----------------------------//

	////////////////////////
	/// Variables & Data ///
	////////////////////////

//----------------------------//


	////////////////////////////////
	/// Managers & Functionality ///
	////////////////////////////////

	//Game Object that manages pathfinding in the context of a grid
	MapTilePathfinder m_PathFinder;
	//Manages the matrix for shifting the scene around, producing a camera effect
	FixedDist2DCamera m_Camera;
	//Manages a manifest of highlighted tiles & operations for enabling grid effects
	DiamondRadiusTargeting m_TargetingSystem;
	//Tracks the internal mode state
	MODE_STATE m_State;

	/////////////
	/// Setup ///
	/////////////

	//Camera snapping position for placing units
	DirectX::XMINT2 m_TeamOneCameraPos = { 11, 40 };
	DirectX::XMINT2 m_TeamTwoCameraPos = { 31, 8 };

	//Hard reset point for the cursor
	DirectX::XMINT2 m_DefaultCamPos = { 25, 25 };

	//Store and hold the Tile Dims for setups and mathematical functions
	DirectX::XMINT2 m_TileDims = { 0, 0 };
	//Store Map X/Y Maximum
	DirectX::XMINT2 m_MapLimit = { 0, 0 };

	//Holding value for selector, for determining what class unit to display
	int m_UnitSelectorClassID = 0;
	//Tracks how many units have been placed so far in Unit Placement State(s)
	int m_PlacedUnitCount = 0;
	//Unit Display for Selection Process
	UnitEntity* m_UnitSelectionDisplayUnit;


	/////////////////////////
	/// Object Containers ///
	/////////////////////////

	//Background sprites
	std::vector<EntityInterface*> m_Backgrounds;
	//Scene Terrain Objects (Tiles)
	std::vector<MapTile*> m_TileMap;
	//Scene Actors (Player, Enemies etc)
	std::vector<EntityInterface*> m_Actors;
	//Scene Projectiles
	std::vector<EntityInterface*> m_Projectiles;


	//Stores a copy pointer for each teams unit for operations & quick access
	std::vector<UnitEntity*> m_TeamOne;
	std::vector<UnitEntity*> m_TeamTwo;


	////////////
	/// Team ///
	////////////
	
	//Value hold what team starts/started (coin toss, odd/even)
	int m_StartingTeam = 1;
	//Holds what the currently active team is (1 or 2)
	int m_CurrentTeamID = -1;
	//Flag point for letting the program know to switch teams (after end turn calls or no unit actions left etc)
	bool m_SwitchTeams = false;
	//Keep track of the currently alive count
	int m_TeamOneAliveCount = TEAM_SIZE;
	int m_TeamTwoAliveCount = TEAM_SIZE;


	//////////
	/// UI ///
	//////////

	//Placement grid data
	std::vector<MapTile*> m_PlacementGridTiles;
	DirectX::XMINT2 m_PlacementGridSize = { 5, 5 };
	DirectX::XMINT2 m_TeamOneGridStart = { 9, 38 };
	DirectX::XMINT2 m_TeamTwoGridStart = { 29, 6 };


	//Cursor object for navigating and searching for and holding objects in the game (Tiles, units etc)
	CursorEntity* m_Cursor = nullptr;

	//NOTE: Pending UI Overhaul for this mode to clean this up

	//Holding pointers for menus and UI elements
	NavigationalMenu* m_UnitMenu = nullptr;
	NavigationalMenu* m_ActionMenu = nullptr;
	NavigationalMenu* m_EndTurnMenu = nullptr;

	NonNavigationUI* m_TileTooltip = nullptr;
	NonNavigationUI* m_FriendlyUnitUI = nullptr;
	NonNavigationUI* m_EnemyUnitUI = nullptr;
	NonNavigationUI* m_TargetFrame = nullptr;
	NonNavigationUI* m_SkillTooltip = nullptr;
	NonNavigationUI* m_StatusFrame = nullptr;

	NonNavigationUI* m_ArrowLeft = nullptr;
	NonNavigationUI* m_ArrowRight = nullptr;
	NonNavigationUI* m_SelectionFrame = nullptr;
	NonNavigationUI* m_PlacementFrame = nullptr;
	NonNavigationUI* m_TeamFrame = nullptr;
	NonNavigationUI* m_GameOverFrame = nullptr;
	NonNavigationUI* m_ContextControlGame = nullptr;
	NonNavigationUI* m_ContextControlCharSelect = nullptr;

	//Tracking value for UI in determining what skill is selected
	int m_SkillIndex = 0;


	//////////////////////////////////////
	/// Temporary Testing Data/Objects ///
	//////////////////////////////////////

};