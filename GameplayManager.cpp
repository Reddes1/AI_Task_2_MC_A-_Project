#include "GameplayManager.h"
#include "GameConstants.h"
#include "Game.h"
#include "TestEntity.h"

GameplayManager::GameplayManager()
{
	m_AttackStateUnits.reserve(3);
}

GameplayManager::~GameplayManager()
{
	for (auto& a : m_AttackStateUnits)
	{
		delete a;
		a = nullptr;
	}
}

void GameplayManager::Init(int w, int h)
{
	Game* game = Game::GetGame();
	UnitEntity* unit1 = new UnitEntity();
	UnitEntity* unit2 = new UnitEntity();
	UnitEntity* projectile = new UnitEntity();

	//Active Unit
	unit1->GetPrimarySprite().SetPosition(w * 0.33, h * 0.77);
	unit1->GetPrimarySprite().SetScale(1.5, 1.5);
	unit1->GetPrimarySprite().SetEffect(DirectX::SpriteEffects::SpriteEffects_None);
	//Target Unit
	unit2->GetPrimarySprite().SetPosition(w * 0.66, h * 0.77);
	unit2->GetPrimarySprite().SetScale(1.5, 1.5);
	unit2->GetPrimarySprite().SetEffect(DirectX::SpriteEffects::SpriteEffects_FlipHorizontally);
	//Projectile Unit
	projectile->GetPrimarySprite().SetPosition(w * 0.44, h * 0.77);
	projectile->GetPrimarySprite().SetScale(1.5, 1.5);
	projectile->GetPrimarySprite().SetEffect(DirectX::SpriteEffects::SpriteEffects_None);

	m_AttackStateUnits.push_back(unit1);
	m_AttackStateUnits.push_back(unit2);
	m_AttackStateUnits.push_back(projectile);

	TestEntity* bg = new TestEntity(game->GetAssetManager().GetSpriteTextureData(std::string("Battle_Scene_Bridge")));
	EntityInterface::Add(m_BattleScenes, bg);
}

void GameplayManager::Update(const GameTimer& gt, UnitEntity* unit1, UnitEntity* unit2,int index)
{
	MoveProjectiles(gt);

	if (AttackPlayed && DamageHit)
	{
		if (unit1->GetSkillAtIndex(index)->GetSkillRange() > 1 && ProjectileHit)
		{
			PlayHit(unit2);
		}
		else if(unit1->GetSkillAtIndex(index)->GetSkillRange() <= 1)
		{
			PlayHit(unit2);
		}
	}

	if (AttackPlayed && m_AttackStateUnits[2]->GetPrimarySprite().GetPosition().x > m_AttackStateUnits[1]->GetPrimarySprite().GetPosition().x - (ProjectileOffsetX * 10) && DamageDodge)
	{
		PlayDodge(unit2);
	}

	if (m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().GetAnimatorData().play == false && Unit1Reset == false)
		ResetUnit1ToIdle(unit1);

	if (m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().GetAnimatorData().play == false && Unit2Reset == false)
		ResetUnit2ToIdle(unit2);
}

void GameplayManager::ProcessSkill(UnitEntity* unit1, UnitEntity* unit2, int index)
{
	switch (unit1->GetSkillAtIndex(index)->GetSkillType())
	{
	case SkillType::DAMAGE:
		switch (unit1->GetSkillAtIndex(index)->GetDamageType())
		{
		case DamageType::PHYSICAL:

			if (HitPercentage() <= PhysHit(unit1, unit2))
			{
				DamageHit = true;
				if (CritHitPercentage() <= CritHit(unit1,index))
				{
					unit1->GetSkillAtIndex(index)->DoCrit(unit1, unit2);
				}
				else
				{
					unit1->GetSkillAtIndex(index)->DoAction(unit1, unit2);
				}
			}
			else
			{
				DamageDodge = true;
			}
			break;
		case DamageType::MAGICAL:

			if (HitPercentage() <= MagHit(unit1, unit2))
			{
				DamageHit = true;
				if (CritHitPercentage() <= CritHit(unit1, index))
				{
					unit1->GetSkillAtIndex(index)->DoCrit(unit1, unit2);
				}
				else
				{
					unit1->GetSkillAtIndex(index)->DoAction(unit1, unit2);
				}
			}
			else
			{
				DamageDodge = true;
			}
			break;
		}
		break;
	case SkillType::HEAL:
		unit1->GetSkillAtIndex(index)->DoAction(unit1, unit2);
		break;
	case SkillType::BUFF:
		unit1->GetSkillAtIndex(index)->DoAction(unit1, unit2);
		break;
	}
}

