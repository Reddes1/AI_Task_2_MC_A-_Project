#include "SkillInterface.h"
#include "UnitEntity.h"

float PhysicalAttack::DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer)
{
	m_Damage = (AttPlayer->GetClassTotals().TotalPhysAttack * m_Data.PhysDamageScaling) -
		DefPlayer->GetClassTotals().TotalPhysArmour;
	if (m_Damage < 0)
	{
		m_Damage = 0;
	}
	DefPlayer->GetClassTotals().CurrentHP -= m_Damage;
	return 0;
}

float PhysicalAttack::DoCrit(UnitEntity* AttPlayer, UnitEntity* DefPlayer)
{
	m_Damage = ((AttPlayer->GetClassTotals().TotalPhysAttack * m_Data.PhysDamageScaling) -
		DefPlayer->GetClassTotals().TotalPhysArmour) *
		AttPlayer->GetClassTotals().TotalCritMultiplier;
	if (m_Damage < 0)
	{
		m_Damage = 0;
	}
	DefPlayer->GetClassTotals().CurrentHP -= m_Damage;

	return 0;
}

float MagicalAttack::DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer)
{
	m_Damage = (AttPlayer->GetClassTotals().TotalMagAttack * m_Data.MagDamageScaling) -
		DefPlayer->GetClassTotals().TotalMagArmour;
	if (m_Damage < 0)
	{
		m_Damage = 0;
	}
	DefPlayer->GetClassTotals().CurrentHP -= m_Damage;

	return 0;
}

float MagicalAttack::DoCrit(UnitEntity* AttPlayer, UnitEntity* DefPlayer)
{
	m_Damage = ((AttPlayer->GetClassTotals().TotalMagAttack * m_Data.MagDamageScaling) -
		DefPlayer->GetClassTotals().TotalMagArmour) *
		AttPlayer->GetClassTotals().TotalCritMultiplier;
	if (m_Damage < 0)
	{
		m_Damage = 0;
	}
	DefPlayer->GetClassTotals().CurrentHP -= m_Damage;
	return 0;
}

float Heal::DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer)
{
	DefPlayer->GetClassTotals().CurrentHP +=
		AttPlayer->GetClassTotals().MaxHP * m_Data.HealPercent;
	if (DefPlayer->GetClassTotals().CurrentHP > DefPlayer->GetClassTotals().MaxHP)
	{
		DefPlayer->GetClassTotals().CurrentHP = DefPlayer->GetClassTotals().MaxHP;
	}

	return 0;
}

float Buff::DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer)
{
	DefPlayer->GetBuffs().CritDamageBuff = m_Data.CritDamageBuff;
	DefPlayer->GetBuffs().CritHitBuff = m_Data.CritHitBuff;
	DefPlayer->GetBuffs().EvasionBuff = m_Data.EvasionBuff;
	DefPlayer->GetBuffs().MagArmourBuff = m_Data.MagArmourBuff;
	DefPlayer->GetBuffs().MagDamageBuff = m_Data.MagDamageBuff;
	DefPlayer->GetBuffs().MagHitBuff = m_Data.MagHitBuff;
	DefPlayer->GetBuffs().MagResistBuff = m_Data.MagResistBuff;
	DefPlayer->GetBuffs().MovespeedBuff = m_Data.MovespeedBuff;
	DefPlayer->GetBuffs().PhysArmourBuff = m_Data.PhysArmourBuff;
	DefPlayer->GetBuffs().PhysDamageBuff = m_Data.PhysDamageBuff;
	DefPlayer->GetBuffs().PhysHitBuff = m_Data.PhysHitBuff;
	DefPlayer->GetBuffs().TurnCount = m_Data.TurnCount;
	DefPlayer->SetBuffState(true);
	DefPlayer->UpdateTotalValues();

	return 0;
}
