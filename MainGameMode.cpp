#include "MainGameMode.h"

#include "RapidJSONLoaderUtils.h"   //Map data loader

#include "GeneralUtils.h"		//String Conversion
#include "TilemapUtils.h"		//Tilemap Loading Help

//Game Objects
#include "TestEntity.h"         //Entity Testing
#include "AnimTestEntity.h"	    //Animation Testing
#include "MapTile.h"			//Terrain
#include "UnitEntity.h"			//Game Unit
#include "CursorEntity.h"		//Cursor

using namespace DirectX;
using namespace rapidjson;
using namespace TiledLoaders;


MainGameMode::MainGameMode(size_t id)
	:ModeInterface(id), m_State(MODE_STATE::UNIT_SELECTION)
{
	m_TeamOne.reserve(5);
	m_TeamTwo.reserve(5);

	//Populate Containers
	InitMapTiles();

	//Testing UnitSelection Init
	InitUnitSelection();

	InitUIFunctionality();
	FinaliseInit();

}

MainGameMode::~MainGameMode()
{
	Release();
}

void MainGameMode::Enter()
{
	StartFreshGame();	
}

bool MainGameMode::Exit()
{
	ReleaseAllUIElements();
	return true;
}

void MainGameMode::Update(const GameTimer& gt)
{
	//If a team switch is flagged, call function here (might be alt line of update/render like a state when expanded later)
	if (m_SwitchTeams)
		SwitchTeams();

	//Update Actors
	for (auto& a : m_Actors)
	{
		a->Update(gt);
	}
	//Update Terrain
	for (auto& a : m_TileMap)
	{
		a->Update(gt);
	}

	//Update Display Unit for Animation Purposes
	m_UnitSelectionDisplayUnit->Update(gt);

	//Update Looping Audio
	Game::GetGame()->GetAudioManager().UpdateAudioEngine(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFXInstance0);
	//Check for camera updates
	m_Camera.UpdateCamera();

	switch (m_State)
	{
		//////////////////////////
		/// Camera Move States ///
		//////////////////////////

	case MODE_STATE::FREE_NAVIGATION:
		UpdateCursorFreeMode();
		break;
	case MODE_STATE::MOVE_MENU_OPEN:
		UpdateCursorTargetingMode();
		break;
	case MODE_STATE::FIND_SKILL_TARGET:
		UpdateCursorTargetingMode();
		break;
		///////////////////
		/// Menu States ///
		///////////////////

	case MODE_STATE::UNIT_MENU_OPEN:
		break;
	case MODE_STATE::ACTION_MENU_OPEN:
		break;
	case MODE_STATE::ATTACK_MENU_OPEN:
		break;
	case MODE_STATE::STATUS_FRAME_OPEN:
		break;
	case MODE_STATE::UNIT_PERFORM_ACTION_STATE:
		Game::GetGame()->GetGameplayManager().GetActionStateUnitAtIndex(0)->Update(gt);
		Game::GetGame()->GetGameplayManager().GetActionStateUnitAtIndex(1)->Update(gt);
		Game::GetGame()->GetGameplayManager().GetActionStateUnitAtIndex(2)->Update(gt);
		Game::GetGame()->GetGameplayManager().Update(gt, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), static_cast<UnitEntity*>(m_Cursor->GetSecondObject()),m_SkillIndex);
		break;
	}
}

void MainGameMode::Render(PassData& data)
{
	//Get Command list for draws
	auto commandList = Game::GetGame()->GetCommandList().Get();

	//Render Background
	RenderBackground(data, commandList);

	//Render Scene Objects
	RenderScene(data, commandList);

	//Render UI Elements
	RenderUI(data, commandList);

	
}

void MainGameMode::ProcessKey(char key)
{
	switch (m_State)
	{
	case MODE_STATE::UNIT_SELECTION:
		ProcessKeyUnitSelectionState(key);
		break;
	case MODE_STATE::UNIT_PLACEMENT:
		ProcessKeyUnitPlacementState(key);
		break;
	case MODE_STATE::FREE_NAVIGATION:
		ProcessKeyFreeMoveState(key);
		break;
	case MODE_STATE::UNIT_MENU_OPEN:
		ProcessKeyUnitMenuOpenState(key);
		break;
	case MODE_STATE::NON_UNIT_MENU_OPEN:
		ProcessKeyNonUnitMenuOpenState(key);
		break;
	case MODE_STATE::STATUS_FRAME_OPEN:
		ProcessKeyStatusFrameOpenState(key);
		break;
	case MODE_STATE::ACTION_MENU_OPEN:
		ProcessKeyUnitActionOpenState(key);
		break;
	case MODE_STATE::MOVE_MENU_OPEN:
		ProcessKeyMoveMenuOpenState(key);
		break;
	case MODE_STATE::FIND_SKILL_TARGET:
		ProcessKeyFindTargetState(key);
		break;
	case MODE_STATE::UNIT_PERFORM_ACTION_STATE:
		ProcessKeyPerformActionState(key);
		break;
	case MODE_STATE::GAME_OVER:
		ProcessKeyGameOverState(key);
		break;
	}
}

void MainGameMode::Release()
{
	//Release Cursor
	delete m_Cursor;
	m_Cursor = nullptr;
	delete m_UnitSelectionDisplayUnit;
	m_UnitSelectionDisplayUnit = nullptr;

	//Release UI Elements
	ReleaseAllUIElements();

	//Release All Containers

	for (auto& a : m_TeamOne)
		a = nullptr;
	for (auto& a : m_TeamTwo)
		a = nullptr;
	
	EntityInterface::ReleaseContainer(m_Backgrounds);
	EntityInterface::ReleaseContainer(m_Actors);
	EntityInterface::ReleaseContainer(m_Projectiles);

	for (auto& a : m_TileMap)
	{
		delete a;
		a = nullptr;
	}
	m_TileMap.clear();
}

void MainGameMode::InitObjectContainers()
{
	//Get Game Handle
	Game* game = Game::GetGame();

	///////////////////////////
	/// Setup background(s) ///
	///////////////////////////

	TestEntity* bg = new TestEntity(game->GetAssetManager().GetSpriteTextureData(std::string("Main_Game_BG_00")));
	EntityInterface::Add(m_Backgrounds, bg);


	//////////////////////
	/// Setup Entities ///
	//////////////////////	

	//Class setups (sync them together)
	std::shared_ptr<SpriteTexture> unitTexs[] =
	{
		game->GetAssetManager().GetSpriteTextureData(std::string("WARRIOR_CLASS_SS_00")),
		game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00")),
		game->GetAssetManager().GetSpriteTextureData(std::string("BARBARIAN_CLASS_SS_00")),
		game->GetAssetManager().GetSpriteTextureData(std::string("NECROMANCER_CLASS_SS_00")),
		game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00")),
	};
	int frames[] =
	{
		static_cast<int>(WarriorAnimIndexes::IDLE_00),
		static_cast<int>(AssassinAnimIndexes::IDLE_00),
		static_cast<int>(BarbarianAnimIndexes::IDLE_00),
		static_cast<int>(NecromancerAnimIndexes::IDLE_00),
		static_cast<int>(SorceressAnimIndexes::IDLE_00),

	};
	int classes[]
	{
		GameClasses::WARRIOR,
		GameClasses::ASSASSIN,
		GameClasses::BARBARIAN,
		GameClasses::NECROMANCER,
		GameClasses::SORCERESS,
	};

	//Team One Coordinates
	XMINT2 coords[] =
	{
		{20, 21},
		{21, 22},
		{22, 23},
		{21, 24},
		{20, 25}
	};

	//Team Two Coordinates
	XMINT2 coords2[] =
	{
		{27, 21},
		{26, 22},
		{25, 23},
		{26, 24},
		{27, 25}
	};
	//Team One Loop
	for (int i(0); i < 5; ++i)
	{
		UnitEntity* unit = new UnitEntity(unitTexs[i]);

		unit->GetPrimarySprite().SetPosition((coords[i].x * m_TileDims.x) + (m_TileDims.x * 0.5f), (coords[i].y * m_TileDims.y) + (m_TileDims.y * 0.5f));
		unit->SetMapCoordinates(coords[i]);
		unit->SetMapCoordinateMax(m_MapLimit);
		unit->SetTileSize(m_TileDims);
		unit->GetPrimarySprite().SetScale(0.20f, 0.20f);
		unit->GetPrimarySprite().GetAnimator().SetAnimation(frames[i], true, true, false);
		unit->GetPrimarySprite().SetOriginToCenter();
		unit->SetUnitType(UnitEntity::UNIT_TYPE::LAND);
		unit->SetUnitTeamID(1);
		unit->SetUnitClass(classes[i]);
		unit->SetUnitBaseEquipment(1, 0, 2, 3, 4);
		unit->InitTotalValues();
		unit->SetSkillsBasedOnClass();

		EntityInterface::Add(m_Actors, unit);
		m_TeamOne.push_back(unit);

	}
	//Team Two loop
	for (int i(0); i < 5; ++i)
	{
		UnitEntity* unit = new UnitEntity(unitTexs[4-i]);

		unit->GetPrimarySprite().SetPosition((coords2[i].x * m_TileDims.x) + (m_TileDims.x * 0.5f), (coords2[i].y * m_TileDims.y) + (m_TileDims.y * 0.5f));
		unit->SetMapCoordinates(coords2[i]);
		unit->SetMapCoordinateMax(m_MapLimit);
		unit->SetTileSize(m_TileDims);
		unit->GetPrimarySprite().SetScale(0.20f, 0.20f);
		unit->GetPrimarySprite().GetAnimator().SetAnimation(frames[4-i], true, true, false);
		unit->GetPrimarySprite().SetOriginToCenter();
		unit->SetUnitType(UnitEntity::UNIT_TYPE::LAND);
		unit->SetUnitTeamID(2);
		unit->SetUnitClass(classes[4-i]);
		unit->SetUnitBaseEquipment(1, 0, 2, 3, 4);
		unit->InitTotalValues();
		unit->SetSkillsBasedOnClass();
		unit->GetPrimarySprite().SetEffect(SpriteEffects_FlipHorizontally);

		EntityInterface::Add(m_Actors, unit);
		m_TeamTwo.push_back(unit);
	}

}

