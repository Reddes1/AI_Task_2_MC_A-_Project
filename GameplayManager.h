#pragma once
#include "UnitEntity.h"
#include "MapTile.h"

class GameplayManager
{
public:
	enum SkillType
	{
		DAMAGE,
		HEAL,
		BUFF
	};

	enum DamageType
	{
		PHYSICAL,
		MAGICAL
	};

	enum SkillIndex
	{
		BASIC,
		SKILL1,
		SKILL2,
		SKILL3
	};

	GameplayManager();
	~GameplayManager();

	//Initialises the 2 Action display units
	void Init(int w, int h);
	
	void Update(const GameTimer& gt, UnitEntity* unit1, UnitEntity* unit2,int index);

	//Decides which calculations to run based on skill type and if necessary damage type
	/*
	* Note -
	* Unit 1 and 2 can be the same unit
	* i.e. unit 1 wants to heal themselves, when selecting heal target they select
	* themselves which makes them both unit 1 and 2
	*/
	void ProcessSkill(UnitEntity* unit1, UnitEntity* unit2, int index);

	//Calculate the mana reductions and do a check to see if remaining mana
	//is less than 0 is it is set it = 0
	int ManaReductions(UnitEntity* unit1, int index);

	//Called when we are about to enter the PerformUnitActionState
	void BeforeStateEnter(UnitEntity* unit1, UnitEntity* unit2, std::vector<MapTile*>& tiles, int maxMapX, int index);
	void OnStateExit();

	//Function for handling when to play certain animations
	void ActionStateAnimationsHandler(UnitEntity* unit1, UnitEntity* unit2, int index);
	//Animate projectiles
	void MoveProjectiles(const GameTimer& gt);
	//Getters
	UnitEntity*& GetActionStateUnitAtIndex(int index) { return m_AttackStateUnits[index]; }
	EntityInterface*& GetBattleSceneAtIndex(int index) { return m_BattleScenes[index]; }
	bool& GetHasAnimPlayed() { return HasAnimPlayed; }
	
	//Setters
	void SetHasAnimPlayed(bool Played) { HasAnimPlayed = Played; }
	void SetUnit1ResetBool(bool reset) { Unit1Reset = reset; }
	void SetUnit2ResetBool(bool reset) { Unit2Reset = reset; }
private:
	//Hit chance generator (rand num between 0-100)
	int HitPercentage();
	int CritHitPercentage();

	//Calculate the Hit chances for Physical, Magical and Crit
	float PhysHit(UnitEntity* unit1, UnitEntity* unit2);
	float MagHit(UnitEntity* unit1, UnitEntity* unit2);
	float CritHit(UnitEntity* unit1, int index);

	//Sets up each of the Action display units
	void SetupAttackStateUnit1(UnitEntity* unit1);
	void SetupAttackStateUnit2(UnitEntity* unit2);
	void SetupProjectileUnit(UnitEntity* unit1, int index);

	//Plays an attack animation based on the calss and the range of the skill
	void PlayAttack(UnitEntity* unit1, int index);

	//Plays a Hurt animaition, called when the unit is hit
	void PlayHit(UnitEntity* unit2);

	//Plays a "dodge" animation, called when the unit1 skill misses
	void PlayDodge(UnitEntity* unit2);

	//Plays a "Heal" animations
	void PlayHeal(UnitEntity* unit1);
	void RecieveHeal(UnitEntity* unit2);

	//Plays a "Buff" animation
	void PlayBuff(UnitEntity* unit1);
	void RecieveBuff(UnitEntity* unit2);

	//Reset the units to their idle animation
	void ResetUnit1ToIdle(UnitEntity* unit1);
	void ResetUnit2ToIdle(UnitEntity* unit2);

	//Set up Battle Scene
	void SetupBattleScene(UnitEntity* unit1, std::vector<MapTile*>& tiles, int maxMapX);

	////////////////
	/// Utilties ///
	////////////////

	MapTile*& GetTileInArrayByCoordinates(std::vector<MapTile*>& tiles, DirectX::XMINT2& coords, int maxMapX);

	//Flags used for simple checks
	bool Unit1Reset = false;
	bool Unit2Reset = false;
	bool HasAnimPlayed = false;
	bool DamageHit = false;
	bool DamageDodge = false;
	bool AttackPlayed = false;
	bool ProjectileHit = false;
	//Vector storing 2 UnitEntities that are used just for playing animations
	std::vector<UnitEntity*> m_AttackStateUnits;

	//Vector holding the battle scenes
	std::vector<EntityInterface*> m_BattleScenes;


	//Testing


};
