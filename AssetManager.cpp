#include "AssetManager.h"
#include "RapidJSONLoaderUtils.h"			//For frame loading
#include "SkillInterface.h"

using namespace DirectX;

void AssetManager::SetupNewTexture(std::string& name, std::wstring& filePath, size_t handle,
	ID3D12Device* device, ResourceUploadBatch& resourceUpload, DescriptorHeap* heap)
{
	//Create a new Resource
	Microsoft::WRL::ComPtr<ID3D12Resource> texture;

	//Do a little conversion for the following call
	const wchar_t* fp = filePath.c_str();

	//Create texture
	ThrowIfFailed(CreateDDSTextureFromFile(
		device,
		resourceUpload,
		fp,
		texture.ReleaseAndGetAddressOf())
	);

	//Create a SRV for this new texture
	CreateShaderResourceView(
		device,
		texture.Get(),
		heap->GetCpuHandle(handle)
	);

	//Create new SpriteTexture object, populate and move into container

	std::unique_ptr<SpriteTexture> newTex = std::make_unique<SpriteTexture>();

	newTex->fileName = filePath;
	newTex->textureResource = texture;
	newTex->srvIndex = handle;

	m_SprTexMap[name] = std::move(newTex);
}

void AssetManager::SetupNewTexture(std::string& name, std::wstring& filePath, std::string framesFilePath, size_t handle,
	ID3D12Device* device, DirectX::ResourceUploadBatch& resourceUpload, DirectX::DescriptorHeap* heap)
{
	//Create a new Resource
	Microsoft::WRL::ComPtr<ID3D12Resource> texture;

	//Do a little conversion for the following call
	const wchar_t* fp = filePath.c_str();

	//Create texture
	ThrowIfFailed(CreateDDSTextureFromFile(
		device,
		resourceUpload,
		fp,
		texture.ReleaseAndGetAddressOf())
	);

	//Create a SRV for this new texture
	CreateShaderResourceView(
		device,
		texture.Get(),
		heap->GetCpuHandle(handle)
	);

	//Create new SpriteTexture object, populate and move into container

	std::unique_ptr<SpriteTexture> newTex = std::make_unique<SpriteTexture>();

	newTex->fileName = filePath;
	newTex->textureResource = texture;
	newTex->srvIndex = handle;

	/////////////////////
	/// Frame Loading ///
	/////////////////////

	//Hold file data here
	rapidjson::Document doc;
	//Parse Data
	ParseNewJSONDocument(doc, framesFilePath);

	//
	// Load Frames
	//
	newTex->spriteFrames.reserve(doc["frames"].GetArray().Size());
	for (auto& a : doc["frames"].GetArray())
	{
		long left = a["frame"]["x"].GetInt();
		long top = a["frame"]["y"].GetInt();
		long right = left + a["frame"]["w"].GetInt();
		long bottom = top + a["frame"]["h"].GetInt();
		RECT frame = { left, top, right, bottom };
		newTex->spriteFrames.push_back(frame);
	}

	//
	// Load Animations
	//

	//This array has to be user defined, so if the sheet requires no animations
	//then no need to attempt to load
	if (doc.HasMember("animations"))
	{
		newTex->animations.reserve(doc["animations"].GetArray().Size());
		for (auto& a : doc["animations"].GetArray())
		{
			FrameAnimation anim;
			anim.animName = a["Animation Name"].GetString();
			anim.animStart = a["Animation Start"].GetInt();
			anim.animEnd = a["Animation End"].GetInt();
			anim.animSpeed = a["Animation Speed"].GetInt();
			newTex->animations.push_back(anim);
		}
	}



	m_SprTexMap[name] = std::move(newTex);
}