void MainGameMode::InitUIFunctionality()
{
	//Get Game Handle
	Game* game = Game::GetGame();

	////////////////////
	/// Cursor Setup ///
	////////////////////

	//Get new cursor
	m_Cursor = new CursorEntity(Game::GetGame()->GetAssetManager().GetSpriteTextureData(std::string("UI_Atlas_01")));


	//Calculate the approximate middle of the screen by measuring how many tiles can fit on the screen (rounding if required)
	DirectX::XMINT2 m_CursorScreenOffset = { static_cast<int>(game->GetWindowSize().x / m_TileDims.x) / 2, static_cast<int>(game->GetWindowSize().y / m_TileDims.y) / 2 };

	//Offset the cursor to the middle of the screen
	m_Cursor->GetPrimarySprite().SetPosition((m_CursorScreenOffset.x * m_TileDims.x) + (m_TileDims.x * 0.5f), (m_CursorScreenOffset.y * m_TileDims.y) + (m_TileDims.y * 0.5f));
	m_Cursor->GetPrimarySprite().SetFrame(20);
	m_Cursor->GetPrimarySprite().SetOriginToCenter();
	m_Cursor->SetMapCoordinates(m_DefaultCamPos);
	m_Cursor->SetMapCoordinateMax(m_MapLimit);
	m_Cursor->SetTileSize(m_TileDims);

	////////////////////
	/// Camera Setup ///
	////////////////////

	m_Camera.SetupCamera(
		m_CursorScreenOffset,
		XMINT2(0, 0),
		m_MapLimit,
		m_DefaultCamPos,
		m_TileDims
	);
}

void MainGameMode::InitMapTiles()
{
	//
	// JSON Setup
	//

	//Create Document to hold JSON data & parse data
	Document doc;
	ParseNewJSONDocument(doc, std::string("data/sprites/tilemaps/Tilemap_00.json"));

	//Assert all the members we are looking for are present
	AssertBasicTilemapMembers(doc);

	//Grab hold of each sub array
	const Value& layerArr = doc["layers"][0];
	const Value& tileArr = doc["tilesets"][0];

	//
	// Build Objects 
	//

	//Generate some simple, singular frames for each tile mathematically
	std::vector<RECT> frames;
	frames.reserve(tileArr["tilecount"].GetUint());
	GenerateFrameData(doc, 0, frames);

	//Extract & populate container with tile specific data for assignment
	std::unordered_map<int, TileProperties> mapProperties;
	LoadTilePropertiesData(doc, 0, mapProperties);

	//Get the name for the tileset (This should be the same name that is given at TextureLoad time)
	std::string mapName = tileArr["name"].GetString();
	//Get texture pointers for main texture and grid texture
	std::shared_ptr<SpriteTexture> texPtr = Game::GetGame()->GetAssetManager().GetSpriteTextureData(mapName);
	std::shared_ptr<SpriteTexture> gridPtr = Game::GetGame()->GetAssetManager().GetSpriteTextureData(std::string("UI_Atlas_01"));

	//Get the map dimensions to build the map accurately
	int mapRows = doc["height"].GetInt();
	int mapCols = doc["width"].GetInt();
	//Get tile dimensions
	int tileWidth = doc["tilewidth"].GetInt();
	int tileHeight = doc["tileheight"].GetInt();

	//Store map related information for later
	m_TileDims.x = tileWidth;
	m_TileDims.y = tileHeight;
	m_MapLimit.x = mapCols - 1;
	m_MapLimit.y = mapRows - 1;

	//counter for "data" sub-array
	int count = 0;
	//Start building the map (left->right, top->bot order)
	for (int y(0); y < mapRows; ++y)
	{
		for (int x(0); x < mapCols; ++x)
		{
			//Create new object and pass texture
			MapTile* tile = new MapTile(texPtr, gridPtr);

			//Get the appropriate tile ID using the data array value
			unsigned int tileID = doc["layers"][0]["data"][count++].GetUint();

			//Use tileID rotation flags to set rotation (if required)
			tile->GetPrimarySprite().SetRotation(GetTileRotationValue(tileID));

			//Iterate through container and find this tiles unique properties (- 1 because tiled offsets awkwardly)
			std::unordered_map<int, TileProperties>::iterator it = mapProperties.find(tileID - 1);
			if (it != mapProperties.end())
				tile->SetTileProperties(it->second);

			//Decrement the ID to align it with frames container
			--tileID;
			//Use ID to set correct RECT
			tile->GetPrimarySprite().SetRect(frames.at(tileID));
			//The tile might need rotating, so center the origin
			tile->GetPrimarySprite().SetOriginToCenter();

			//Set position through row tiling (add an offset to account for origin shift)
			tile->GetPrimarySprite().SetPosition(
				static_cast<float>((tileWidth * x) + tileWidth * 0.5f),
				static_cast<float>((tileWidth * y) + tileHeight * 0.5f)
			);

			//Set the tiles x/y coordinates in world space
			tile->SetMapCoordinates(x, y);
			tile->SetMapCoordinateMax(m_MapLimit);

			//Configure the secondary sprite
			tile->MirrorTileToGridData();
			tile->GetGridSprite().SetFrame(21);
			tile->GetGridSprite().SetOriginToCenter();
			tile->GetGridSprite().SetColour(XMVECTOR{ 0.5f, 0.5f, 0.5f, 0.5f });


			//Add to container
			m_TileMap.push_back(tile);
		}
	}

	//Now all members are setup, need to set their neighbouring pointers for pathfinding purposes
	for (auto& t : m_TileMap)
	{
		//Get coords
		XMINT2& coords = t->GetMapCoordinates();

		//Do some checking to verify what neighbours should be added
		if (coords.y > 0)				//North Check
			t->SetNeighbourAtIndex(0, m_TileMap.at((coords.x + (coords.y - 1)) + ((coords.y - 1) * m_MapLimit.y)));
		if (coords.x < m_MapLimit.x)	//East Check
			t->SetNeighbourAtIndex(1, m_TileMap.at((coords.x + 1 + coords.y) + (coords.y * m_MapLimit.y)));
		if (coords.y < m_MapLimit.y)	//South Check
			t->SetNeighbourAtIndex(2, m_TileMap.at((coords.x + (coords.y + 1)) + ((coords.y + 1) * m_MapLimit.y)));
		if (coords.x > 0)				//West Check
			t->SetNeighbourAtIndex(3, m_TileMap.at((coords.x - 1 + coords.y) + (coords.y * m_MapLimit.y)));

	}

}

void MainGameMode::FinaliseInit()
{
	//Run cursor search for edge case errors
	m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);
	m_Cursor->SearchForUnitObject(m_Actors, m_MapLimit.x);

	//Set starting team ID
	m_CurrentTeamID = 1;

	//Update targeting system with what it needs to know
	m_TargetingSystem.SetTileMapLength(m_MapLimit.x);
	m_TargetingSystem.SetTileOverlayFrameIndex(19);

}

void MainGameMode::InitUnitSelection()
{
	//Get Game Handle
	Game* game = Game::GetGame();

	///////////////////////////
	/// Setup background(s) ///
	///////////////////////////

	TestEntity* bg = new TestEntity(game->GetAssetManager().GetSpriteTextureData(std::string("Main_Game_BG_00")));
	EntityInterface::Add(m_Backgrounds, bg);

	//Setup display unit (starting on Assassin Class)
	m_UnitSelectionDisplayUnit = new UnitEntity(game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00")));
	m_UnitSelectionDisplayUnit->GetPrimarySprite().SetScale(0.30f, 0.30f);
	m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation(static_cast<int>(AssassinAnimIndexes::IDLE_00), true, true, false);
	m_UnitSelectionDisplayUnit->GetPrimarySprite().SetOriginToCenter();
	m_UnitSelectionDisplayUnit->GetPrimarySprite().SetPosition(215.f, 160.f);


	//Init Actors and Team Containers with blank slates (No Class, Equipment, Texture, Skills)

	//Team One
	for (int i(0); i < TEAM_SIZE; ++i)
	{
		UnitEntity* unit = new UnitEntity();

		unit->SetMapCoordinateMax(m_MapLimit);
		unit->SetTileSize(m_TileDims);
		unit->GetPrimarySprite().SetScale(SPRITE_SCALE_ADJ, SPRITE_SCALE_ADJ);
		unit->SetUnitTeamID(1);
		unit->GetEntityData().isActive = false;

		m_Actors.push_back(unit);
		m_TeamOne.push_back(unit);
	}

	//Team Two
	for (int i(0); i < TEAM_SIZE; ++i)
	{
		UnitEntity* unit = new UnitEntity();

		unit->SetMapCoordinateMax(m_MapLimit);
		unit->SetTileSize(m_TileDims);
		unit->GetPrimarySprite().SetScale(SPRITE_SCALE_ADJ, SPRITE_SCALE_ADJ);
		unit->SetUnitTeamID(2);
		unit->GetEntityData().isActive = false;

		m_Actors.push_back(unit);
		m_TeamTwo.push_back(unit);
	}
}

void MainGameMode::InitModeAudio()
{
	//Background sounds/music
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX10,
		L"data/music/Misc/backgroundMusic.wav");
	//Menu Sounds
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX2,
		L"data/music/Menu/Pause.wav");
	//Combat/Action Sounds
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3,
		L"data/music/Combat/Buff.wav");
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX4,
		L"data/music/Combat/DamageHit.wav");
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX5,
		L"data/music/Combat/Death.wav");
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6,
		L"data/music/Combat/Heal.wav");
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX7,
		L"data/music/Combat/Melee.wav");
	Game::GetGame()->GetAudioManager().LoadSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8,
		L"data/music/Combat/Ranged.wav");
	Game::GetGame()->GetAudioManager().PlayLoopingSound(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX10,
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFXInstance0, 0.4f);
}

