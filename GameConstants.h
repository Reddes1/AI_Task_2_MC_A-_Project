#pragma once

#include "D3DUtils.h"
/*
	Any Game Level Constants that entities may need.
*/

///////////////////////////
/// Defines & Constants ///
///////////////////////////

#define TEAM_SIZE 5
#define SPRITE_SCALE_ADJ 0.20f

const DirectX::XMVECTOR TEAM_ONE_COLOUR = DirectX::Colors::DodgerBlue;
const DirectX::XMVECTOR TEAM_TWO_COLOUR = DirectX::Colors::Yellow;
const DirectX::XMVECTOR DISABLED_UNIT_COLOUR = DirectX::Colors::DarkSlateGray;

const DirectX::XMVECTOR NAVIGATABLE_MENU_DEFAULT_COLOUR = DirectX::Colors::MintCream;
const DirectX::XMVECTOR NAVIGATABLE_MENU_DISABLED_COLOUR = DirectX::Colors::Black;
const DirectX::XMVECTOR NON_NAVIGATION_DEFAULT_COLOUR = DirectX::Colors::Black;

//Skill and Projectile colours
const DirectX::XMVECTOR HEAL_UNIT_COLOUR = DirectX::Colors::GreenYellow;
const DirectX::XMVECTOR BUFF_UNIT_COLOUR = DirectX::Colors::LightYellow;
const DirectX::XMVECTOR PROJECTILE_DEFAULT_COLOUR = DirectX::Colors::Transparent;
const DirectX::XMVECTOR FIREBALL_COLOUR = DirectX::Colors::Orange;
const DirectX::XMVECTOR SMITE_COLOUR = DirectX::Colors::Yellow;
const DirectX::XMVECTOR NECROTIC_BLAST_COLOUR = DirectX::Colors::Black;
//Projectile offsets
const float ProjectileOffsetX = 100;
const float ProjectileOffsetY = 25;


////////////////////
/// Enumerations ///
////////////////////

//Class Enumerators

enum GameClasses
{
	ASSASSIN,
	BARBARIAN,
	ARCHER,
	NECROMANCER,
	SORCERESS,
	WARRIOR
};

enum class AssassinAnimIndexes
{
	ATTACK_00,
	DEFEAT_00,
	FALL_00,
	HIT_00,
	HURT_00,
	IDLE_00,
	JUMP_00,
	PICK_UP_00,
	PULL_00,
	PUSH_00,
	SPECIAL_ATTACK_00,
	WALK_00,
	SHOCKWAVE_00
};
enum class BarbarianAnimIndexes
{
	ATTACK_00,
	AXE_HIT_00,
	DEFEAT_00,
	FALL_00,
	HURT_00,
	IDLE_00,
	JUMP_00,
	PICK_UP_00,
	PULL_00,
	PUSH_00,
	SPECIAL_ATTACK_00,
	WALK_00
};
enum class ArcherAnimIndexes
{
	ATTACK_00,
	ARROW_00,
	ATTACK_01,
	DEFEAT_00,
	FALL_00,
	HURT_00,
	IDLE_00,
	JUMP_00,
	PICK_UP_00,
	PULL_00,
	PUSH_00,
	SPECIAL_ATTACK_00,
	WALK_00
};
enum class NecromancerAnimIndexes
{
	ATTACK_00,
	DEFEAT_00,
	FALL_00,
	HIT_00,
	HURT_00,
	IDLE_00,
	JUMP_00,
	PICK_UP_00,
	PULL_00,
	PUSH_00,
	SPECIAL_ATTACK_00,
	SHOT_00,
	WALK_00
};
enum class SorceressAnimIndexes
{
	ATTACK_00,
	BOOK_ATTACK_00,
	BOOK_ATTACK_01,
	DEFEAT_00,
	FALL_00,
	HIT_00,
	HURT_00,
	IDLE_00,
	JUMP_00,
	PICK_UP_00,
	PULL_00,
	PUSH_00,
	SPECIAL_ATTACK_00,
	SHOT_00,
	SPECIAL_ATTACK_01,
	WALK_00
};
enum class WarriorAnimIndexes
{
	ATTACK_00,
	DEFEAT_00,
	FALL_00,
	IDLE_00,
	JUMP_00,
	HURT_00,
	PULL_00,
	PUSH_00,
	ATTACK_01,
	WALK_00,
	SPECIAL_ATTACK_00
};