void AssetManager::LoadClasses(std::string filepath)
{
	//Hold file data
	rapidjson::Document doc;
	//Parse data
	ParseNewJSONDocument(doc, filepath);
	//Load Classes
	if (doc.HasMember("Classes"))
	{
		for (auto& a : doc["Classes"].GetArray())
		{
			std::unique_ptr<CharacterClass> newClass = std::make_unique<CharacterClass>();
			newClass->ClassName = a["Class Name"].GetString();
			newClass->UniqueID = a["Class Unique ID"].GetInt();
			newClass->HP = a["HP"].GetInt();
			newClass->MP = a["MP"].GetInt();
			newClass->Movespeed = a["Movespeed"].GetInt();
			newClass->PhysAttack = a["Physical Attack"].GetInt();
			newClass->MagAttack = a["Magical Attack"].GetInt();
			newClass->PhysHit = a["Physical Hit %"].GetInt();
			newClass->MagHit = a["Magical Hit %"].GetInt();
			newClass->CritHit = a["Critical Hit %"].GetInt();
			newClass->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newClass->PhysArmour = a["Physical Armour"].GetInt();
			newClass->MagArmour = a["Magical Armour"].GetInt();
			newClass->Evasion = a["Evasion %"].GetInt();
			newClass->MagResist = a["Magic Resist %"].GetInt();

			m_ClassMap[newClass->UniqueID] = std::move(newClass);
		}
	}
}

