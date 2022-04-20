#pragma once

#include "D3DUtils.h"
#include <string>

class UnitEntity;

class SkillInterface
{
public:

	virtual float DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer) { return 0; };
	virtual float DoCrit(UnitEntity* AttPlayer, UnitEntity* DefPlayer) { return 0; };
	virtual const std::string& GetActionName() = 0;
	virtual const std::string& GetTooltip() = 0;
	virtual int GetManaCost() = 0;
	virtual int GetSkillType() = 0;
	virtual int GetTargeting() = 0;
	virtual int GetDamageType() { return -1; };
	virtual int GetSkillRange() = 0;
	virtual int GetSkillRadius() = 0;
	virtual float GetSkillCrit() { return -1; };
private:

};

/*
* Four different skill types
*	Physical Attack
*	Magical Attack
*	Heal
*	Buff
*/
class PhysicalAttack : public SkillInterface
{
public:
	//Calculates damage value
	float DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer) override;
	float DoCrit(UnitEntity* player1, UnitEntity* player2) override;
	//Get//
	const std::string& GetActionName() override { return m_Data.Name; }
	std::string& GetTooltip() override { return m_Data.ToolTip; }
	int GetManaCost() override { return m_Data.ManaCost; }
	int GetSkillType() override { return m_Data.SkillType; }
	int GetDamageType() override { return m_Data.DamageType; }
	int GetTargeting() override { return m_Data.Targeting; }
	int GetSkillRange() override { return m_Data.Range; }
	int GetSkillRadius() override { return m_Data.Radius; }
	float GetSkillCrit() override { return m_Data.InnateCrit; }
	DamageSkills& GetData() { return m_Data; }
	
private:
	DamageSkills m_Data;
	float m_Damage = 0.0f;
};

class MagicalAttack : public SkillInterface
{
public:
	//Calculates damage value
	float DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer) override;
	float DoCrit(UnitEntity* AttPlayer, UnitEntity* DefPlayer) override;
	//Get//
	const std::string& GetActionName() override { return m_Data.Name; }
	std::string& GetTooltip() override { return m_Data.ToolTip; }
	int GetManaCost() override { return m_Data.ManaCost; }
	int GetSkillType() override { return m_Data.SkillType; }
	int GetDamageType() override { return m_Data.DamageType; }
	int GetTargeting() override { return m_Data.Targeting; }
	int GetSkillRange() override { return m_Data.Range; }
	int GetSkillRadius() override { return m_Data.Radius; }
	float GetSkillCrit() override { return m_Data.InnateCrit; }
	DamageSkills& GetData() { return m_Data; }
private:
	DamageSkills m_Data;
	float m_Damage = 0.0f;
};

class Heal : public SkillInterface
{
public:
	//Calculates heal value
	float DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer) override;
	//Get//
	const std::string& GetActionName() override { return m_Data.Name; }
	std::string& GetTooltip() override { return m_Data.ToolTip; }

	int GetManaCost() override { return m_Data.ManaCost; }
	int GetSkillType() override { return m_Data.SkillType; }
	int GetTargeting() override { return m_Data.Targeting; }
	int GetSkillRange() override { return m_Data.Range; }
	int GetSkillRadius() override { return m_Data.Radius; }
	HealSkills& GetData() { return m_Data; }
private:
	HealSkills m_Data;

};

class Buff : public SkillInterface
{
public:
	//Applies buffs
	float DoAction(UnitEntity* AttPlayer, UnitEntity* DefPlayer) override;
	 //Get//
	const std::string& GetActionName()override { return m_Data.Name; }
	std::string& GetTooltip() override { return m_Data.ToolTip; }

	int GetManaCost() override { return m_Data.ManaCost; }
	int GetSkillType() override { return m_Data.SkillType; }
	int GetTargeting() override { return m_Data.Targeting; }
	int GetSkillRange() override { return m_Data.Range; }
	int GetSkillRadius() override { return m_Data.Radius; }
	BuffSkills& GetData() { return m_Data; }
private:
	BuffSkills m_Data;

};