void MainGameMode::OnStateEnter(MODE_STATE newState)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	//Shift State BEFORE changes
	m_State = newState;

	switch (m_State)
	{
		//////////////////////////
		/// Camera Move States ///
		//////////////////////////

	case MODE_STATE::FREE_NAVIGATION:
		UpdateCursorFreeMode();
		break;
	case MODE_STATE::MOVE_MENU_OPEN:
		UpdateCursorTargetingMode();
		break;
	case MODE_STATE::FIND_SKILL_TARGET:
		UpdateCursorTargetingMode();
		break;
		///////////////////
		/// Menu States ///
		///////////////////

	case MODE_STATE::UNIT_MENU_OPEN:
		break;
	case MODE_STATE::ACTION_MENU_OPEN:
		break;
	case MODE_STATE::ATTACK_MENU_OPEN:
		break;
	case MODE_STATE::STATUS_FRAME_OPEN:
		break;
	case MODE_STATE::UNIT_PERFORM_ACTION_STATE:
		DBOUT("ENTERING PERFORM ACTION STATE");
		break;
	}
}

void MainGameMode::OnStateExit(MODE_STATE newState)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (m_State)
	{
		//////////////////////////
		/// Camera Move States ///
		//////////////////////////

	case MODE_STATE::FREE_NAVIGATION:
		break;
	case MODE_STATE::MOVE_MENU_OPEN:
		break;
	case MODE_STATE::FIND_SKILL_TARGET:
		break;
		///////////////////
		/// Menu States ///
		///////////////////

	case MODE_STATE::UNIT_MENU_OPEN:		
		break;
	case MODE_STATE::ACTION_MENU_OPEN:
		break;
	case MODE_STATE::ATTACK_MENU_OPEN:
		break;
	case MODE_STATE::STATUS_FRAME_OPEN:
		break;
	case MODE_STATE::UNIT_PERFORM_ACTION_STATE:
		DBOUT("EXITING PERFORM ACTION STATE");
		break;
	}

	//Shift State BEFORE changes
	m_State = newState;

}

void MainGameMode::CheckIfTeamActionsSpent()
{
	//PH - Use const team count when added
	int unitCount = 0;
	int count = 5;

	if (m_CurrentTeamID == 1)
	{
		for (auto& a : m_TeamOne)
		{
			//If the unit has no actions, or is not alive, uptick the count
			if (!a->CheckRemainingActions() || !a->GetUnitStateFlags().isAlive)
			{
				++unitCount;
			}
		}
	}
	else
	{
		for (auto& a : m_TeamTwo)
		{
			//If the unit has no actions, or is not alive, uptick the count
			if (!a->CheckRemainingActions() || !a->GetUnitStateFlags().isAlive)
			{
				++unitCount;
			}
		}
	}

	if (unitCount >= count)
		m_SwitchTeams = true;

}

void MainGameMode::SwitchTeams()
{
	//Check which team is active before the switch and clean up their units on on the way out
	if (m_CurrentTeamID == 1)
		for (auto& a : m_TeamOne)
		{
			if (a->GetAliveState() == true)
			{
				a->EnableUnit();
				a->GetPrimarySprite().SetColour(TEAM_ONE_COLOUR);
				if (a->GetBuffState())
				{
					if (a->GetBuffs().TurnCount > 0)
					{
						a->GetBuffs().TurnCount -= 1;
					}
					else
					{
						a->ResetBuffValues();
					}
				}
			}

		}	
	else
		for (auto& a : m_TeamTwo)
		{
			if (a->GetAliveState() == true)
			{
				a->EnableUnit();
				a->GetPrimarySprite().SetColour(TEAM_TWO_COLOUR);
				if (a->GetBuffState())
				{
					if (a->GetBuffs().TurnCount > 0)
					{
						a->GetBuffs().TurnCount -= 1;
					}
					else
					{
						a->ResetBuffValues();
					}
				}
			}
		}

	//Switch teams
	ChangeTeamID();
	//Snap the camera to the first alive unit on team
	SnapCameraToUnit();
	//Update cursor accordingly
	UpdateCursorFreeMode();
	//Flag operation as done
	m_SwitchTeams = false;
}

void MainGameMode::ProcessKeyFreeMoveState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		//Open System Menu?
		break;

	case 'f':
	case 'F':
		//Check if cursor is on a unit, if the unit is alive, can act and is of the same team as currently playing team
		if (m_Cursor->GetCurrentObject() && static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetAliveState() &&
			static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetUnitTeamID() == m_CurrentTeamID &&
			static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->CheckRemainingActions())
		{
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);

			//Get Unit Menu and setup
			SetupUnitMenu(m_UnitMenu, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()));
			m_UnitMenu->ResetCursor();

			//Change State
			m_State = MODE_STATE::UNIT_MENU_OPEN;
		}
		//If no unit found, must be on an empty tile so open non-unit menu
		else if (!m_Cursor->GetCurrentObject())
		{
			//Change State
			m_State = MODE_STATE::NON_UNIT_MENU_OPEN;
		}
		break;

	case 'w':
	case 'W':
		m_Cursor->DecrementYCoordinate();
		m_Camera.MoveSceneDown();
		UpdateCursorFreeMode();
		break;

	case 's':
	case 'S':
		m_Cursor->IncrementYCoordinate();
		m_Camera.MoveSceneUp();
		UpdateCursorFreeMode();
		break;

	case 'a':
	case 'A':
		m_Cursor->DecrementXCoordinate();
		m_Camera.MoveSceneRight();
		UpdateCursorFreeMode();
		break;

	case 'd':
	case 'D':
		m_Cursor->IncrementXCoordinate();
		m_Camera.MoveSceneLeft();
		UpdateCursorFreeMode();
		break;

	}
}

void MainGameMode::ProcessKeyUnitMenuOpenState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		//Update Cursor
		UpdateCursorFreeMode();
		//Change State
		m_State = MODE_STATE::FREE_NAVIGATION;
		break;

	case 'f':
	case 'F':
	
		//Check what menu index is, and change menu based on it
		switch (m_UnitMenu->GetMenuIndex().y)
		{
		case 0:  //Action
			//Check if the unit can act, and open menu if so
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetUnitStateFlags().canAct)
			{
				//Play Select Audio
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
				m_SkillIndex = 0;
				UpdateSkillTooltip();
				m_ActionMenu->ResetCursor();

				//Change State
				m_State = MODE_STATE::ACTION_MENU_OPEN;
			}			
			break;

		case 1:  //Move
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetUnitStateFlags().canMove)
			{
				//Play Select Audio
				Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
				//Generate movement grid from unit position
				m_PathFinder.GenerateTileGrid(m_TileMap, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), m_MapLimit.x);
				//Change State
				m_State = MODE_STATE::MOVE_MENU_OPEN;
			}
			break;

		case 2:  //Status
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			//Open Status/Equip Menu Here
			UpdateStatusFrame(m_StatusFrame, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()));
			m_State = MODE_STATE::STATUS_FRAME_OPEN;
			break;

		case 3:  //End Units Turn
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);

			//End units turn, check if team actions spend and change state
			static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->DisableUnit();
			CheckIfTeamActionsSpent();
			m_State = MODE_STATE::FREE_NAVIGATION;
			break;

		}
		break;
	
	case 'w':
	case 'W':
		m_UnitMenu->MoveUp();
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		break;

	case 's':
	case 'S':
		m_UnitMenu->MoveDown();
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		break;
	}
}

void MainGameMode::ProcessKeyUnitActionOpenState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		//Change state to Unit Menu
		m_State = MODE_STATE::UNIT_MENU_OPEN;
		break;

	case 'f':
	case 'F':
		//Get Menu Index, execute action based on index
		switch (m_ActionMenu->GetMenuIndex().y)
		{
		case 0:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			m_SkillIndex = m_ActionMenu->GetMenuIndex().y;
			if(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetClassTotals().CurrentMP >=
				static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetManaCost())
				SwitchUnitMenuToTargetingMode(manager, MODE_STATE::FIND_SKILL_TARGET);
			break;
		case 1:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			m_SkillIndex = m_ActionMenu->GetMenuIndex().y;
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetClassTotals().CurrentMP >=
				static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetManaCost())
				SwitchUnitMenuToTargetingMode(manager, MODE_STATE::FIND_SKILL_TARGET);
			break;

		case 2:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			m_SkillIndex = m_ActionMenu->GetMenuIndex().y;
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetClassTotals().CurrentMP >=
				static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetManaCost())
				SwitchUnitMenuToTargetingMode(manager, MODE_STATE::FIND_SKILL_TARGET);
			break;
		case 3:
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			m_SkillIndex = m_ActionMenu->GetMenuIndex().y;
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetClassTotals().CurrentMP >=
				static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetManaCost())
				SwitchUnitMenuToTargetingMode(manager, MODE_STATE::FIND_SKILL_TARGET);
			break;
		}
		break;

	case 'w':
	case 'W':
		//Move menu and update tooltip accordingly
		m_ActionMenu->MoveUp();
		m_SkillIndex = m_ActionMenu->GetMenuIndex().y;
		UpdateSkillTooltip();

		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		break;

	case 's':
	case 'S':
		//Move menu and update tooltip accordingly
		m_ActionMenu->MoveDown();
		m_SkillIndex = m_ActionMenu->GetMenuIndex().y;
		UpdateSkillTooltip();

		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		break;
	}
}