void AssetManager::LoadEquipment(std::string filepath)
{
	//Hold file data
	rapidjson::Document doc;
	//Parse data
	ParseNewJSONDocument(doc, filepath);

	if (doc["Weapons"].HasMember("1H Swords"))
	{
		for (auto& a : doc["Weapons"]["1H Swords"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}

	if (doc["Weapons"].HasMember("2H Swords"))
	{
		for (auto& a : doc["Weapons"]["2H Swords"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();
					
			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}

	if (doc["Weapons"].HasMember("Axes"))
	{
		for (auto& a : doc["Weapons"]["Axes"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}
	if (doc["Weapons"].HasMember("Bows"))
	{
		for (auto& a : doc["Weapons"]["Bows"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}
	if (doc["Weapons"].HasMember("Staffs"))
	{
		for (auto& a : doc["Weapons"]["Staffs"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}
	if (doc["Weapons"].HasMember("Books"))
	{
		for (auto& a : doc["Weapons"]["Books"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}

	if (doc["Armour"].HasMember("Light Armour"))
	{
		for (auto& a : doc["Armour"]["Light Armour"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}

	if (doc["Armour"].HasMember("Medium Armour"))
	{
		for (auto& a : doc["Armour"]["Medium Armour"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}

	if (doc["Armour"].HasMember("Heavy Armour"))
	{
		for (auto& a : doc["Armour"]["Heavy Armour"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}

	if (doc.HasMember("Accessories"))
	{
		for (auto& a : doc["Accessories"].GetArray())
		{
			std::unique_ptr<Equipment> newEquip = std::make_unique<Equipment>();
			newEquip->EquipmentName = a["Name"].GetString();
			newEquip->EquipmentTypeID = a["Equipment Type ID"].GetInt();
			newEquip->UniqueEquipmentID = a["Unique Equipment ID"].GetInt();
			newEquip->PhysAttack = a["Physical Attack"].GetInt();
			newEquip->MagAttack = a["Magical Attack"].GetInt();
			newEquip->PhysHit = a["Physical Hit %"].GetInt();
			newEquip->MagHit = a["Magical Hit %"].GetInt();
			newEquip->CritHit = a["Critical Hit %"].GetInt();
			newEquip->CritMultiplier = a["Critical Damage Multiplier"].GetFloat();
			newEquip->HP = a["HP"].GetInt();
			newEquip->MP = a["MP"].GetInt();
			newEquip->PhysArmour = a["Physical Armour"].GetInt();
			newEquip->MagArmour = a["Magical Armour"].GetInt();
			newEquip->Evasion = a["Evasion %"].GetInt();
			newEquip->Movespeed = a["Movespeed"].GetInt();
			newEquip->MagResist = a["Magic Resist %"].GetInt();

			m_EquipmentMap[newEquip->UniqueEquipmentID] = std::move(newEquip);
		}

	}
}

void AssetManager::LoadSkills(std::string filepath)
{
	//Hold file data
	rapidjson::Document doc;
	//Parse data
	ParseNewJSONDocument(doc, filepath);

	//has member
	const rapidjson::Value& arr = doc["Skills"];

	for (int i(0); i < doc["Skills"].GetArray().Size(); ++i)
	{
		//Each skill has different attributes, so determine what it is and load differently


		switch (doc["Skills"][i]["Skill Type"].GetInt())
		{
		case 0: //Damage Skill
			//Different damage types, so check a little more
			switch (doc["Skills"][i]["Damage Type"].GetInt())
			{
			case 0: //Physical Attack
				LoadSkillType1(arr,i);
				break;
			case 1: //Magical Attack
				LoadSkillType2(arr, i);
				break;
			}
			break;
		case 1:
			LoadSkillType3(arr, i);
			break;
		case 2:
			LoadSkillType4(arr, i);
			break;
		}
	}
}

std::shared_ptr<SpriteTexture> AssetManager::GetSpriteTextureData(std::string& name)
{
	SpriteTextureMap::iterator it = m_SprTexMap.find(name);
	if (it != m_SprTexMap.end())
		return it->second;
	else
		return nullptr;
}

std::shared_ptr<CharacterClass> AssetManager::GetClassData(int ClassUniqueID)
{
	ClassMap::iterator it = m_ClassMap.find(ClassUniqueID);
	if (it != m_ClassMap.end())
		return it->second;
	else
		return nullptr;
}

std::shared_ptr<Equipment> AssetManager::GetEquipmentData(int UniqueEquipmentID)
{
	EquipmentMap::iterator it = m_EquipmentMap.find(UniqueEquipmentID);
	if (it != m_EquipmentMap.end())
		return it->second;
	else
		return nullptr;
}

std::shared_ptr<SkillInterface> AssetManager::GetSkillData(int UniqueSkillID)
{
	SkillMap::iterator it = m_SkillMap.find(UniqueSkillID);
	if (it != m_SkillMap.end())
		return it->second;
	else
		return nullptr;
}

void AssetManager::LoadSkillType1(const rapidjson::Value& doc, int index)
{
	std::shared_ptr<PhysicalAttack> newSkill = std::make_shared<PhysicalAttack>();

	newSkill->GetData().UniqueID = doc[index]["Unique ID"].GetInt();
	newSkill->GetData().Class = doc[index]["Class"].GetString();
	newSkill->GetData().Name = doc[index]["Name"].GetString();
	newSkill->GetData().SkillType = doc[index]["Skill Type"].GetInt();
	newSkill->GetData().DamageType = doc[index]["Damage Type"].GetInt();
	newSkill->GetData().Targeting = doc[index]["Targeting"].GetInt();
	newSkill->GetData().ManaCost = doc[index]["Mana Cost"].GetInt();
	newSkill->GetData().Range = doc[index]["Range"].GetInt();
	newSkill->GetData().Radius = doc[index]["Radius"].GetInt();
	newSkill->GetData().PhysDamageScaling = doc[index]["Physical Damage Scaling"].GetFloat();
	newSkill->GetData().MagDamageScaling = doc[index]["Magic Damage Scaling"].GetFloat();
	newSkill->GetData().InnateCrit = doc[index]["Innate Crit %"].GetFloat();
	newSkill->GetData().ToolTip = doc[index]["Tool Tip"].GetString();
	
	m_SkillMap[newSkill->GetData().UniqueID] = std::move(newSkill);
}

void AssetManager::LoadSkillType2(const rapidjson::Value& doc, int index)
{
	std::shared_ptr<MagicalAttack> newSkill = std::make_shared<MagicalAttack>();
	
	newSkill->GetData().UniqueID = doc[index]["Unique ID"].GetInt();
	newSkill->GetData().Class = doc[index]["Class"].GetString();
	newSkill->GetData().Name = doc[index]["Name"].GetString();
	newSkill->GetData().SkillType = doc[index]["Skill Type"].GetInt();
	newSkill->GetData().DamageType = doc[index]["Damage Type"].GetInt();
	newSkill->GetData().Targeting = doc[index]["Targeting"].GetInt();
	newSkill->GetData().ManaCost = doc[index]["Mana Cost"].GetInt();
	newSkill->GetData().Range = doc[index]["Range"].GetInt();
	newSkill->GetData().Radius = doc[index]["Radius"].GetInt();
	newSkill->GetData().PhysDamageScaling = doc[index]["Physical Damage Scaling"].GetFloat();
	newSkill->GetData().MagDamageScaling = doc[index]["Magic Damage Scaling"].GetFloat();
	newSkill->GetData().InnateCrit = doc[index]["Innate Crit %"].GetFloat();
	newSkill->GetData().ToolTip = doc[index]["Tool Tip"].GetString();

	m_SkillMap[newSkill->GetData().UniqueID] = std::move(newSkill);
}

void AssetManager::LoadSkillType3(const rapidjson::Value& doc, int index)
{
	std::shared_ptr<Heal> newSkill = std::make_shared<Heal>();

	newSkill->GetData().UniqueID = doc[index]["Unique ID"].GetInt();
	newSkill->GetData().Class = doc[index]["Class"].GetString();
	newSkill->GetData().Name = doc[index]["Name"].GetString();
	newSkill->GetData().SkillType = doc[index]["Skill Type"].GetInt();
	newSkill->GetData().Targeting = doc[index]["Targeting"].GetInt();
	newSkill->GetData().ManaCost = doc[index]["Mana Cost"].GetInt();
	newSkill->GetData().Range = doc[index]["Range"].GetInt();
	newSkill->GetData().Radius = doc[index]["Radius"].GetInt();
	newSkill->GetData().HealPercent = doc[index]["Heal %"].GetFloat();
	newSkill->GetData().ToolTip = doc[index]["Tool Tip"].GetString();

	m_SkillMap[newSkill->GetData().UniqueID] = std::move(newSkill);
}

void AssetManager::LoadSkillType4(const rapidjson::Value& doc, int index)
{
	std::shared_ptr<Buff> newSkill = std::make_shared<Buff>();

	newSkill->GetData().UniqueID = doc[index]["Unique ID"].GetInt();
	newSkill->GetData().Class = doc[index]["Class"].GetString();
	newSkill->GetData().Name = doc[index]["Name"].GetString();
	newSkill->GetData().SkillType = doc[index]["Skill Type"].GetInt();
	newSkill->GetData().Targeting = doc[index]["Targeting"].GetInt();
	newSkill->GetData().ManaCost = doc[index]["Mana Cost"].GetInt();
	newSkill->GetData().TurnCount = doc[index]["Turn Count"].GetInt();
	newSkill->GetData().Range = doc[index]["Range"].GetInt();
	newSkill->GetData().Radius = doc[index]["Radius"].GetInt();
	newSkill->GetData().PhysDamageBuff = doc[index]["Physical Damage Buff %"].GetFloat();
	newSkill->GetData().MagDamageBuff = doc[index]["Magical Damage Buff %"].GetFloat();
	newSkill->GetData().PhysHitBuff = doc[index]["Physical Hit Buff %"].GetFloat();
	newSkill->GetData().MagHitBuff = doc[index]["Magical Hit Buff %"].GetFloat();
	newSkill->GetData().PhysArmourBuff = doc[index]["Physical Armour Buff %"].GetFloat();
	newSkill->GetData().MagArmourBuff = doc[index]["Magic Armour Buff %"].GetFloat();
	newSkill->GetData().MagResistBuff = doc[index]["Magic Resist Buff %"].GetFloat();
	newSkill->GetData().CritHitBuff = doc[index]["Crit Hit Buff %"].GetFloat();
	newSkill->GetData().CritDamageBuff = doc[index]["Crit Damage Buff %"].GetFloat();
	newSkill->GetData().EvasionBuff = doc[index]["Evasion Buff %"].GetFloat();
	newSkill->GetData().MovespeedBuff = doc[index]["Movespeed Buff %"].GetFloat();
	newSkill->GetData().ToolTip = doc[index]["Tool Tip"].GetString();

	m_SkillMap[newSkill->GetData().UniqueID] = std::move(newSkill);
}