int GameplayManager::HitPercentage()
{
	srand((unsigned)time(0));

	int hitchance = rand() % 100 + 1;
	return hitchance;
}

int GameplayManager::CritHitPercentage()
{
	srand((unsigned)time(0));

	int critChance = rand() % 100 + 1;
	return critChance;
}

float GameplayManager::PhysHit(UnitEntity* unit1, UnitEntity* unit2)
{
	float PhysHit = unit1->GetClassTotals().TotalPhysHit -
		unit2->GetClassTotals().TotalEvasion;
	return PhysHit;
}

float GameplayManager::MagHit(UnitEntity* unit1, UnitEntity* unit2)
{
	float MagHit = unit1->GetClassTotals().TotalMagHit -
		unit2->GetClassTotals().TotalMagResist;
	return MagHit;
}

float GameplayManager::CritHit(UnitEntity* unit1, int index)
{
	float CritHit = unit1->GetClassTotals().TotalCritHit +
		unit1->GetSkillAtIndex(index)->GetSkillCrit();
	return CritHit;
}

int GameplayManager::ManaReductions(UnitEntity* unit1,int index)
{
	unit1->GetClassTotals().CurrentMP -= unit1->GetSkillAtIndex(index)->GetManaCost();
	if (unit1->GetClassTotals().CurrentMP < 0)
	{
		unit1->GetClassTotals().CurrentMP = 0;
	}
	return 0;
}

void GameplayManager::BeforeStateEnter(UnitEntity* unit1, UnitEntity* unit2, std::vector<MapTile*>& tiles, int maxMapX, int index)
{
	SetupAttackStateUnit1(unit1);
	SetupAttackStateUnit2(unit2);
	SetupBattleScene(unit1, tiles, maxMapX);
	SetupProjectileUnit(unit1, index);
}

void GameplayManager::OnStateExit()
{
}

void GameplayManager::SetupAttackStateUnit1(UnitEntity* unit1)
{
	Game* game = Game::GetGame();

	switch (unit1->GetUnitClassID())
	{
	case GameClasses::ASSASSIN:
		m_AttackStateUnits[0]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00"))
		);
		m_AttackStateUnits[0]->SetUnitClass(GameClasses::ASSASSIN);
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[0]->GetPrimarySprite().SetOriginToCenter();
		break;

	case GameClasses::BARBARIAN:
		m_AttackStateUnits[0]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("BARBARIAN_CLASS_SS_00"))
		);
		m_AttackStateUnits[0]->SetUnitClass(GameClasses::BARBARIAN);
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[0]->GetPrimarySprite().SetOriginToCenter();

		break;

	case GameClasses::ARCHER:
		m_AttackStateUnits[0]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ARCHER_CLASS_SS_00"))
		);
		m_AttackStateUnits[0]->SetUnitClass(GameClasses::ARCHER);
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[0]->GetPrimarySprite().SetOriginToCenter();
		break;
	case GameClasses::NECROMANCER:
		m_AttackStateUnits[0]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("NECROMANCER_CLASS_SS_00"))
		);
		m_AttackStateUnits[0]->SetUnitClass(GameClasses::NECROMANCER);
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[0]->GetPrimarySprite().SetOriginToCenter();

		break;

	case GameClasses::SORCERESS:
		m_AttackStateUnits[0]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
		);
		m_AttackStateUnits[0]->SetUnitClass(GameClasses::SORCERESS);
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[0]->GetPrimarySprite().SetOriginToCenter();

		break;

	case GameClasses::WARRIOR:
		m_AttackStateUnits[0]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("WARRIOR_CLASS_SS_00"))
		);
		m_AttackStateUnits[0]->SetUnitClass(GameClasses::WARRIOR);
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[0]->GetPrimarySprite().SetOriginToCenter();

		break;
	}
}