void MainGameMode::ProcessKeyMoveMenuOpenState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		//Snap cursor & camera back to the unit
		m_Cursor->MoveFixedCursorCoords(m_Cursor->GetCurrentObject()->GetMapCoordinates());
		m_Camera.MoveCameraToCoordinates(m_Cursor->GetCurrentObject()->GetMapCoordinates());
		//Release current manifest
		m_PathFinder.ReleaseManifest();
		//Change State
		m_State = MODE_STATE::UNIT_MENU_OPEN;
		break;

	case 'f':
	case 'F':
		//If current highlighted tile is in the grid
		if (m_PathFinder.IsTileInGrid(m_Cursor->GetCurrentTileObject()))
		{
			//Play Select Audio
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			//Flag that current tile is no longer occupied
			MapTile::FindTileInArray(m_TileMap, m_Cursor->GetCurrentObject()->GetMapCoordinates(),
				m_MapLimit.x)->GetTileProperties().occupied = false;

			//Move unit
			static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->MoveToCoordinate(
				m_Cursor->GetCurrentTileObject()->GetMapCoordinates());

			//Flag current tile as occupied
			MapTile::FindTileInArray(m_TileMap, m_Cursor->GetCurrentObject()->GetMapCoordinates(),
				m_MapLimit.x)->GetTileProperties().occupied = true;

			//Release current manifest
			m_PathFinder.ReleaseManifest();

			//Flag the unit as unable to move, then decide on what to do based on flags
			static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->SetMoveState(false);
			//If the unit has no remaining actions
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->CheckRemainingActions() == false)
			{
				static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->DisableUnit();
				m_State = MODE_STATE::FREE_NAVIGATION;
				CheckIfTeamActionsSpent();
			}
			else //Revert to unit menu
			{
				//Change state back to Unit Menu
				SetupUnitMenu(m_UnitMenu, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()));
				m_State = MODE_STATE::UNIT_MENU_OPEN;
			}
		}
		break;

	case 'w':
	case 'W':
		m_Cursor->DecrementYCoordinate();
		m_Camera.MoveSceneDown();
		if (m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x))
		{
			UpdateTileTooltip(m_TileTooltip, m_Cursor->GetCurrentTileObject());

			//Run A*

			//If the tile highlighted is in the grid, attempt to path find
			if (m_PathFinder.IsTileInGrid(m_Cursor->GetCurrentTileObject()))
			{
				m_PathFinder.ResetGridEffectToDefault();
				const XMINT2& coords = m_Cursor->GetCurrentObject()->GetMapCoordinates();
				MapTile* tile = m_TileMap.at((coords.x + (coords.y)) + ((coords.y) * m_MapLimit.y));
				m_PathFinder.RunAStarAlgorithm(tile, m_Cursor->GetCurrentTileObject());

			}
			//The cursor has likely left the grid, so remove the effect
			else
			{
				m_PathFinder.ResetGridEffectToDefault();
			}

		}
		break;

	case 's':
	case 'S':
		m_Cursor->IncrementYCoordinate();
		m_Camera.MoveSceneUp();
		if (m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x))
		{
			UpdateTileTooltip(m_TileTooltip, m_Cursor->GetCurrentTileObject());

			//Run A*

			//If the tile highlighted is in the grid, attempt to path find
			if (m_PathFinder.IsTileInGrid(m_Cursor->GetCurrentTileObject()))
			{
				m_PathFinder.ResetGridEffectToDefault();
				const XMINT2& coords = m_Cursor->GetCurrentObject()->GetMapCoordinates();
				MapTile* tile = m_TileMap.at((coords.x + (coords.y)) + ((coords.y) * m_MapLimit.y));
				m_PathFinder.RunAStarAlgorithm(tile, m_Cursor->GetCurrentTileObject());

			}
			//The cursor has likely left the grid, so remove the effect
			else
			{
				m_PathFinder.ResetGridEffectToDefault();
			}

		}
		break;

	case 'a':
	case 'A':
		m_Cursor->DecrementXCoordinate();
		m_Camera.MoveSceneRight();
		if (m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x))
		{
			UpdateTileTooltip(m_TileTooltip, m_Cursor->GetCurrentTileObject());

			//Run A*

			//If the tile highlighted is in the grid, attempt to path find
			if (m_PathFinder.IsTileInGrid(m_Cursor->GetCurrentTileObject()))
			{
				m_PathFinder.ResetGridEffectToDefault();
				const XMINT2& coords = m_Cursor->GetCurrentObject()->GetMapCoordinates();
				MapTile* tile = m_TileMap.at((coords.x + (coords.y)) + ((coords.y) * m_MapLimit.y));
				m_PathFinder.RunAStarAlgorithm(tile, m_Cursor->GetCurrentTileObject());

			}
			//The cursor has likely left the grid, so remove the effect
			else
			{
				m_PathFinder.ResetGridEffectToDefault();

				//Run A*

				//If the tile highlighted is in the grid, attempt to path find
				if (m_PathFinder.IsTileInGrid(m_Cursor->GetCurrentTileObject()))
				{
					m_PathFinder.ResetGridEffectToDefault();
					const XMINT2& coords = m_Cursor->GetCurrentObject()->GetMapCoordinates();
					MapTile* tile = m_TileMap.at((coords.x + (coords.y)) + ((coords.y) * m_MapLimit.y));
					m_PathFinder.RunAStarAlgorithm(tile, m_Cursor->GetCurrentTileObject());

				}
				//The cursor has likely left the grid, so remove the effect
				else
				{
					m_PathFinder.ResetGridEffectToDefault();
				}

			}

		}
		break;

	case 'd':
	case 'D':
		m_Cursor->IncrementXCoordinate();
		m_Camera.MoveSceneLeft();
		if (m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x))
		{
			UpdateTileTooltip(m_TileTooltip, m_Cursor->GetCurrentTileObject());

			//Run A*

			//If the tile highlighted is in the grid, attempt to path find
			if (m_PathFinder.IsTileInGrid(m_Cursor->GetCurrentTileObject()))
			{
				m_PathFinder.ResetGridEffectToDefault();
				const XMINT2& coords = m_Cursor->GetCurrentObject()->GetMapCoordinates();
				MapTile* tile = m_TileMap.at((coords.x + (coords.y)) + ((coords.y) * m_MapLimit.y));
				m_PathFinder.RunAStarAlgorithm(tile, m_Cursor->GetCurrentTileObject());

			}
			//The cursor has likely left the grid, so remove the effect
			else
			{
				m_PathFinder.ResetGridEffectToDefault();
			}

		}
		break;

	}
}

void MainGameMode::ProcessKeyNonUnitMenuOpenState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		//Update Cursor
		UpdateCursorFreeMode();
		//Change State
		m_State = MODE_STATE::FREE_NAVIGATION;
		break;

	case 'f':
	case 'F':
		//Get Menu Index, execute action based on index
		switch (m_EndTurnMenu->GetMenuIndex().y)
		{
		case 0:  //End Turn

			Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
			//Run team switch process
			SwitchTeams();
			//Change State
			m_State = MODE_STATE::FREE_NAVIGATION;
			break;
		}
		break;

	case 'w':
	case 'W':

		Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		m_EndTurnMenu->MoveUp();
		break;

	case 's':
	case 'S':

		Game::GetGame()->GetAudioManager().PlayOneShot(Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		m_EndTurnMenu->MoveDown();
		break;

	}
}

void MainGameMode::ProcessKeyStatusFrameOpenState(char key)
{
	switch (key)
	{
	case VK_ESCAPE:
		//Change state back to Unit Menu
		m_State = MODE_STATE::UNIT_MENU_OPEN;
		break;
	}
}

void MainGameMode::ProcessKeyFindTargetState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		//Snap cursor & camera back to the unit
		m_Cursor->MoveFixedCursorCoords(m_Cursor->GetCurrentObject()->GetMapCoordinates());
		m_Camera.MoveCameraToCoordinates(m_Cursor->GetCurrentObject()->GetMapCoordinates());
		//Release current manifest
		m_PathFinder.ReleaseManifest();

		//Disable Targeting Grid
		m_TargetingSystem.DisableGrid();

		//Change State
		m_State = MODE_STATE::UNIT_MENU_OPEN;
		break;

	case 'f':
	case 'F':
		//Check if the cursor is currently holding a unit
		if (m_Cursor->GetSecondObject() && static_cast<UnitEntity*>(m_Cursor->GetSecondObject())->GetAliveState() && m_TargetingSystem.IsTargetInGrid(m_Cursor->GetSecondObject()->GetMapCoordinates()))
		{
			switch (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetTargeting())
			{
			case SkillTarget::ALL_UNITS:
				//Play Select Audio
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);

				Game::GetGame()->GetGameplayManager().BeforeStateEnter(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
					static_cast<UnitEntity*>(m_Cursor->GetSecondObject()), m_TileMap,m_MapLimit.x, m_SkillIndex);
				m_State = MODE_STATE::UNIT_PERFORM_ACTION_STATE;
				break;
			case SkillTarget::FRIENDLY_UNITS:
				if (static_cast<UnitEntity*>(m_Cursor->GetSecondObject())->GetUnitTeamID() == m_CurrentTeamID)
				{
					//Checks if the selected skill is a buff
					if (static_cast<UnitEntity*>(m_Cursor->GetSecondObject())->GetSkillAtIndex(m_SkillIndex)->GetSkillType() == 2)
					{
						/*
						* Checks to see if the unit has a buff active
						* If buffActive is true, break out of the statement to go back to targeting
						* If buffActive is false apply the buff to the unit
						*/
						switch (static_cast<UnitEntity*>(m_Cursor->GetSecondObject())->GetUnitStateFlags().buffActive)
						{
						case true:
							break;
						case false:
							//Play Select Audio
							Game::GetGame()->GetAudioManager().PlayOneShot(
								Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
							Game::GetGame()->GetGameplayManager().BeforeStateEnter(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
								static_cast<UnitEntity*>(m_Cursor->GetSecondObject()), m_TileMap, m_MapLimit.x, m_SkillIndex);
							m_State = MODE_STATE::UNIT_PERFORM_ACTION_STATE;
							break;
						}
					}
					else
					{
						//Play Select Audio
						Game::GetGame()->GetAudioManager().PlayOneShot(
							Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
						Game::GetGame()->GetGameplayManager().BeforeStateEnter(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
							static_cast<UnitEntity*>(m_Cursor->GetSecondObject()), m_TileMap, m_MapLimit.x, m_SkillIndex);
						m_State = MODE_STATE::UNIT_PERFORM_ACTION_STATE;
					}
				}
				else
				{
					DBOUT("WRONG TARGET");
				}
				break;
			case SkillTarget::ENEMY_UNITS:
				if (static_cast<UnitEntity*>(m_Cursor->GetSecondObject())->GetUnitTeamID() != m_CurrentTeamID)
				{
					//Play Select Audio
					Game::GetGame()->GetAudioManager().PlayOneShot(
						Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
					Game::GetGame()->GetGameplayManager().BeforeStateEnter(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
						static_cast<UnitEntity*>(m_Cursor->GetSecondObject()), m_TileMap, m_MapLimit.x, m_SkillIndex);
					m_State = MODE_STATE::UNIT_PERFORM_ACTION_STATE;
				}
				else
				{
					DBOUT("WRONG TARGET");
				}
				break;
			}
		}

		break;
	case 'w':
	case 'W':
		m_Cursor->DecrementYCoordinate();
		m_Camera.MoveSceneDown();
		UpdateCursorTargetingMode();

		break;
	case 's':
	case 'S':
		m_Cursor->IncrementYCoordinate();
		m_Camera.MoveSceneUp();
		UpdateCursorTargetingMode();

		break;
	case 'a':
	case 'A':
		m_Cursor->DecrementXCoordinate();
		m_Camera.MoveSceneRight();
		UpdateCursorTargetingMode();

		break;
	case 'd':
	case 'D':
		m_Cursor->IncrementXCoordinate();
		m_Camera.MoveSceneLeft();
		UpdateCursorTargetingMode();

		break;
	}
}

