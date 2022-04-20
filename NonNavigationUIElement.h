#pragma once

#include "EntityInterface.h"

/*	
	Non-Navigatable UI Element for use with UIManager.
*/

class NonNavigationUI
{
public:

	NonNavigationUI() {}
	~NonNavigationUI() { Release(); }

	///////////
	/// Get ///
	///////////

	Sprite& GetUISprite() { return m_UISprite; }
	int GetID() { return m_ID; }
	bool GetUsageState() { return m_FreeForUse; }
	bool GetEnabledFlag() { return m_Enabled; }


	///////////
	/// Set ///
	///////////

	void SetID(int id) { m_ID = id; }
	void SetUsageState(bool free) { m_FreeForUse = free; }
	void SetEnabledFlag(bool enable) { m_Enabled = enable; };


	//////////////////
	/// Operations ///
	//////////////////

	void Render(DirectX::SpriteBatch& sb, DirectX::DescriptorHeap* heap);

	//Resets draw string to loaded defaults
	void ResetDrawStrings();
	//Changes whole string at index
	void ChangeStringByIndex(std::string& msg, unsigned int index);
	//Appends string to existing string at index
	void AppendToStringByIndex(std::string& msg, unsigned int index);
	void AppendToStringByIndex(std::stringstream& msg, unsigned int index);

	//Reserves space on all containers equal to number given
	void ReserveContainerSpace(int count);

	//Push back new string
	void StoreNewString(std::string& msg) {	m_DrawStrings.push_back(msg); m_TextStrings.push_back(msg);	}
	//Push back new string position
	void StoreNewStringPosition(DirectX::XMFLOAT2& pos) { m_TextPositions.push_back(pos); }
	//Push back new string justification id
	void StoreStringJustification(short id) { m_StringJustifications.push_back(id); }
	//Push back new colour
	void StoreNewColour(DirectX::XMVECTOR& colour) { m_TextColours.push_back(colour); }
	//Push back new font
	void StoreNewFont(DirectX::SpriteFont* font) { m_Fonts.push_back(font); }

private:

	//Clean up and release
	void Release();

	//Main UI Sprite
	Sprite m_UISprite;

	//Holds the Original Test Strings
	std::vector<std::string> m_TextStrings;
	//Holds the modifiable strings that are drawn
	std::vector<std::string> m_DrawStrings;
	//Holds colour values for each string
	std::vector<DirectX::XMVECTOR> m_TextColours;
	//Holds the UI text positions
	std::vector<DirectX::XMFLOAT2> m_TextPositions;
	//Holds pointers to spritefonts for each string
	std::vector<DirectX::SpriteFont*> m_Fonts;
	//Justification Data for each string
	std::vector<short> m_StringJustifications;

	//An ID describing what this item is (this has meaning externally)
	int m_ID = -1;
	//Track if this object is in use in another mode
	bool m_FreeForUse = true;
	//Flag for if the ui is enabled or not
	bool m_Enabled = true;
};