void GameplayManager::SetupAttackStateUnit2(UnitEntity* unit2)
{
	Game* game = Game::GetGame();

	switch (unit2->GetUnitClassID())
	{
	case GameClasses::ASSASSIN:
		m_AttackStateUnits[1]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00"))
		);
		m_AttackStateUnits[1]->SetUnitClass(GameClasses::ASSASSIN);
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[1]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[1]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
		break;

	case GameClasses::BARBARIAN:
		m_AttackStateUnits[1]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("BARBARIAN_CLASS_SS_00"))
		);
		m_AttackStateUnits[1]->SetUnitClass(GameClasses::BARBARIAN);
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[1]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[1]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());

		break;

	case GameClasses::ARCHER:
		m_AttackStateUnits[1]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ARCHER_CLASS_SS_00"))
		);
		m_AttackStateUnits[1]->SetUnitClass(GameClasses::ARCHER);
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[1]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[1]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());

		break;
	case GameClasses::NECROMANCER:
		m_AttackStateUnits[1]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("NECROMANCER_CLASS_SS_00"))
		);
		m_AttackStateUnits[1]->SetUnitClass(GameClasses::NECROMANCER);
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[1]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[1]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());

		break;

	case GameClasses::SORCERESS:
		m_AttackStateUnits[1]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
		);
		m_AttackStateUnits[1]->SetUnitClass(GameClasses::SORCERESS);
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[1]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[1]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());

		break;

	case GameClasses::WARRIOR:
		m_AttackStateUnits[1]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("WARRIOR_CLASS_SS_00"))
		);
		m_AttackStateUnits[1]->SetUnitClass(GameClasses::WARRIOR);
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::IDLE_00), true, true, false);
		m_AttackStateUnits[1]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[1]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());

		break;
	}
}

void GameplayManager::SetupProjectileUnit(UnitEntity* unit1, int index)
{
	Game* game = Game::GetGame();

	switch (unit1->GetUnitClassID())
	{
	case GameClasses::ASSASSIN:

		m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("ASSASSIN_CLASS_SS_00"))
		);
		m_AttackStateUnits[2]->SetUnitClass(GameClasses::ASSASSIN);
		m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::SHOCKWAVE_00), false, true, false);
		m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
		m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
		m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX,m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);
		break;
	case GameClasses::BARBARIAN:
		m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("BARBARIAN_CLASS_SS_00"))
		);
		m_AttackStateUnits[2]->SetUnitClass(GameClasses::BARBARIAN);
		m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::ATTACK_00), false, true, false);
		m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
		m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
		m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);
		break;
	case GameClasses::ARCHER:

		switch (index)
		{
		case SkillIndex::BASIC:
			m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("ARCHER_CLASS_SS_00"))
			);
			m_AttackStateUnits[2]->SetUnitClass(GameClasses::ARCHER);
			m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::ARROW_00), false, true, false);
			m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
			m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
			m_AttackStateUnits[2]->GetPrimarySprite().SetScale(1.0, 1.0);
			m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

			break;
		case SkillIndex::SKILL1:
			m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("ARCHER_CLASS_SS_00"))
			);
			m_AttackStateUnits[2]->SetUnitClass(GameClasses::ARCHER);
			m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::ARROW_00), false, true, false);
			m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
			m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
			m_AttackStateUnits[2]->GetPrimarySprite().SetScale(1.5, 1.5);
			m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

			break;
		}

		break;
	case GameClasses::NECROMANCER:

		m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("NECROMANCER_CLASS_SS_00"))
		);
		m_AttackStateUnits[2]->SetUnitClass(GameClasses::NECROMANCER);
		m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::SHOT_00), false, true, false);
		m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
		m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
		m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

		break;

	case GameClasses::SORCERESS:

		switch (index)
		{
		case SkillIndex::BASIC:
			m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
			);
			m_AttackStateUnits[2]->SetUnitClass(GameClasses::SORCERESS);
			m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::SHOT_00), false, true, false);
			m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
			m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
			m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
			m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

			break;
		case SkillIndex::SKILL1:
			m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
			);
			m_AttackStateUnits[2]->SetUnitClass(GameClasses::SORCERESS);
			m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::SHOT_00), false, true, false);
			m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
			m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
			m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
			m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

			break;
		case SkillIndex::SKILL2:
			m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
				game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
			);
			m_AttackStateUnits[2]->SetUnitClass(GameClasses::SORCERESS);
			m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::SPECIAL_ATTACK_01), false, true, false);
			m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
			m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
			m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
			m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

			break;
		}
		break;

	case GameClasses::WARRIOR:

		m_AttackStateUnits[2]->GetPrimarySprite().SetTexturePtr(
			game->GetAssetManager().GetSpriteTextureData(std::string("SORCERESS_CLASS_SS_00"))
		);
		m_AttackStateUnits[2]->SetUnitClass(GameClasses::WARRIOR);
		m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::SPECIAL_ATTACK_01), false, true, false);
		m_AttackStateUnits[2]->GetPrimarySprite().SetOriginToCenter();
		m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
		m_AttackStateUnits[2]->GetPrimarySprite().SetScale(2.0, 2.0);
		m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);

		break;
	}
}

