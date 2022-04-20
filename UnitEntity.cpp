#include "UnitEntity.h"
#include "Game.h"			//Manager Access

using namespace DirectX;

UnitEntity::UnitEntity(std::shared_ptr<SpriteTexture> texture)
	:EntityInterface(texture)
{
	InitUnit();
}

void UnitEntity::Update(const GameTimer& gt)
{
	if (m_EntityData.isActive)
	{
		m_PrimarySprite.GetAnimator().Update(gt.DeltaTime());
	}
}

void UnitEntity::SetUnitClass(int id)
{
	m_UnitClass = Game::GetGame()->GetAssetManager().GetClassData(id);
}

void UnitEntity::SetUnitBaseEquipment(int MainHand, int OffHand, int Armour, int Trinket1, int Trinket2)
{
	m_UnitEquipment.push_back(Game::GetGame()->GetAssetManager().GetEquipmentData(MainHand));
	m_UnitEquipment.push_back(Game::GetGame()->GetAssetManager().GetEquipmentData(OffHand));
	m_UnitEquipment.push_back(Game::GetGame()->GetAssetManager().GetEquipmentData(Armour));
	m_UnitEquipment.push_back(Game::GetGame()->GetAssetManager().GetEquipmentData(Trinket1));
	m_UnitEquipment.push_back(Game::GetGame()->GetAssetManager().GetEquipmentData(Trinket2));
}

void UnitEntity::SetBaseSkills(int Basic, int Skill1, int Skill2, int Skill3)
{
	m_Skills.clear();

	m_Skills.push_back(Game::GetGame()->GetAssetManager().GetSkillData(Basic));
	m_Skills.push_back(Game::GetGame()->GetAssetManager().GetSkillData(Skill1)); 
	m_Skills.push_back(Game::GetGame()->GetAssetManager().GetSkillData(Skill2));
	m_Skills.push_back(Game::GetGame()->GetAssetManager().GetSkillData(Skill3));
}

void UnitEntity::SetSkillsBasedOnClass()
{
	switch (m_UnitClass->UniqueID)
	{
	case 0: //Assassin
		SetBaseSkills(0, 5, 16, 17);
		break;
	case 1: //Barbarian
		SetBaseSkills(0, 7, 11, 14);
		break;
	case 2: //Archer
		SetBaseSkills(1, 6, 8, 15);
		break;
	case 3: //Necromancer
		SetBaseSkills(1, 4, 13, 18);
		break;
	case 4: //Sorceress
		SetBaseSkills(1, 2, 3, 19);
		break;
	case 5: //Cleric
		SetBaseSkills(0, 9, 10, 12);
		break;
	}
}


