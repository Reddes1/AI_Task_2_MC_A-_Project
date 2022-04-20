#pragma once

#include "EntityInterface.h"
#include "GameUtilities.h"
#include "SkillInterface.h"

struct ClassTotals {
	float MaxHP = 0.0f;
	float CurrentHP = 0.0f;
	int MaxMP = 0;
	int CurrentMP = 0;
	float TotalMovespeed = 0.0f;
	float TotalPhysAttack = 0.0f;
	float TotalMagAttack = 0.0f;
	float TotalPhysHit = 0.0f;
	float TotalMagHit = 0.0f;
	float TotalCritHit = 0.0f;
	float TotalCritMultiplier = 0.0f;
	float TotalPhysArmour = 0.0f;
	float TotalMagArmour = 0.0f;
	float TotalEvasion = 0.0f;
	float TotalMagResist = 0.0f;
};

struct BuffPercentages {
	float PhysDamageBuff = 1.0f;
	float MagDamageBuff = 1.0f;
	float PhysHitBuff = 1.0f;
	float MagHitBuff = 1.0f;
	float PhysArmourBuff = 1.0f;
	float MagArmourBuff = 1.0f;
	float MagResistBuff = 1.0f;
	float CritHitBuff = 1.0f;
	float CritDamageBuff = 1.0;
	float EvasionBuff = 1.0f;
	float MovespeedBuff = 1.0f;
	int TurnCount = 0;
};

struct UnitStateFlags
{
	UnitStateFlags()
		:isAlive(true), canMove(true), canAct(true), buffActive(false)
	{}

	bool isAlive : 1;
	bool canMove : 1;
	bool canAct : 1;
	bool buffActive : 1;
};

/*
	Specialised class for representing a game unit. Setup for specific navigation and interact with the world space.
*/
class UnitEntity : public EntityInterface
{
public:

	enum class UNIT_TYPE : int
	{
		LAND,
		SEA,
		AIR
	};


	UnitEntity() { InitUnit(); }
	UnitEntity(std::shared_ptr<SpriteTexture> texture);
	~UnitEntity() {}

	/////////////////
	/// Overrides ///
	/////////////////

	void Update(const GameTimer& gt);
	void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap)
	{
		if (m_EntityData.isActive)
			EntityInterface::Render(batch, heap);
	}


	///////////
	/// Set ///
	///////////

	void SetUnitType(UNIT_TYPE type) { m_Type = type; }
	void SetUnitTeamID(int id) { m_UnitTeamID = id; }

	void SetUnitClass(int id);
	void SetUnitBaseEquipment(int MainHand, int OffHand, int Armour, int Trinket1, int Trinket2);
	void SetBaseSkills(int Basic, int Skill1, int Skill2, int Skill3);
	void SetSkillsBasedOnClass();
	void SetTileSize(DirectX::XMINT2 dist) { m_TileSize = dist; }
	void SetTileSize(int32_t x, int32_t y) { m_TileSize.x = x; m_TileSize.y = y; }

	//Updates the unit can act state
	void SetActState(bool newState) { m_StateFlags.canAct = newState; }
	//Update the unit can move state
	void SetMoveState(bool newState) { m_StateFlags.canMove = newState; }
	//Update alive state
	void SetAliveState(bool newState) { m_StateFlags.isAlive = newState; }
	//Update buff state
	void SetBuffState(bool newState) { m_StateFlags.buffActive = newState; }




	///////////
	/// Get ///
	///////////

	UNIT_TYPE GetUnitType() { return m_Type; }
	int GetUnitTeamID() { return m_UnitTeamID; }
	int GetUnitClassID() { return m_UnitClass->UniqueID; }
	std::string GetUnitClass() { return m_UnitClass->ClassName; }

	ClassTotals& GetClassTotals() { return m_ClassTotals; }
	BuffPercentages& GetBuffs() { return m_CurrentBuffs; }
	SkillInterface* GetSkillAtIndex(int index) { return m_Skills.at(index).get(); }
	const Equipment* GetEquipmentAtIndex(int index) { return m_UnitEquipment.at(index).get(); }
	const UnitStateFlags& GetUnitStateFlags() { return m_StateFlags; }
	bool GetAliveState() { return m_StateFlags.isAlive; }
	bool GetActState() { return m_StateFlags.canAct; }
	bool GetMoveState() { return m_StateFlags.canMove; }
	bool GetBuffState() { return m_StateFlags.buffActive; }


	//////////////////
	/// Operations ///
	//////////////////

	//Up: 0, Down: 1, Left: 2, Right: 3
	void Move(int dir);
	//Force move the unit to world coordinates
	void MoveToCoordinate(DirectX::XMINT2& newCoords);

	//Checks if the unit flags to see if it has any remaining actions, return true if at least one is available.
	bool CheckRemainingActions();

	//Disable the unit for the turn
	void DisableUnit();
	//Enable the unit for play
	void EnableUnit();

	////////////////////////////////
	/// Combat & Stat Operations ///
	////////////////////////////////

	// Called in the unit Init, sets the starting total values
	void InitTotalValues();
	// This will be called each frame in case any
	//values have been changed and the total values need updated
	void UpdateTotalValues();

	//Calculate the Units class totals - these will be used in calculations
	void MaxHealth();
	void MaxMP();
	void InitCurrentHealth();
	void InitCurrentMP();
	void TotalPhysValues();
	void TotalMagValues();
	void OtherTotalValues();

	void ResetBuffValues();

private:

	void InitUnit();

	//Hold tile size of locally for fixed movement
	DirectX::XMINT2 m_TileSize = { 0, 0 };

	//Unit Team ID
	int m_UnitTeamID = -1;
	//Define the units type
	UNIT_TYPE m_Type = UNIT_TYPE::LAND;

	//Hold the class data
	std::shared_ptr<CharacterClass> m_UnitClass;
	//Hold the Classes Equipment Data
	std::vector<std::shared_ptr<Equipment>> m_UnitEquipment;
	//Sum number of base class, equipment etc
	ClassTotals m_ClassTotals;
	//Holds the values of the current buffs
	BuffPercentages m_CurrentBuffs;

	//Holds Units Skills
	/*
	* index [0] = Basic attack always
	* indexes [1-3] = Any other skill
	*/
	std::vector<std::shared_ptr<SkillInterface>> m_Skills;
	//Flags for state control
	UnitStateFlags m_StateFlags;
};