void GameplayManager::ActionStateAnimationsHandler(UnitEntity* unit1, UnitEntity* unit2, int index)
{
	switch (unit1->GetSkillAtIndex(index)->GetSkillType())
	{
	case SkillType::DAMAGE:
		PlayAttack(unit1,index);
		AttackPlayed = true;
		break;
	case SkillType::HEAL:
		PlayHeal(unit1);
		RecieveHeal(unit2);
		break;
	case SkillType::BUFF:
		PlayBuff(unit1);
		RecieveBuff(unit2);
		break;
	}

	HasAnimPlayed = true;
}

void GameplayManager::MoveProjectiles(const GameTimer& gt)
{
	float speed = 500 * gt.DeltaTime();
	DirectX::XMFLOAT2 pos = m_AttackStateUnits[2]->GetPrimarySprite().GetPosition();

	if(m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().GetAnimatorData().play == true)
		pos.x += speed;

	if (m_AttackStateUnits[2]->GetPrimarySprite().GetPosition().x < m_AttackStateUnits[1]->GetPrimarySprite().GetPosition().x)
	{
		m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(pos);
	}
	else
	{
		ProjectileHit = true;
		m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(false);
		m_AttackStateUnits[2]->GetPrimarySprite().SetPosition(m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().x + ProjectileOffsetX, m_AttackStateUnits[0]->GetPrimarySprite().GetPosition().y + ProjectileOffsetY);
		m_AttackStateUnits[2]->GetPrimarySprite().SetColour(PROJECTILE_DEFAULT_COLOUR);
	}
}

void GameplayManager::PlayAttack(UnitEntity* unit1, int index)
{
		switch (unit1->GetUnitClassID())
		{
		case GameClasses::ASSASSIN:
			switch (index)
			{
			case SkillIndex::BASIC:
					m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::ATTACK_00), true, false, false);
					Game::GetGame()->GetAudioManager().PlayOneShot(
						Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX7);
				break;
			case SkillIndex::SKILL1:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::SPECIAL_ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			}
			break;
		case GameClasses::BARBARIAN:
			switch (index)
			{
			case SkillIndex::BASIC:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::ATTACK_00), true, false, false);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX7);
				break;
			case SkillIndex::SKILL1:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::SPECIAL_ATTACK_00), true, false, false);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX7);
				break;
			}
			break;
		case GameClasses::ARCHER:
			switch (index)
			{
			case SkillIndex::BASIC:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::ATTACK_01), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			case SkillIndex::SKILL1:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::SPECIAL_ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			}
			break;
		case GameClasses::NECROMANCER:
			switch (index)
			{
			case SkillIndex::BASIC:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			case SkillIndex::SKILL1:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::SPECIAL_ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(NECROTIC_BLAST_COLOUR);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			case SkillIndex::SKILL2:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::ATTACK_00), true, false, false);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX7);
				break;
			}
			break;
		case GameClasses::SORCERESS:
			switch (index)
			{
			case SkillIndex::BASIC:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			case SkillIndex::SKILL1:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::SPECIAL_ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(FIREBALL_COLOUR);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			case SkillIndex::SKILL2:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::SPECIAL_ATTACK_00), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(m_AttackStateUnits[0]->GetPrimarySprite().GetColour());
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			}
			break;
		case GameClasses::WARRIOR:
			switch (index)
			{
			case SkillIndex::BASIC:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::ATTACK_00), true, false, false);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX7);
				break;
			case SkillIndex::SKILL3:
				m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::ATTACK_01), true, false, false);
				m_AttackStateUnits[2]->GetPrimarySprite().GetAnimator().Play(true);
				m_AttackStateUnits[2]->GetPrimarySprite().SetColour(SMITE_COLOUR);
				Game::GetGame()->GetAudioManager().PlayOneShot(
					Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX8);
				break;
			}
			break;
		}
}