bool MainGameMode::SetupSelectedUnit()
{
	Game* game = Game::GetGame();

	//Get hold of class
	

	//First check that the tile is valid and inside the grid
	if (!m_Cursor->GetCurrentTileObject()->GetTileProperties().impassable &&
		!m_Cursor->GetCurrentTileObject()->GetTileProperties().occupied &&
		IsTileInPlacementGrid(m_Cursor->GetCurrentTileObject()))
	{
		//Hold a unit pointer
		UnitEntity* unit = nullptr;
		//Based on current team selection, grab a unit from the teams unit pool
		if (m_CurrentTeamID == 1)
		{
			unit = m_TeamOne.at(m_PlacedUnitCount);
			unit->GetPrimarySprite().SetColour(XMVECTOR(TEAM_ONE_COLOUR));
		}
		else
		{
			unit = m_TeamTwo.at(m_PlacedUnitCount);
			unit->GetPrimarySprite().SetColour(TEAM_TWO_COLOUR);
			unit->GetPrimarySprite().SetEffect(SpriteEffects::SpriteEffects_FlipHorizontally);
		}


		//Setup General Details
		unit->GetEntityData().isActive = true;
		unit->MoveToCoordinate(m_Cursor->GetMapCoordinates());

		//Setup the units class based on selection ID
		switch (m_UnitSelectorClassID)
		{
		case GameClasses::ASSASSIN:
			unit->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00"))
			);
			unit->SetUnitClass(GameClasses::ASSASSIN);
			unit->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::IDLE_00), false, true, false);
			unit->GetPrimarySprite().SetOriginToCenter();
			unit->SetUnitBaseEquipment(0, 0, 7, 16, 18);
			unit->InitTotalValues();
			unit->SetSkillsBasedOnClass();

			break;

		case GameClasses::BARBARIAN:
			unit->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("BARBARIAN_CLASS_SS_00"))
			);
			unit->SetUnitClass(GameClasses::BARBARIAN);
			unit->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::IDLE_00), false, true, false);
			unit->GetPrimarySprite().SetOriginToCenter();
			unit->SetUnitBaseEquipment(3, 1, 8, 13, 14);
			unit->InitTotalValues();
			unit->SetSkillsBasedOnClass();

			break;

		case GameClasses::ARCHER:
			unit->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("ARCHER_CLASS_SS_00"))
			);
			unit->SetUnitClass(GameClasses::ARCHER);
			unit->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::IDLE_00), false, true, false);
			unit->GetPrimarySprite().SetOriginToCenter();
			unit->SetUnitBaseEquipment(4, 0, 7, 15, 11);
			unit->InitTotalValues();
			unit->SetSkillsBasedOnClass();

			break;

		case GameClasses::NECROMANCER:
			unit->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("NECROMANCER_CLASS_SS_00"))
			);
			unit->SetUnitClass(GameClasses::NECROMANCER);
			unit->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::IDLE_00), false, true, false);
			unit->GetPrimarySprite().SetOriginToCenter();
			unit->SetUnitBaseEquipment(5, 0, 7, 12, 10);
			unit->InitTotalValues();
			unit->SetSkillsBasedOnClass();

			break;

		case GameClasses::SORCERESS:
			unit->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
			);
			unit->SetUnitClass(GameClasses::SORCERESS);
			unit->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::IDLE_00), false, true, false);
			unit->GetPrimarySprite().SetOriginToCenter();
			unit->SetUnitBaseEquipment(6, 0, 7, 17, 14);
			unit->InitTotalValues();
			unit->SetSkillsBasedOnClass();

			break;

		case GameClasses::WARRIOR:
			unit->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("WARRIOR_CLASS_SS_00"))
			);
			unit->SetUnitClass(GameClasses::WARRIOR);
			unit->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::IDLE_00), false, true, false);
			unit->GetPrimarySprite().SetOriginToCenter();
			unit->SetUnitBaseEquipment(2, 1, 9, 13, 15);
			unit->InitTotalValues();
			unit->SetSkillsBasedOnClass();

			break;
		}

		m_Cursor->GetCurrentTileObject()->GetTileProperties().occupied = true;

		return true;
	}

	return false;
}

void MainGameMode::StartFreshGame()
{
	//Get Game Handle
	Game* game = Game::GetGame();
	//Grab the UI manager for use
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	//Reset units to default

	for (auto& a : m_TeamOne)
	{
		a->GetEntityData().isActive = false;
		a->SetAliveState(true);
		a->SetActState(true);
		a->SetMoveState(true);
	}

	for (auto& a : m_TeamTwo)
	{
		a->GetEntityData().isActive = false;
		a->SetAliveState(true);
		a->SetActState(true);
		a->SetMoveState(true);
	}

	for (auto& t : m_TileMap)
	{
		t->GetTileProperties().occupied = false;
	}

	//Init Navigation Elements
	manager->GetNavigationMenuByTypeID(m_UnitMenu, UIElementIDs::UNIT_MENU_00);
	manager->GetNavigationMenuByTypeID(m_ActionMenu, UIElementIDs::UNIT_ACTION_MENU_00);
	manager->GetNavigationMenuByTypeID(m_EndTurnMenu, UIElementIDs::NON_UNIT_MENU_00);


	//Init Non-Navigation Elements

	game->GetUIManager().GetNonNavigationUIByTypeID(m_TileTooltip, UIElementIDs::TOOLTIP_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_FriendlyUnitUI, UIElementIDs::FRIENDLY_UNIT_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_EnemyUnitUI, UIElementIDs::ENEMY_UNIT_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_TargetFrame, UIElementIDs::DECLARE_TARGET_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_SkillTooltip, UIElementIDs::ABILITY_TOOLTIP_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_StatusFrame, UIElementIDs::UNIT_STATUS_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_ArrowLeft, UIElementIDs::UNIT_SELECTION_ARROW_LEFT_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_ArrowRight, UIElementIDs::UNIT_SELECTION_ARROW_RIGHT_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_SelectionFrame, UIElementIDs::UNIT_SELECTION_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_PlacementFrame, UIElementIDs::UNIT_PLACEMENT_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_TeamFrame, UIElementIDs::TEAM_SELECTION_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_GameOverFrame, UIElementIDs::GAME_OVER_FRAME_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_ContextControlGame, UIElementIDs::CONTEXT_CONTROLS_00);
	game->GetUIManager().GetNonNavigationUIByTypeID(m_ContextControlCharSelect, UIElementIDs::CONTEXT_CONTROLS_02);


	UpdateTileTooltip(m_TileTooltip, m_Cursor->GetCurrentTileObject());

	game->GetUIManager().GetNonNavigationUIByTypeID(m_TeamFrame, UIElementIDs::TEAM_SELECTION_FRAME_00);
	m_CurrentTeamID = 1;
	std::stringstream ss;
	ss << m_CurrentTeamID;
	m_TeamFrame->ResetDrawStrings();
	m_TeamFrame->AppendToStringByIndex(ss, 0);
	game->GetGameplayManager().Init(game->GetWindowData().clientWidth, game->GetWindowData().clientHeight);
	game->GetGameplayManager().SetHasAnimPlayed(false);
	InitModeAudio();

	m_TargetingSystem.DisableGrid();
	m_TargetingSystem.DisableAoEGrid();

	m_PlacedUnitCount = 0;
	m_TeamOneAliveCount = TEAM_SIZE;
	m_TeamTwoAliveCount = TEAM_SIZE;
	m_PlacedUnitCount = 0;

	m_SkillIndex = 0;

	m_State = MODE_STATE::UNIT_SELECTION;

}

void MainGameMode::ReleaseAllUIElements()
{
	//Grab the UI manager for use
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	//Navigational Elements
	manager->ReleaseNavUIElement(m_UnitMenu);
	manager->ReleaseNavUIElement(m_ActionMenu);

	//Non-Navigational Elements

	manager->ReleaseNonNavUIElement(m_TileTooltip);
	manager->ReleaseNonNavUIElement(m_FriendlyUnitUI);
	manager->ReleaseNonNavUIElement(m_EnemyUnitUI);
	manager->ReleaseNonNavUIElement(m_TargetFrame);
	manager->ReleaseNonNavUIElement(m_SkillTooltip);
	manager->ReleaseNonNavUIElement(m_StatusFrame);
	manager->ReleaseNonNavUIElement(m_ArrowLeft);
	manager->ReleaseNonNavUIElement(m_ArrowRight);
	manager->ReleaseNonNavUIElement(m_SelectionFrame);
	manager->ReleaseNonNavUIElement(m_PlacementFrame);
	manager->ReleaseNonNavUIElement(m_TeamFrame);
	manager->ReleaseNonNavUIElement(m_GameOverFrame);
	manager->ReleaseNonNavUIElement(m_ContextControlGame);
	manager->ReleaseNonNavUIElement(m_ContextControlCharSelect);	
}

