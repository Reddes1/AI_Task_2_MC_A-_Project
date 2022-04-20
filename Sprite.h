#pragma once

#include "SpriteBatch.h"
#include "AssetManager.h"	//Type Information + Loading

/*
	Animator Class to be used with Sprite Class
*/

class Sprite;
class Animator
{
public:

	//////////////////
	/// Structures ///
	//////////////////

	struct AnimatorData
	{
		AnimatorData()
			:elapsed(0), currentFrame(0), play(false), loop(false), reverse(false)
		{}
		float elapsed;
		short int currentFrame;
		bool play : 1;
		bool loop : 1;
		bool reverse : 1;
	};

	////////////////////
	/// Constructors ///
	////////////////////

	Animator(Sprite& spr)
		:m_Sprite(spr)
	{}
	~Animator() {  }


	//////////////////
	/// Operations ///
	//////////////////

	// Flag Start/Stop for Animation
	void Play(bool play) { m_Data.play = play; }
	// Update and change frame if required
	void Update(float dTime);


	///////////
	/// Get ///
	///////////

	//Get AnimatorData (Temp)
	AnimatorData& GetAnimatorData() { return m_Data; }


	///////////
	/// Set ///
	///////////


	/////////////
	/// Setup ///
	/////////////

	//Set Animation by index through sprite texture data
	//Optional control settings.
	void SetAnimation(int index, bool play = true, bool loop = true, bool reverse = false);

private:

	FrameAnimation m_CurrAnim;
	AnimatorData m_Data;
	Sprite& m_Sprite;

};

/*
	DX12 Sprite Class. Used with DirectXTK SpriteBatch calls.
*/
class Sprite
{
public:

	//////////////////
	/// Structures ///
	//////////////////

	//All the Draw call arguments
	struct SpriteDraw
	{
		SpriteDraw()
			:srvIndex(0),
			textureSize(1920, 1080),
			pos(0, 0),
			rect({ 0, 0, 1920, 1080 }),
			colour({ 1,1,1,1 }),
			rotation(0),
			origin(0, 0),
			scale(1, 1),
			effect(DirectX::SpriteEffects::SpriteEffects_None),
			layerDepth(1)
		{ }

		//Reordered for packing purposes
		DirectX::XMVECTOR colour;
		RECT rect;
		DirectX::XMUINT2 textureSize;
		DirectX::XMFLOAT2 origin;
		DirectX::XMFLOAT2 scale;
		DirectX::XMFLOAT2 pos;
		size_t srvIndex;
		float rotation;
		float layerDepth;
		DirectX::SpriteEffects effect;

		//Support Overload
		SpriteDraw& operator=(const SpriteDraw& rSide)
		{
			srvIndex = rSide.srvIndex;
			textureSize = rSide.textureSize;
			pos = rSide.pos;
			rect = rSide.rect;
			colour = rSide.colour;
			rotation = rSide.rotation;
			origin = rSide.origin;
			scale = rSide.scale;
			effect = rSide.effect;
			layerDepth = rSide.layerDepth;
			return *this;
		}

	};

	////////////////////
	/// Constructors ///
	////////////////////

	Sprite()
		: m_Animator(*this)		
	{}

	//////////////////
	/// Operations ///
	//////////////////

	void Draw(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap);


	///////////
	/// Set ///
	///////////

	//
	//Sprite Data Setters
	//
	void SetSRVIndex(size_t newIndex)                 { m_DrawData.srvIndex = newIndex; }
	void SetTexSize(DirectX::XMUINT2 newTexSize)      { m_DrawData.textureSize = newTexSize; }
	void SetTexSize(uint32_t x, uint32_t y)           { m_DrawData.textureSize.x = x; m_DrawData.textureSize.y = y; }
	void SetPosition(DirectX::XMFLOAT2& newPos)       { m_DrawData.pos = newPos; }
	void SetPosition(float x, float y)                { m_DrawData.pos.x = x; m_DrawData.pos.y = y; }
	void SetRect(RECT newRect)                        { m_DrawData.rect = newRect; }
	void SetColour(DirectX::XMVECTOR newColour)       { m_DrawData.colour = newColour; }
	void SetRotation(float newRotation)               { m_DrawData.rotation = newRotation; }
	void SetOrigin(DirectX::XMFLOAT2 newOrigin)       { m_DrawData.origin = newOrigin; }
	void SetOrigin(float x, float y)                  { m_DrawData.origin.x = x; m_DrawData.origin.y = y; }
	void SetScale(DirectX::XMFLOAT2 newScale)         { m_DrawData.scale = newScale; }
	void SetScale(float x, float y)                   { m_DrawData.scale.x = x; m_DrawData.scale.y = y; }
	void SetEffect(DirectX::SpriteEffects newEffect)  { m_DrawData.effect = newEffect; }
	void SetLayerDepth(float newDepth)                { m_DrawData.layerDepth = newDepth; }


	//Set origin to center of texture using current RECT
	void SetOriginToCenter();
	//Set Frame through Texture Data (Called by Animator)
	void SetFrame(int index);

	//Increment rotation by a value (bundles common rot += value calculation)
	void IncrementRotation(float incValue) { m_DrawData.rotation += incValue; }
	//Use a SpriteDraw Object to set data
	void SetAllDrawValues(SpriteDraw& newData);
	//Alternate method, set all values at once individually
	void SetAllDrawValues(int srvIndex, DirectX::XMUINT2& textureSize, DirectX::XMFLOAT2& pos,
		RECT& rect, DirectX::XMVECTOR& colour, float rotation, DirectX::XMFLOAT2& origin,
		DirectX::XMFLOAT2& scale, DirectX::SpriteEffects effect, float layerDepth);

	//Set Texture via Pointer (OPTIONAL: Set flag to set new TextureSize & Rect from data)
	void SetTexturePtr(std::shared_ptr<SpriteTexture> tex, bool setDrawValues = false);
	//Set Texture via name through Manager (OPTIONAL: Set flag to set new TextureSize & Rect from data)
	void SetTexturePtr(std::string texName, bool setDrawValues = false);


	///////////
	/// Get ///
	///////////

	//
	//Sprite Data Getters
	//
	size_t GetSRVIndex() const                        { return m_DrawData.srvIndex; }
	DirectX::XMUINT2 GetTexSize() const               { return m_DrawData.textureSize; }
	DirectX::XMFLOAT2 GetPosition() const             { return m_DrawData.pos; }
	RECT GetRect() const                              { return m_DrawData.rect; }
	DirectX::XMVECTOR GetColour() const               { return m_DrawData.colour; }
	float GetRotation() const                         { return m_DrawData.rotation; }
	DirectX::XMFLOAT2 GetOrigin() const               { return m_DrawData.origin; }
	DirectX::XMFLOAT2 GetScale() const                { return m_DrawData.scale; }
	DirectX::SpriteEffects GetSpriteEffect() const    { return m_DrawData.effect; }
	float GetLayerDepth() const                       { return m_DrawData.layerDepth; }

	Animator& GetAnimator()						      { return m_Animator; }
													  
	//Get the texture data assigned to this object	  
	std::shared_ptr<SpriteTexture> GetTexturePtr()    { return m_TextureData; }


	/////////////
	/// Utils ///
	/////////////

	/*
		Returns the sprites to a safe default state (middle of window, texture default size,
		no scaling or rotations etc). This excludes the SRV Index.
	*/
	void DefaultSprite();


private:
	//Animation Manager
	Animator m_Animator;
	//Everything called in the draw
	SpriteDraw m_DrawData;
	//Handle to the texture this object is using
	std::shared_ptr<SpriteTexture> m_TextureData;
};