void GameplayManager::PlayHit(UnitEntity* unit2)
{
	Game::GetGame()->GetAudioManager().PlayOneShot(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX4);
		switch (unit2->GetUnitClassID())
		{
		case GameClasses::ASSASSIN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::HURT_00), true, false, false);
			break;

		case GameClasses::BARBARIAN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::HURT_00), true, false, false);
			break;

		case GameClasses::ARCHER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::HURT_00), true, false, false);
			break;
		case GameClasses::NECROMANCER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::HURT_00), true, false, false);
			break;

		case GameClasses::SORCERESS:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::HURT_00), true, false, false);

			break;

		case GameClasses::WARRIOR:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::HURT_00), true, false, false);
			break;
		}
		DamageHit = false;
		AttackPlayed = false;
		ProjectileHit = false;
}

void GameplayManager::PlayDodge(UnitEntity* unit2)
{
		switch (unit2->GetUnitClassID())
		{
		case GameClasses::ASSASSIN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::JUMP_00), true, false, false);
			break;

		case GameClasses::BARBARIAN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::JUMP_00), true, false, false);
			break;

		case GameClasses::ARCHER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::JUMP_00), true, false, false);
			break;
		case GameClasses::NECROMANCER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::JUMP_00), true, false, false);
			break;

		case GameClasses::SORCERESS:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::JUMP_00), true, false, false);
			break;

		case GameClasses::WARRIOR:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::JUMP_00), true, false, false);
			break;
		}
		DamageDodge = false;
		AttackPlayed = false;
		ProjectileHit = false;
}

void GameplayManager::PlayHeal(UnitEntity* unit1)
{
	switch (unit1->GetUnitClassID())
	{
	case GameClasses::ARCHER:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::PUSH_00), true, false, false);
		break;
	case GameClasses::WARRIOR:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::PUSH_00), true, false, false);
		break;
	}
}

void GameplayManager::RecieveHeal(UnitEntity* unit2)
{
	Game::GetGame()->GetAudioManager().PlayOneShot(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
		switch (unit2->GetUnitClassID())
		{
		case GameClasses::ASSASSIN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::PULL_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(HEAL_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
			break;

		case GameClasses::BARBARIAN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::PULL_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(HEAL_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
			break;

		case GameClasses::ARCHER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::PULL_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(HEAL_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
			break;
		case GameClasses::NECROMANCER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::PULL_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(HEAL_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
			break;

		case GameClasses::SORCERESS:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::PULL_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(HEAL_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
			break;

		case GameClasses::WARRIOR:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::PULL_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(HEAL_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX6);
			break;
		}
}

void GameplayManager::PlayBuff(UnitEntity* unit1)
{
		switch (unit1->GetUnitClassID())
		{
		case GameClasses::ASSASSIN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::PULL_00), true, false, false);
			break;

		case GameClasses::BARBARIAN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::PULL_00), true, false, false);
			break;

		case GameClasses::ARCHER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::PULL_00), true, false, false);
			break;
		case GameClasses::NECROMANCER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::PULL_00), true, false, false);
			break;

		case GameClasses::SORCERESS:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::PULL_00), true, false, false);
			break;
		}
}