void MainGameMode::ChangeDisplayUnit()
{
	Game* game = Game::GetGame();

	//Setup the unit display based off class ID
	switch (m_UnitSelectorClassID)
	{
	case GameClasses::ASSASSIN:
		m_UnitSelectionDisplayUnit->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00"))
		);
		m_UnitSelectionDisplayUnit->SetUnitClass(GameClasses::ASSASSIN);
		m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::IDLE_00));

		break;

	case GameClasses::BARBARIAN:
		m_UnitSelectionDisplayUnit->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("BARBARIAN_CLASS_SS_00"))
		);
		m_UnitSelectionDisplayUnit->SetUnitClass(GameClasses::BARBARIAN);
		m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::IDLE_00));

		break;

	case GameClasses::ARCHER:
		m_UnitSelectionDisplayUnit->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ARCHER_CLASS_SS_00"))
		);
		m_UnitSelectionDisplayUnit->SetUnitClass(GameClasses::ARCHER);
		m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::IDLE_00));

		break;

	case GameClasses::NECROMANCER:
		m_UnitSelectionDisplayUnit->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("NECROMANCER_CLASS_SS_00"))
		);
		m_UnitSelectionDisplayUnit->SetUnitClass(GameClasses::NECROMANCER);
		m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::IDLE_00));


		break;

	case GameClasses::SORCERESS:
		m_UnitSelectionDisplayUnit->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
		);
		m_UnitSelectionDisplayUnit->SetUnitClass(GameClasses::SORCERESS);
		m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::IDLE_00));

		break;

	case GameClasses::WARRIOR:
		m_UnitSelectionDisplayUnit->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("WARRIOR_CLASS_SS_00"))
		);
		m_UnitSelectionDisplayUnit->SetUnitClass(GameClasses::WARRIOR);
		m_UnitSelectionDisplayUnit->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::IDLE_00));

		break;

	}
}

void MainGameMode::SetupUnitMenu(NavigationalMenu* menu, UnitEntity* unit)
{
	//If the unit cannot do actions, change colour of string to indicate
	if (!unit->GetActState())
		menu->SetColourAtIndex(XMVECTOR(NAVIGATABLE_MENU_DISABLED_COLOUR), 0);
	else
		menu->SetColourAtIndex(XMVECTOR(NAVIGATABLE_MENU_DEFAULT_COLOUR), 0);

	if (!unit->GetMoveState())
		menu->SetColourAtIndex(XMVECTOR(NAVIGATABLE_MENU_DISABLED_COLOUR), 1);
	else
		menu->SetColourAtIndex(XMVECTOR(NAVIGATABLE_MENU_DEFAULT_COLOUR), 1);

}

void MainGameMode::SetupGameOverMenu(int teamWinID)
{
	switch (teamWinID)
	{
	case 0:
		m_GameOverFrame->ChangeStringByIndex(std::string("It's a Draw!"), 0);
		break;
	case 1:
		m_GameOverFrame->ChangeStringByIndex(std::string("Team 1 Wins!"), 0);
		break;
	case 2:
		m_GameOverFrame->ChangeStringByIndex(std::string("Team 2 Wins!"), 0);
		break;
	}
}

void MainGameMode::EnablePlacementGrid()
{
	switch (m_CurrentTeamID)
	{
	case 1:
		for (int x(0); x < m_PlacementGridSize.x; ++x)
		{
			for (int y(0); y < m_PlacementGridSize.y; ++y)
			{

				//			t->SetNeighbourAtIndex(0, m_TileMap.at((coords.x + (coords.y - 1)) + ((coords.y - 1) * m_MapLimit.y)));

				int index = (m_TeamOneGridStart.x + m_TeamOneGridStart.y + x + y) + ((m_TeamOneGridStart.y + y) * m_MapLimit.y);

				MapTile* tile = m_TileMap.at(index);
				tile->SetDrawGridFlag(true);
				tile->GetGridSprite().SetFrame(UI_ATLAS_01_FRAMES::MOVE_TILE_HIGHLIGHT);
				m_PlacementGridTiles.push_back(tile);
			}
		}

		break;
	case 2:
		for (int x(0); x < m_PlacementGridSize.x; ++x)
		{
			for (int y(0); y < m_PlacementGridSize.y; ++y)
			{

				//			t->SetNeighbourAtIndex(0, m_TileMap.at((coords.x + (coords.y - 1)) + ((coords.y - 1) * m_MapLimit.y)));

				int index = (m_TeamTwoGridStart.x + m_TeamTwoGridStart.y + x + y) + ((m_TeamTwoGridStart.y + y) * m_MapLimit.y);

				MapTile* tile = m_TileMap.at(index);
				tile->SetDrawGridFlag(true);
				tile->GetGridSprite().SetFrame(UI_ATLAS_01_FRAMES::MOVE_TILE_HIGHLIGHT);
				m_PlacementGridTiles.push_back(tile);
			}
		}

		break;
	}
}

void MainGameMode::DisablePlacementGrid()
{
	for (auto& a : m_PlacementGridTiles)
	{
		a->SetDrawGridFlag(false);
		a = nullptr;
	}

	m_PlacementGridTiles.clear();
}

bool MainGameMode::IsTileInPlacementGrid(MapTile* tile)
{
	for (auto& a : m_PlacementGridTiles)
	{
		//Return true if tile match found
		if (tile == a)
			return true;
	}
	
	//No match so return false
	return false;
}

void MainGameMode::ProcessKeyUnitSelectionState(char key)
{
	switch (key)
	{
	case VK_ESCAPE:
		break;
	case 'f':
	case 'F':
		//Play Select Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
		//Snap the camera to a starting position based on current team selecting
		if (m_CurrentTeamID == 1)
		{
			//Move camera, cursor and update cursor
			m_Cursor->MoveFixedCursorCoords(m_TeamOneCameraPos);
			m_Camera.MoveCameraToCoordinates(m_TeamOneCameraPos);
			m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);
			//Enable placement grid
			EnablePlacementGrid();
		}
		else
		{
			//Move camera, cursor and update cursor
			m_Cursor->MoveFixedCursorCoords(m_TeamTwoCameraPos);
			m_Camera.MoveCameraToCoordinates(m_TeamTwoCameraPos);
			m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);
			EnablePlacementGrid();

		}
		m_Cursor->GetCurrentTileObject();

		//Change State to unit placement
		m_State = MODE_STATE::UNIT_PLACEMENT;

		break;

	case 'a':
	case 'A':
		//Play Nav Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		//Downtick current ID
		--m_UnitSelectorClassID;

		//Range Check
		if (m_UnitSelectorClassID < 0)
			m_UnitSelectorClassID = 0;
		else if (m_UnitSelectorClassID > 5)
			m_UnitSelectorClassID = 5;

		ChangeDisplayUnit();

		break;

	case 'd':
	case 'D':
		//Play Nav Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX0);
		//Uptick current ID
		++m_UnitSelectorClassID;

		//Range Check
		if (m_UnitSelectorClassID < 0)
			m_UnitSelectorClassID = 0;
		else if (m_UnitSelectorClassID > 5)
			m_UnitSelectorClassID = 5;

		ChangeDisplayUnit();

		break;
	}
}

void MainGameMode::ProcessKeyUnitPlacementState(char key)
{
	switch (key)
	{
	case VK_ESCAPE:

		//Change State to unit placement
		m_State = MODE_STATE::UNIT_SELECTION;
		break;
	case 'f':
	case 'F':
		//Play Select Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX1);
		//Setup Unit at Current Coordinates
		if (SetupSelectedUnit())
		{
			//Update the placed count and disable the placement grid
			++m_PlacedUnitCount;
			DisablePlacementGrid();

			//If Team One is done selecting, move to team two
			if (m_PlacedUnitCount == TEAM_SIZE && m_CurrentTeamID == 1)
			{
				m_CurrentTeamID = 2;
				m_PlacedUnitCount = 0;
				m_Cursor->MoveFixedCursorCoords(m_TeamTwoCameraPos);
				m_Camera.MoveCameraToCoordinates(m_TeamTwoCameraPos);

				UpdateTeamFrame();

				m_State = MODE_STATE::UNIT_SELECTION;
			}
			//If Team Two is done selecting, move to game
			else if (m_PlacedUnitCount == TEAM_SIZE && m_CurrentTeamID == 2)
			{
				//Enable Animations
				for (int i(0); i < TEAM_SIZE; i++)
				{
					m_TeamOne.at(i)->GetPrimarySprite().GetAnimator().Play(true);
					m_TeamTwo.at(i)->GetPrimarySprite().GetAnimator().Play(true);
				}

				m_CurrentTeamID = m_StartingTeam;
								
				//Snap camera to first seen unit for current team
				if (!SnapCameraToUnit())
				{
					m_Cursor->MoveFixedCursorCoords(m_DefaultCamPos);
					m_Camera.MoveCameraToCoordinates(m_DefaultCamPos);
				}
				UpdateTeamFrame();

				m_State = MODE_STATE::FREE_NAVIGATION;
			}
			//Current team still has choices to make so go back to selection
			else
			{
				m_State = MODE_STATE::UNIT_SELECTION;
			}
		}
		break;

	case 'w':
	case 'W':
		m_Cursor->DecrementYCoordinate();
		m_Camera.MoveSceneDown();
		m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);

		break;
	case 's':
	case 'S':
		m_Cursor->IncrementYCoordinate();
		m_Camera.MoveSceneUp();
		m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);


		break;
	case 'a':
	case 'A':
		m_Cursor->DecrementXCoordinate();
		m_Camera.MoveSceneRight();
		m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);


		break;
	case 'd':
	case 'D':
		m_Cursor->IncrementXCoordinate();
		m_Camera.MoveSceneLeft();
		m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x);


		break;
	}
}

