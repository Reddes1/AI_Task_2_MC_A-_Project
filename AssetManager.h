#pragma once

#include "D3DUtils.h"       //Get Types	
#include "RapidJSONLoaderUtils.h"
#include "SkillInterface.h" 


//Type defs
typedef std::unordered_map<std::string, std::shared_ptr<SpriteTexture>> SpriteTextureMap;
typedef std::unordered_map<int, std::shared_ptr<CharacterClass>> ClassMap;
typedef std::unordered_map<int, std::shared_ptr<Equipment>> EquipmentMap;
typedef std::unordered_map<int, std::shared_ptr<SkillInterface>> SkillMap;
class AssetManager
{
public:

	////////////////////
	/// Data Loading ///
	////////////////////

	//Call this ONCE for each texture you want to load
	void SetupNewTexture(std::string& name, std::wstring& filePath, size_t handle, ID3D12Device* device,
		DirectX::ResourceUploadBatch& resourceUpload, DirectX::DescriptorHeap* heap);
	//Call this ONCE for each texture WITH frames you want to load (Use "Free Texture Packer" Json output)
	void SetupNewTexture(std::string& name, std::wstring& filePath, std::string framesFilePath, size_t handle, ID3D12Device* device,
		DirectX::ResourceUploadBatch& resourceUpload, DirectX::DescriptorHeap* heap);
	//Load in all of the class data
	void LoadClasses(std::string filepath);
	//Load in all of the equipment data
	void LoadEquipment(std::string filepath);
	//Load in all of the Skill data
	void LoadSkills(std::string filepath);
	///////////
	/// Get ///
	///////////
	// 
	//Gets sprite texture by name and returns the pointer to the data
	std::shared_ptr<SpriteTexture> GetSpriteTextureData(std::string& name);

	//Get the number of textures loaded
	size_t GetSprTexCount() { return m_SprTexMap.size(); }

	//Get the Class data by id and returns the pointer to the data
	std::shared_ptr<CharacterClass> GetClassData(int ClassUniqueID);

	//Get the Equipment data by id and returns the pointer to the data
	std::shared_ptr<Equipment> GetEquipmentData(int UniqueEquipmentID);
	//Get the Skill data by id and returns the pointer to the data
	std::shared_ptr<SkillInterface>GetSkillData(int UniqueSkillID);
	//SkillMap& GetSkillData() { return m_SkillMap; }
	 //////////////////////
	// SpriteFont Stuff //
   //////////////////////

	
private:

	SpriteTextureMap m_SprTexMap;
	ClassMap m_ClassMap;
	EquipmentMap m_EquipmentMap;
	SkillMap m_SkillMap;

	//Skills loading has some wrinkles, so break down the loading into sub functions
	//Physical Attack
	void LoadSkillType1(const rapidjson::Value& doc, int index);
	//Magical Attack
	void LoadSkillType2(const rapidjson::Value& doc, int index);
	//Heal
	void LoadSkillType3(const rapidjson::Value& doc, int index);
	//Buff
	void LoadSkillType4(const rapidjson::Value& doc, int index);
};