void UnitEntity::Move(int dir)
{
	switch (dir)
	{
	case 0:
		--m_MapCoordinates.y;
		if (m_MapCoordinates.y < 0)
		{
			m_MapCoordinates.y = 0;
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		else
		{
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		break;

	case 1:
		++m_MapCoordinates.y;
		if (m_MapCoordinates.y > 22)
		{
			m_MapCoordinates.y = 22;
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		else
		{
			m_PrimarySprite.SetPosition(m_PrimarySprite.GetPosition().x, (m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
		}
		break;
	case 2:
		--m_MapCoordinates.x;
		if (m_MapCoordinates.x < 0)
		{
			m_MapCoordinates.x = 0;
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		else
		{
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		break;
	case 3:
		++m_MapCoordinates.x;
		if (m_MapCoordinates.x > 39)
		{
			m_MapCoordinates.x = 39;
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		else
		{
			m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f, m_PrimarySprite.GetPosition().y);
		}
		break;
	}
}

void UnitEntity::MoveToCoordinate(DirectX::XMINT2& newCoords)
{
	//Set new coords
	m_MapCoordinates = newCoords;
	//Adjust sprite position
	m_PrimarySprite.SetPosition((m_MapCoordinates.x * m_TileSize.x) + m_TileSize.x * 0.5f,
		(m_MapCoordinates.y * m_TileSize.y) + m_TileSize.y * 0.5f);
}

bool UnitEntity::CheckRemainingActions()
{
	return (m_StateFlags.canAct || m_StateFlags.canMove);
}

void UnitEntity::DisableUnit()
{
	m_PrimarySprite.SetColour(XMVECTOR(Colors::DarkSlateGray));
	m_StateFlags.canAct = false;
	m_StateFlags.canMove = false;
}

void UnitEntity::EnableUnit()
{
	m_PrimarySprite.SetColour(XMVECTOR({ 1, 1, 1, 1 }));
	m_StateFlags.canAct = true;
	m_StateFlags.canMove = true;
}

void UnitEntity::InitTotalValues()
{
	MaxHealth();
	MaxMP();
	InitCurrentHealth();
	InitCurrentMP();
	TotalPhysValues();
	TotalMagValues();
	OtherTotalValues();
}

void UnitEntity::UpdateTotalValues()
{
	MaxHealth();
	MaxMP();
	TotalPhysValues();
	TotalMagValues();
	OtherTotalValues();
}

void UnitEntity::InitUnit()
{
	//Safe reservation amount
	m_UnitEquipment.reserve(5);
	m_Skills.reserve(4);
}

void UnitEntity::MaxHealth()
{
	m_ClassTotals.MaxHP = m_UnitClass->HP + m_UnitEquipment[0]->HP + m_UnitEquipment[1]->HP +
		m_UnitEquipment[2]->HP + m_UnitEquipment[3]->HP + m_UnitEquipment[4]->HP;
}

void UnitEntity::MaxMP()
{
	m_ClassTotals.MaxMP = m_UnitClass->MP + m_UnitEquipment[0]->MP + m_UnitEquipment[1]->MP +
		m_UnitEquipment[2]->MP + m_UnitEquipment[3]->MP + m_UnitEquipment[4]->MP;
}

void UnitEntity::InitCurrentHealth()
{
	m_ClassTotals.CurrentHP = m_ClassTotals.MaxHP;
}

void UnitEntity::InitCurrentMP()
{
	m_ClassTotals.CurrentMP = m_ClassTotals.MaxMP;
}

void UnitEntity::TotalPhysValues()
{
	//Total Physical Armour
	m_ClassTotals.TotalPhysArmour = (m_UnitClass->PhysArmour + m_UnitEquipment[0]->PhysArmour +
		m_UnitEquipment[1]->PhysArmour + m_UnitEquipment[2]->PhysArmour +
		m_UnitEquipment[3]->PhysArmour + m_UnitEquipment[4]->PhysArmour) *
		m_CurrentBuffs.PhysArmourBuff;
	//Total Physical Attack
	m_ClassTotals.TotalPhysAttack = (m_UnitClass->PhysAttack + m_UnitEquipment[0]->PhysAttack +
		m_UnitEquipment[1]->PhysAttack + m_UnitEquipment[2]->PhysAttack +
		m_UnitEquipment[3]->PhysAttack + m_UnitEquipment[4]->PhysAttack) * 
		m_CurrentBuffs.PhysDamageBuff;
	//Total Physical Hit %
	m_ClassTotals.TotalPhysHit = ((m_UnitClass->PhysHit + m_UnitEquipment[0]->PhysHit +
		m_UnitEquipment[1]->PhysHit + m_UnitEquipment[2]->PhysHit +
		m_UnitEquipment[3]->PhysHit + m_UnitEquipment[4]->PhysHit) * 
		m_CurrentBuffs.PhysHitBuff);
}

void UnitEntity::TotalMagValues()
{
	//Total Magic Armour
	m_ClassTotals.TotalMagArmour = (m_UnitClass->MagArmour + m_UnitEquipment[0]->MagArmour +
		m_UnitEquipment[1]->MagArmour + m_UnitEquipment[2]->MagArmour +
		m_UnitEquipment[3]->MagArmour + m_UnitEquipment[4]->MagArmour) *
		m_CurrentBuffs.MagArmourBuff;
	//Total Magic Attack
	m_ClassTotals.TotalMagAttack = (m_UnitClass->MagAttack + m_UnitEquipment[0]->MagAttack +
		m_UnitEquipment[1]->MagAttack + m_UnitEquipment[2]->MagAttack +
		m_UnitEquipment[3]->MagAttack + m_UnitEquipment[4]->MagAttack) * 
		m_CurrentBuffs.MagDamageBuff;
	//Total Magic hit %
	m_ClassTotals.TotalMagHit = ((m_UnitClass->MagHit + m_UnitEquipment[0]->MagHit +
		m_UnitEquipment[1]->MagHit + m_UnitEquipment[2]->MagHit +
		m_UnitEquipment[3]->MagHit + m_UnitEquipment[4]->MagHit) *
		m_CurrentBuffs.MagHitBuff);
	//Total Maigic Resist
	m_ClassTotals.TotalMagResist = (m_UnitClass->MagResist + m_UnitEquipment[0]->MagResist +
		m_UnitEquipment[1]->MagResist + m_UnitEquipment[2]->MagResist +
		m_UnitEquipment[4]->MagResist + m_UnitEquipment[4]->MagResist) *
		m_CurrentBuffs.MagResistBuff;

}

void UnitEntity::OtherTotalValues()
{
	//Total Critical Hit %
	m_ClassTotals.TotalCritHit = (m_UnitClass->CritHit + m_UnitEquipment[0]->CritHit +
		m_UnitEquipment[1]->CritHit + m_UnitEquipment[2]->CritHit +
		m_UnitEquipment[3]->CritHit + m_UnitEquipment[4]->CritHit) *
		m_CurrentBuffs.CritHitBuff;
	//Total Critical Damage Multiplier
	m_ClassTotals.TotalCritMultiplier = (m_UnitClass->CritMultiplier + m_UnitEquipment[0]->CritMultiplier +
		m_UnitEquipment[1]->CritMultiplier + m_UnitEquipment[2]->CritMultiplier +
		m_UnitEquipment[3]->CritMultiplier + m_UnitEquipment[4]->CritMultiplier) *
		m_CurrentBuffs.CritDamageBuff;
	//Total Evasion
	m_ClassTotals.TotalEvasion = (m_UnitClass->Evasion + m_UnitEquipment[0]->Evasion +
		m_UnitEquipment[1]->Evasion + m_UnitEquipment[2]->Evasion +
		m_UnitEquipment[3]->Evasion + m_UnitEquipment[4]->Evasion) *
		m_CurrentBuffs.EvasionBuff;
	//Total Movespeed
	m_ClassTotals.TotalMovespeed = (m_UnitClass->Movespeed + m_UnitEquipment[0]->Movespeed +
		m_UnitEquipment[1]->Movespeed + m_UnitEquipment[2]->Movespeed +
		m_UnitEquipment[3]->Movespeed + m_UnitEquipment[4]->Movespeed) *
		m_CurrentBuffs.MovespeedBuff;
}

void UnitEntity::ResetBuffValues()
{
	m_CurrentBuffs.PhysDamageBuff	= 1.0f;
	m_CurrentBuffs.MagDamageBuff	= 1.0f;
	m_CurrentBuffs.PhysHitBuff		= 1.0f;
	m_CurrentBuffs.MagHitBuff		= 1.0f;
	m_CurrentBuffs.PhysArmourBuff	= 1.0f;
	m_CurrentBuffs.MagArmourBuff	= 1.0f;
	m_CurrentBuffs.MagResistBuff	= 1.0f;
	m_CurrentBuffs.CritHitBuff		= 1.0f;
	m_CurrentBuffs.CritDamageBuff	= 1.0;
	m_CurrentBuffs.EvasionBuff		= 1.0f;
	m_CurrentBuffs.MovespeedBuff	= 1.0f;
	m_CurrentBuffs.TurnCount		= 0;
	SetBuffState(false);
	UpdateTotalValues();
}