void MainGameMode::ProcessKeyPerformActionState(char key)
{
	UIElementManager* manager = &Game::GetGame()->GetUIManager();

	switch (key)
	{
	case VK_ESCAPE:
		if (Game::GetGame()->GetGameplayManager().GetHasAnimPlayed())
		{
			PostActionCleanUp();
		}
		else
		{
			//Snap cursor & camera back to the unit
			m_Cursor->MoveFixedCursorCoords(m_Cursor->GetCurrentObject()->GetMapCoordinates());
			m_Camera.MoveCameraToCoordinates(m_Cursor->GetCurrentObject()->GetMapCoordinates());
			//Release current manifest
			m_PathFinder.ReleaseManifest();

			//Disable Targeting Grid
			m_TargetingSystem.DisableGrid();
			m_TargetingSystem.DisableAoEGrid();
			//Change State
			m_State = MODE_STATE::UNIT_MENU_OPEN;
		}
		break;
	case 'f':
	case'F':
		if (!Game::GetGame()->GetGameplayManager().GetHasAnimPlayed())
		{
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetSkillRadius() > 0)
			{
				AOE();
				Game::GetGame()->GetGameplayManager().ActionStateAnimationsHandler(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
					static_cast<UnitEntity*>(m_Cursor->GetSecondObject()), m_SkillIndex);
				Game::GetGame()->GetGameplayManager().ManaReductions(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), m_SkillIndex);
				Game::GetGame()->GetGameplayManager().SetHasAnimPlayed(true);
			}
			else
			{
				UseSelectedSkillOnTarget(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
					static_cast<UnitEntity*>(m_Cursor->GetSecondObject()));
				Game::GetGame()->GetGameplayManager().ActionStateAnimationsHandler(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
					static_cast<UnitEntity*>(m_Cursor->GetSecondObject()), m_SkillIndex);
			
				Game::GetGame()->GetGameplayManager().ManaReductions(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), m_SkillIndex);
			}
		}
		break;
	case 'e':
	case 'E':
		if (!Game::GetGame()->GetGameplayManager().GetHasAnimPlayed())
		{
			if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetSkillRadius() > 0)
			{
				AOE();
				Game::GetGame()->GetGameplayManager().ManaReductions(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), m_SkillIndex);
			}
			else
			{
				UseSelectedSkillOnTarget(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()),
					static_cast<UnitEntity*>(m_Cursor->GetSecondObject()));

				Game::GetGame()->GetGameplayManager().ManaReductions(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), m_SkillIndex);
			}
			if (m_State != MODE_STATE::GAME_OVER)
				PostActionCleanUp();
		}
		break;
	}
}

void MainGameMode::ProcessKeyGameOverState(char key)
{
	switch (key)
	{
	case VK_ESCAPE:
		PostQuitMessage(0);
		break;
	case 'f':
	case 'F':
		StartFreshGame();
		break;
	}
}

void MainGameMode::PostActionCleanUp()
{
	//Set Gameplay flags back to false
	Game::GetGame()->GetGameplayManager().SetHasAnimPlayed(false);
	Game::GetGame()->GetGameplayManager().SetUnit1ResetBool(false);
	Game::GetGame()->GetGameplayManager().SetUnit2ResetBool(false);

	//Disable Targeting Grid
	m_TargetingSystem.DisableGrid();
	m_TargetingSystem.DisableAoEGrid();
	//Flag the unit as unable to act
	static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->SetActState(false);
	//If the unit has no remaining actions
	if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->CheckRemainingActions() == false)
	{
		static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->DisableUnit();

		//Post attack clean up to get the UI aligned with the free navigation state
		UpdateCursorFreeMode();
		m_State = MODE_STATE::FREE_NAVIGATION;
		CheckIfTeamActionsSpent();
	}
	else
	{
		//Post attack clean up to get the UI aligned with the free navigation state
		UpdateCursorFreeMode();
		//Change state
		m_State = MODE_STATE::FREE_NAVIGATION;
	}

}

bool MainGameMode::SnapCameraToUnit()
{
	switch (m_CurrentTeamID)
	{
	case 1:
		for (auto& a : m_TeamOne)
		{
			if (a->GetAliveState())
			{
				m_Cursor->MoveFixedCursorCoords(a->GetMapCoordinates());
				m_Camera.MoveCameraToCoordinates(a->GetMapCoordinates());
				//Updated the camera so return true
				return true;
			}
		}
		break;
	case 2:
		for (auto& a : m_TeamTwo)
		{
			if (a->GetAliveState())
			{
				m_Cursor->MoveFixedCursorCoords(a->GetMapCoordinates());
				m_Camera.MoveCameraToCoordinates(a->GetMapCoordinates());
				//Updated the camera so return true
				return true;
			}
		}
		break;
	}

	//No alive units found, so return a false
	return false;
}

void MainGameMode::UseSelectedSkillOnTarget(UnitEntity* unit1, UnitEntity* unit2)
{
	Game::GetGame()->GetGameplayManager().ProcessSkill(unit1,unit2, m_SkillIndex);

	//Run unit state check, and then team alive count check
	UnitStatusCheck(unit2);
}

void MainGameMode::UnitStatusCheck(UnitEntity* unit)
{
	//Is the unit dead
	if (unit->GetClassTotals().CurrentHP <= 0)
	{
		unit->GetClassTotals().CurrentHP = 0;
		unit->SetAliveState(false);
		unit->GetPrimarySprite().SetColour(DISABLED_UNIT_COLOUR);
		SetDeathAnim(unit);
		//Play Select Audio
		Game::GetGame()->GetAudioManager().PlayOneShot(
			Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX5);
		if (unit->GetUnitTeamID() == 1)
			--m_TeamOneAliveCount;
		else
			--m_TeamTwoAliveCount;

		//Check if team(s) have been wiped out
		CheckTeamAliveStates();
	}
}

void MainGameMode::CheckTeamAliveStates()
{
	//Check for draw
	if (m_TeamOneAliveCount <= 0 && m_TeamTwoAliveCount <= 0)
	{
		SetupGameOverMenu(0);
		m_State = MODE_STATE::GAME_OVER;
	}
	//Check team one win
	else if (m_TeamTwoAliveCount <= 0)
	{
		SetupGameOverMenu(1);
		m_State = MODE_STATE::GAME_OVER;
	}
	//Check team two win
	else if (m_TeamOneAliveCount <= 0)
	{
		SetupGameOverMenu(2);
		m_State = MODE_STATE::GAME_OVER;
	}
}

void MainGameMode::SetDeathAnim(UnitEntity* unit)
{
	switch (unit->GetUnitClassID())
	{
	case GameClasses::ASSASSIN:
		unit->GetPrimarySprite().GetAnimator().SetAnimation((int)AssassinAnimIndexes::DEFEAT_00, true, false, false);
		break;

	case GameClasses::BARBARIAN:
		unit->GetPrimarySprite().GetAnimator().SetAnimation((int)BarbarianAnimIndexes::DEFEAT_00, true, false, false);
		break;

	case GameClasses::ARCHER:
		unit->GetPrimarySprite().GetAnimator().SetAnimation((int)ArcherAnimIndexes::DEFEAT_00, true, false, false);
		break;

	case GameClasses::NECROMANCER:
		unit->GetPrimarySprite().GetAnimator().SetAnimation((int)NecromancerAnimIndexes::DEFEAT_00, true, false, false);
		break;

	case GameClasses::SORCERESS:
		unit->GetPrimarySprite().GetAnimator().SetAnimation((int)SorceressAnimIndexes::DEFEAT_00, true, false, false);
		break;

	case GameClasses::WARRIOR:
		unit->GetPrimarySprite().GetAnimator().SetAnimation((int)WarriorAnimIndexes::DEFEAT_00, true, false, false);
		break;

	}
}

void MainGameMode::AOE()
{
	m_TargetingSystem.DisableGrid();

	m_TargetingSystem.GenerateAoEGrid(m_TileMap, m_Cursor->GetMapCoordinates(),
		static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetSkillRadius());

	switch (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetUnitTeamID())
	{
	case 1:
		for (auto a : m_TeamTwo)
		{
			if (m_TargetingSystem.IsUnitInAoEGrid(a->GetMapCoordinates()))
			{
				UseSelectedSkillOnTarget(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), a);
			}
		}
		break;
	case 2:
		for (auto a : m_TeamOne)
		{
			if (m_TargetingSystem.IsUnitInAoEGrid(a->GetMapCoordinates()))
			{
				UseSelectedSkillOnTarget(static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()), a);
			}
		}
		break;
	}
}

void MainGameMode::KillUnit(UnitEntity* unit)
{
	unit->GetClassTotals().CurrentHP = 0;
}

void MainGameMode::UpdateSkillTooltip()
{
	UnitEntity* unit = static_cast<UnitEntity*>(m_Cursor->GetCurrentObject());
	std::stringstream ss;

	m_SkillTooltip->ResetDrawStrings();

	//Set name
	ss << unit->GetSkillAtIndex(m_SkillIndex)->GetActionName();
	m_SkillTooltip->ChangeStringByIndex(ss.str(), 0);
	std::stringstream().swap(ss);

	//Set MP
	ss << unit->GetSkillAtIndex(m_SkillIndex)->GetManaCost();
	m_SkillTooltip->AppendToStringByIndex(ss.str(), 1);
	std::stringstream().swap(ss);

	//Set Range
	ss << unit->GetSkillAtIndex(m_SkillIndex)->GetSkillRange();
	m_SkillTooltip->AppendToStringByIndex(ss.str(), 2);
	std::stringstream().swap(ss);

	//Set Radius
	ss << unit->GetSkillAtIndex(m_SkillIndex)->GetSkillRadius();
	m_SkillTooltip->AppendToStringByIndex(ss.str(), 3);
	std::stringstream().swap(ss);

	//Set Tooltip
	ss << unit->GetSkillAtIndex(m_SkillIndex)->GetTooltip();
	m_SkillTooltip->ChangeStringByIndex(ss.str(), 4);
	std::stringstream().swap(ss);

}

void MainGameMode::UpdateTeamFrame()
{
	m_TeamFrame->ResetDrawStrings();
	std::stringstream ss;
	ss << m_CurrentTeamID;
	m_TeamFrame->AppendToStringByIndex(ss, 0);
}

