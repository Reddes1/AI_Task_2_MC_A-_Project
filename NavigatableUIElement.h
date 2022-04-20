#pragma once

#include "EntityInterface.h"

/*
	Navigatable UI Element for use with UIManager.
*/
class NavigationalMenu : public EntityInterface
{
public:
	////////////////////
	/// Constructors ///
	////////////////////

	NavigationalMenu() {}
	NavigationalMenu(std::shared_ptr<SpriteTexture> menuTex, int id);
	NavigationalMenu(std::shared_ptr<SpriteTexture> menuTex, std::shared_ptr<SpriteTexture> cursorTex);
	~NavigationalMenu() { Release(); }

	/////////////////
	/// Overrides ///
	/////////////////

	//Don't strictly need this yet (maybe add a fading in out effect later)
	void Update(const GameTimer& gt) override { }
	void Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap) override;

	//Optional Overides (Uncomment as needed)
	//void ResetEntityToDefault() { }

	///////////
	/// Get ///
	///////////

	Sprite& GetCursorSprite() { return m_CursorHighlight; }
	DirectX::XMINT2 GetMenuIndex() const { return m_ActiveIndex; }
	DirectX::XMINT2 GetMenuIndexMax() const { return m_IndexMax; }
	DirectX::XMFLOAT2 GetOffsets() const { return m_Offsets; }
	DirectX::XMFLOAT2 GetStartPosition() const { return m_CursorAnchorPos; }
	bool GetEnabledFlag() { return m_Enabled; }

	int GetID() { return m_ID; }
	bool GetUsageState() { return m_FreeForUse; }

	///////////
	/// Set ///
	///////////

	void SetAnchorPosition(DirectX::XMFLOAT2& initPos) { m_CursorAnchorPos = initPos; }
	void SetAnchorPosition(float xAnchor, float yAnchor) { m_CursorAnchorPos.x = xAnchor; m_CursorAnchorPos.y = yAnchor; }
	void SetIndex(DirectX::XMINT2& indexes)	{ m_ActiveIndex = indexes; }
	void SetIndex(int x, int y)	{ m_ActiveIndex.x = x; m_ActiveIndex.y = y; }
	void SetMaxIndex(DirectX::XMINT2& maxIndex)	{ m_IndexMax = maxIndex; }
	void SetMaxIndex(int xMax, int yMax) { m_IndexMax.x = xMax; m_IndexMax.y = yMax; }
	void SetDefaultIndex(DirectX::XMINT2& defIndex) { m_IndexDefault = defIndex; }
	void SetDefaultIndex(int xDef, int yDef) { m_IndexDefault.x = xDef; m_IndexDefault.y = yDef; }
	void SetCursorOffset(DirectX::XMFLOAT2& offsets) { m_Offsets = offsets; }
	void SetCursorOffset(float xOffset, float yOffset) { m_Offsets.x = xOffset; m_Offsets.y = yOffset; }
	void SetTextAnchorPosition(DirectX::XMFLOAT2& initPos) { m_TextAnchorPos = initPos; }
	void SetTextAnchorPosition(float xAnchor, float yAnchor) { m_TextAnchorPos.x = xAnchor; m_TextAnchorPos.y = yAnchor; }

	void SetColourAtIndex(DirectX::XMVECTOR& newColour, int index) { m_TextColours[index] = newColour; }

	void SetID(int id) { m_ID = id; }
	void SetUsageState(bool free) { m_FreeForUse = free; }
	void SetEnabledFlag(bool enable) { m_Enabled = enable; };

	//////////////////
	/// Operations ///
	//////////////////

	//Move Cursor Up
	void MoveUp();
	//Move Cursor Down
	void MoveDown();
	//Move Cursor Left
	void MoveLeft();
	//Move Cursor Right
	void MoveRight();

	//Push back new string
	void StoreNewString(std::string& msg) { m_MenuStrings.push_back(msg); }
	//Push back new colour
	void StoreNewColour(DirectX::XMVECTOR& colour) { m_TextColours.push_back(colour); }
	//Push back new font
	void StoreNewFont(DirectX::SpriteFont* font) { m_Fonts.push_back(font); }
	//Push back new string justification id
	void StoreStringJustification(short id) { m_StringJustifications.push_back(id); }

	//Resets cursor to default position and index
	void ResetCursor();

	//Reserves space on all containers equal to number given
	void ReserveContainerSpace(int count);

private:

	void Release();

	//Menu Strings
	std::vector<std::string> m_MenuStrings;
	//Holds colour values for each string
	std::vector<DirectX::XMVECTOR> m_TextColours;
	//Holds pointers to spritefonts for each string
	std::vector<DirectX::SpriteFont*> m_Fonts;
	//Justification Data for each string
	std::vector<short> m_StringJustifications;

	//The cursor or highlight for indicating menu index
	Sprite m_CursorHighlight;	
	//Tracks where the menu cursor should be
	DirectX::XMINT2 m_ActiveIndex = { 0, 0 };
	//Denotes the last available index in the menu
	DirectX::XMINT2 m_IndexMax = { 0, 0 };
	//Index value when defaulting the cursor
	DirectX::XMINT2 m_IndexDefault = { 0, 0 };

	//This is the cursor sprites starting position. Used for positioning math
	DirectX::XMFLOAT2 m_CursorAnchorPos = { 0, 0 };
	//Multiplied with index value to offset the cursor according to menu entry
	DirectX::XMFLOAT2 m_Offsets = { 0, 0 };
	//Text position anchor
	DirectX::XMFLOAT2 m_TextAnchorPos = { 0, 0 };

	//An ID describing what this item is (this has meaning externally)
	int m_ID = -1;
	//Track if this object is in use in another mode
	bool m_FreeForUse = true;

	//Flag for if the ui is enabled or not
	bool m_Enabled = true;

};