void GameplayManager::RecieveBuff(UnitEntity* unit2)
{
	Game::GetGame()->GetAudioManager().PlayOneShot(
		Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
		switch (unit2->GetUnitClassID())
		{
		case GameClasses::ASSASSIN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(AssassinAnimIndexes::JUMP_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(BUFF_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
			break;

		case GameClasses::BARBARIAN:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(BarbarianAnimIndexes::JUMP_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(BUFF_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
			break;

		case GameClasses::ARCHER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(ArcherAnimIndexes::JUMP_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(BUFF_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
			break;
		case GameClasses::NECROMANCER:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(NecromancerAnimIndexes::JUMP_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(BUFF_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
			break;

		case GameClasses::SORCERESS:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(SorceressAnimIndexes::JUMP_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(BUFF_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
			break;

		case GameClasses::WARRIOR:
			m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)(WarriorAnimIndexes::JUMP_00), true, false, false);
			m_AttackStateUnits[1]->GetPrimarySprite().SetColour(BUFF_UNIT_COLOUR);
			Game::GetGame()->GetAudioManager().PlayOneShot(
				Game::GetGame()->GetAudioManager().GetSFXManager().m_SFX3);
			break;
		}
}

void GameplayManager::SetupBattleScene(UnitEntity* unit1, std::vector<MapTile*>& tiles, int maxMapX)
{
	Game* game = Game::GetGame();

	if (GetTileInArrayByCoordinates(tiles, unit1->GetMapCoordinates(), maxMapX)->GetTileProperties().tileName == "Grass" | 
		GetTileInArrayByCoordinates(tiles, unit1->GetMapCoordinates(), maxMapX)->GetTileProperties().tileName == "Grass Paths")
	{
		m_BattleScenes[0]->GetPrimarySprite().SetTexturePtr(game->GetAssetManager().GetSpriteTextureData(std::string("Battle_Scene_Grass")));
	}
	else if (GetTileInArrayByCoordinates(tiles, unit1->GetMapCoordinates(), maxMapX)->GetTileProperties().tileName == "Bridge")
	{
		m_BattleScenes[0]->GetPrimarySprite().SetTexturePtr(game->GetAssetManager().GetSpriteTextureData(std::string("Battle_Scene_Bridge")));

	}
	else if (GetTileInArrayByCoordinates(tiles, unit1->GetMapCoordinates(), maxMapX)->GetTileProperties().tileName == "Sand" |
		GetTileInArrayByCoordinates(tiles, unit1->GetMapCoordinates(), maxMapX)->GetTileProperties().tileName == "Shallow Water")
	{
		m_BattleScenes[0]->GetPrimarySprite().SetTexturePtr(game->GetAssetManager().GetSpriteTextureData(std::string("Battle_Scene_Sand")));

	}

}

MapTile*& GameplayManager::GetTileInArrayByCoordinates(std::vector<MapTile*>& tiles, DirectX::XMINT2& coords, int maxMapX)
{
	MapTile* tile = tiles.at((coords.x + (coords.y)) + (coords.y * maxMapX));

	return tile;
}

void GameplayManager::ResetUnit1ToIdle(UnitEntity* unit1)
{
	Unit1Reset = true;
	switch (unit1->GetUnitClassID())
	{
	case GameClasses::ASSASSIN:
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)AssassinAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::BARBARIAN:
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)BarbarianAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::ARCHER:
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)ArcherAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::NECROMANCER:
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)NecromancerAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::SORCERESS:
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)SorceressAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::WARRIOR:
		m_AttackStateUnits[0]->GetPrimarySprite().GetAnimator().SetAnimation((int)WarriorAnimIndexes::IDLE_00, true, true, false);
		break;
	}
}

void GameplayManager::ResetUnit2ToIdle(UnitEntity* unit2)
{
	Unit2Reset = true;
	switch (unit2->GetUnitClassID())
	{
	case GameClasses::ASSASSIN:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)AssassinAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::BARBARIAN:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)BarbarianAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::ARCHER:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)ArcherAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::NECROMANCER:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)NecromancerAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::SORCERESS:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)SorceressAnimIndexes::IDLE_00, true, true, false);
		break;
	case GameClasses::WARRIOR:
		m_AttackStateUnits[1]->GetPrimarySprite().GetAnimator().SetAnimation((int)WarriorAnimIndexes::IDLE_00, true, true, false);
		break;
	}
}