void MainGameMode::UpdateStatusFrame(NonNavigationUI* ui, UnitEntity* unit)
{
	std::stringstream ss;
	std::string s;

	ui->ResetDrawStrings();
	ui->AppendToStringByIndex(unit->GetUnitClass(), 0);
	ui->AppendToStringByIndex(unit->GetUnitClass(), 1);

	ss << unit->GetClassTotals().CurrentHP << "/" << unit->GetClassTotals().MaxHP;
	ui->AppendToStringByIndex(ss, 2);
	std::stringstream().swap(ss);

	ss << unit->GetClassTotals().CurrentMP << "/" << unit->GetClassTotals().MaxMP;
	ui->AppendToStringByIndex(ss, 3);
	std::stringstream().swap(ss);

	ss << unit->GetClassTotals().TotalMovespeed;
	ui->AppendToStringByIndex(ss, 4);

	s = unit->GetEquipmentAtIndex(0)->EquipmentName;
	ui->AppendToStringByIndex(s, 5);

	s = unit->GetEquipmentAtIndex(1)->EquipmentName;
	ui->AppendToStringByIndex(s, 6);

	s = unit->GetEquipmentAtIndex(2)->EquipmentName;
	ui->AppendToStringByIndex(s, 7);

	s = unit->GetEquipmentAtIndex(3)->EquipmentName;
	ui->AppendToStringByIndex(s, 8);

	s = unit->GetEquipmentAtIndex(4)->EquipmentName;
	ui->AppendToStringByIndex(s, 9);

	s = unit->GetSkillAtIndex(1)->GetActionName();
	ui->AppendToStringByIndex(s, 10);

	s = unit->GetSkillAtIndex(2)->GetActionName();
	ui->AppendToStringByIndex(s, 11);
	
	s = unit->GetSkillAtIndex(3)->GetActionName();
	ui->AppendToStringByIndex(s, 12);


}

void MainGameMode::SwitchUnitMenuToTargetingMode(UIElementManager*& manager, MODE_STATE state)
{
	//Generate range grid
	m_TargetingSystem.GenerateTargetGrid(
		m_TileMap,
		m_Cursor->GetCurrentObject()->GetMapCoordinates(),
		static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetSkillAtIndex(m_SkillIndex)->GetSkillRange()
	);

	//Pre-update the cursor for targeting mode
	UpdateCursorTargetingMode();

	//Change to appropriate state
	m_State = state;
		
}

void MainGameMode::RenderBackground(PassData& data, ID3D12GraphicsCommandList* cl)
{
	//Begin Draw
	data.sceneSB->Begin(cl);


	//Render BGs
	for (auto& a : m_Backgrounds)
		a->Render(*data.sceneSB, data.heap);


	//End Draw
	data.sceneSB->End();
}

void MainGameMode::RenderScene(PassData& data, ID3D12GraphicsCommandList* cl)
{
	//Begin Draw
	data.sceneSB->Begin(cl, SpriteSortMode_Deferred, m_Camera.GetSceneTransform());


	//Render Terrain
	for (auto& a : m_TileMap)
		a->Render(*data.sceneSB, data.heap);

	//Render Actors
	for (auto& a : m_Actors)
		a->Render(*data.sceneSB, data.heap);

	//Render Projectiles
	for (auto& a : m_Projectiles)
		a->Render(*data.sceneSB, data.heap);


	//End Draw
	data.sceneSB->End();

}

void MainGameMode::RenderUI(PassData& data, ID3D12GraphicsCommandList* cl)
{
	//Begin Draw
	data.uiSB->Begin(cl);


	//Draw cursor
	m_Cursor->Render(*data.uiSB, data.heap);

	//Menu Rendering
	switch (m_State)
	{
	case MODE_STATE::UNIT_PERFORM_ACTION_STATE:
		//Get Gameplay Manager Function Call Here
		Game::GetGame()->GetGameplayManager().GetBattleSceneAtIndex(0)->Render(*data.uiSB, data.heap);
		Game::GetGame()->GetGameplayManager().GetActionStateUnitAtIndex(0)->Render(*data.uiSB, data.heap);
		Game::GetGame()->GetGameplayManager().GetActionStateUnitAtIndex(1)->Render(*data.uiSB, data.heap);
		Game::GetGame()->GetGameplayManager().GetActionStateUnitAtIndex(2)->Render(*data.uiSB, data.heap);
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		break;
	case MODE_STATE::MOVE_MENU_OPEN:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		break;
	case MODE_STATE::ATTACK_MENU_OPEN:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		break;
	case MODE_STATE::UNIT_SELECTION:
		m_SelectionFrame->Render(*data.uiSB, data.heap);
		m_ArrowLeft->Render(*data.uiSB, data.heap);
		m_ArrowRight->Render(*data.uiSB, data.heap);
		//Render unit here as its here as a UI element
		m_UnitSelectionDisplayUnit->Render(*data.uiSB, data.heap);
		m_ContextControlCharSelect->Render(*data.uiSB, data.heap);
		break;

	case MODE_STATE::UNIT_PLACEMENT:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		break;

	case MODE_STATE::FREE_NAVIGATION:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		m_FriendlyUnitUI->Render(*data.uiSB, data.heap);
		m_EnemyUnitUI->Render(*data.uiSB, data.heap);
		m_TileTooltip->Render(*data.uiSB, data.heap);

		break;

	case MODE_STATE::UNIT_MENU_OPEN:
		m_UnitMenu->Render(*data.uiSB, data.heap);
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		m_TileTooltip->Render(*data.uiSB, data.heap);
		break;
	case MODE_STATE::NON_UNIT_MENU_OPEN:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		m_EndTurnMenu->Render(*data.uiSB, data.heap);
		m_TileTooltip->Render(*data.uiSB, data.heap);
		break;
	case MODE_STATE::STATUS_FRAME_OPEN:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		m_StatusFrame->Render(*data.uiSB, data.heap);
		break;
	case MODE_STATE::ACTION_MENU_OPEN:
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		m_ActionMenu->Render(*data.uiSB, data.heap);
		if (m_SkillTooltip)
		{
			m_SkillTooltip->Render(*data.uiSB, data.heap);
		}
		break;

	case MODE_STATE::FIND_SKILL_TARGET:
		m_TargetFrame->Render(*data.uiSB, data.heap);
		m_ContextControlGame->Render(*data.uiSB, data.heap);
		m_FriendlyUnitUI->Render(*data.uiSB, data.heap);
		m_EnemyUnitUI->Render(*data.uiSB, data.heap);

		break;

	case MODE_STATE::GAME_OVER:
		m_GameOverFrame->Render(*data.uiSB, data.heap);
		break;

	}

	m_TeamFrame->Render(*data.uiSB, data.heap);

	//End Draw
	data.uiSB->End();
}

void MainGameMode::UpdateCursorFreeMode()
{
	//First, find the underlying tile
	if (m_Cursor->SearchForTileObject(m_TileMap, m_MapLimit.x))
		UpdateTileTooltip(m_TileTooltip, m_Cursor->GetCurrentTileObject());

	//Next, check to see if there is a unit occupying the tile, and of what type it is
	if (m_Cursor->SearchForUnitObject(m_Actors, m_MapLimit.x))
	{

		if (static_cast<UnitEntity*>(m_Cursor->GetCurrentObject())->GetUnitTeamID() == m_CurrentTeamID)
		{
			//Enable frame and update
			m_FriendlyUnitUI->SetEnabledFlag(true);
			UpdateUnitTooltip(m_FriendlyUnitUI, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()));

			//Disable enemy frame
			m_EnemyUnitUI->SetEnabledFlag(false);
		}
		else
		{
			//Enable frame and update
			m_EnemyUnitUI->SetEnabledFlag(true);
			UpdateUnitTooltip(m_EnemyUnitUI, static_cast<UnitEntity*>(m_Cursor->GetCurrentObject()));

			//Disable enemy frame
			m_FriendlyUnitUI->SetEnabledFlag(false);
		}

	}
	//No matches, so make sure both frames are off
	else
	{
		m_FriendlyUnitUI->SetEnabledFlag(false);
		m_EnemyUnitUI->SetEnabledFlag(false);
	}


}

void MainGameMode::UpdateCursorTargetingMode()
{
	//Updates the cursor, looking only for a second unit
	if (m_Cursor->SearchForSecondUnitObject(m_Actors, m_MapLimit.x))
	{

		//Friendly unit
		if (static_cast<UnitEntity*>(m_Cursor->GetSecondObject())->GetUnitTeamID() == m_CurrentTeamID)
		{

			//Enable frame and update
			m_FriendlyUnitUI->SetEnabledFlag(true);
			UpdateUnitTooltip(m_FriendlyUnitUI, static_cast<UnitEntity*>(m_Cursor->GetSecondObject()));

			//Disable enemy frame
			m_EnemyUnitUI->SetEnabledFlag(false);
		}
		else
		{
			//Enable frame and update
			m_EnemyUnitUI->SetEnabledFlag(true);
			UpdateUnitTooltip(m_EnemyUnitUI, static_cast<UnitEntity*>(m_Cursor->GetSecondObject()));

			//Disable enemy frame
			m_FriendlyUnitUI->SetEnabledFlag(false);
		}
	}
	//No match unit found so might of moved off a unit, so clear the UI
	else
	{
		m_FriendlyUnitUI->SetEnabledFlag(false);
		m_EnemyUnitUI->SetEnabledFlag(false);
	}
}

void MainGameMode::UpdateTileTooltip(NonNavigationUI* ui, MapTile* tile)
{
	//Reset existing strings
	ui->ResetDrawStrings();
	ui->ChangeStringByIndex(tile->GetTileProperties().tileName, 0);
	std::stringstream ss;
	ss << tile->GetTileProperties().moveCost;
	ui->AppendToStringByIndex(ss, 1);
}

void MainGameMode::UpdateUnitTooltip(NonNavigationUI* ui, UnitEntity* unit)
{
	//Reset existing strings
	ui->ResetDrawStrings();
	ui->AppendToStringByIndex(unit->GetUnitClass(), 0);
	std::stringstream ss;
	ss << unit->GetClassTotals().CurrentHP << "/" << unit->GetClassTotals().MaxHP;
	ui->AppendToStringByIndex(ss, 1);
	std::stringstream().swap(ss);
	ss << unit->GetClassTotals().CurrentMP << "/" << unit->GetClassTotals().MaxMP;
	ui->AppendToStringByIndex(ss, 2);
}

void MainGameMode::ChangeTeamID()
{
	if (m_CurrentTeamID == 1)
		m_CurrentTeamID = 2;
	else
		m_CurrentTeamID = 1;

	UpdateTeamFrame();
}


