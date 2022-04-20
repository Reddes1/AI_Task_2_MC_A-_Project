#include "Sprite.h"
#include "Game.h"

using namespace DirectX;

//
// Animator
//

void Animator::Update(float dTime)
{
	//Skip out if not flagged to play
	if (!m_Data.play)
		return;

	//Uptick elapsed time
	m_Data.elapsed += dTime;
	//If elapsed over animation speed
	if (m_Data.elapsed > (1.f / m_CurrAnim.animSpeed))
	{
		//Reset the clock
		m_Data.elapsed = 0.f;

		//Determine how to manage the animation based on reverse flag
		switch (m_Data.reverse)
		{
		case(false):
			//Increment the frame count
			++m_Data.currentFrame;
			//Check if out of animation range
			if (m_Data.currentFrame > m_CurrAnim.animEnd)
			{
				//If set to loop, change to first frame, else set frame to end and stop
				if (m_Data.loop)
					m_Data.currentFrame = m_CurrAnim.animStart;
				else
				{
					m_Data.currentFrame = m_CurrAnim.animEnd;
					m_Data.play = false;
				}
			}
			break;

		case(true):
			--m_Data.currentFrame;
			if (m_Data.currentFrame < m_CurrAnim.animStart)
			{
				if (m_Data.loop)
					m_Data.currentFrame = m_CurrAnim.animEnd;
				else
				{
					m_Data.currentFrame = m_CurrAnim.animStart;
					m_Data.play = false;
				}
			}
			break;
		}


		//Class Sprite Set Frame Here
		m_Sprite.SetFrame(m_Data.currentFrame);

	}
}

void Animator::SetAnimation(int index, bool play, bool loop, bool reverse)
{
	assert(index >= 0 && index <= m_Sprite.GetTexturePtr()->animations.size());
	m_CurrAnim = m_Sprite.GetTexturePtr()->animations.at(index);

	m_Data.play = play;
	m_Data.loop = loop;
	m_Data.reverse = reverse;
	m_Data.currentFrame = m_CurrAnim.animStart;

	m_Sprite.SetFrame(m_CurrAnim.animStart);
}



//
// Sprite
//

void Sprite::Draw(SpriteBatch& batch, DescriptorHeap* heap)
{
	batch.Draw(heap->GetGpuHandle(m_DrawData.srvIndex),
		m_DrawData.textureSize,
		m_DrawData.pos,
		&m_DrawData.rect,
		m_DrawData.colour,
		m_DrawData.rotation,
		m_DrawData.origin,
		m_DrawData.scale,
		m_DrawData.effect,
		m_DrawData.layerDepth		
	);
}

void Sprite::SetFrame(int index)
{
	assert(index >= 0 && index <= m_TextureData->spriteFrames.size());
	m_DrawData.rect = m_TextureData->spriteFrames.at(index);
}

void Sprite::SetAllDrawValues(SpriteDraw& newData)
{
	m_DrawData = newData;
}

void Sprite::SetAllDrawValues(int srvIndex, DirectX::XMUINT2& textureSize, DirectX::XMFLOAT2& pos, RECT& rect, DirectX::XMVECTOR& colour, float rotation,
	DirectX::XMFLOAT2& origin, DirectX::XMFLOAT2& scale, DirectX::SpriteEffects effect, float layerDepth)
{
	m_DrawData.srvIndex = srvIndex;
	m_DrawData.textureSize = textureSize;
	m_DrawData.pos = pos;
	m_DrawData.rect = rect;
	m_DrawData.colour = colour;
	m_DrawData.rotation = rotation;
	m_DrawData.origin = origin;
	m_DrawData.scale = scale;
	m_DrawData.effect = effect;
	m_DrawData.layerDepth = layerDepth;
}

void Sprite::SetTexturePtr(std::shared_ptr<SpriteTexture> tex, bool setDrawValues)
{
	assert(tex);
	m_TextureData = tex;
	m_DrawData.srvIndex = m_TextureData.get()->srvIndex;
	m_DrawData.textureSize = DirectX::GetTextureSize(m_TextureData.get()->textureResource.Get());
	m_DrawData.rect = { 0, 0, static_cast<long>(m_DrawData.textureSize.x), static_cast<long>(m_DrawData.textureSize.y) };
}

void Sprite::SetTexturePtr(std::string texName, bool setDrawValues)
{
	m_TextureData = Game::GetGame()->GetAssetManager().GetSpriteTextureData(texName);
	assert(m_TextureData.get());
	m_DrawData.srvIndex = m_TextureData.get()->srvIndex;
	m_DrawData.textureSize = DirectX::GetTextureSize(m_TextureData.get()->textureResource.Get());
	m_DrawData.rect = { 0, 0, static_cast<long>(m_DrawData.textureSize.x), static_cast<long>(m_DrawData.textureSize.y) };

}

void Sprite::DefaultSprite()
{
	XMFLOAT2 windowDim = { static_cast<float>(Game::GetGame()->GetWindowData().clientWidth), 
		static_cast<float>(Game::GetGame()->GetWindowData().clientHeight) };

	//Normalise the reset to safe values
	m_DrawData.textureSize = DirectX::GetTextureSize(m_TextureData.get()->textureResource.Get());
	m_DrawData.rect = { 0, 0, static_cast<long>(m_DrawData.textureSize.x), static_cast<long>(m_DrawData.textureSize.y) };
	m_DrawData.origin = { m_DrawData.textureSize.x * 0.5f, m_DrawData.textureSize.y * 0.5f };
	m_DrawData.pos = { windowDim.x * 0.5f, windowDim.y * 0.5f };
	m_DrawData.colour = { 1, 1, 1, 1 };
	m_DrawData.layerDepth = 1.f;
	m_DrawData.rotation = 0.f;
	m_DrawData.scale = { 1.0f, 1.0f };
	m_DrawData.effect = SpriteEffects::SpriteEffects_None;
}

void Sprite::SetOriginToCenter()
{
	//Get original size's center
	float x = (m_DrawData.rect.right - m_DrawData.rect.left) * 0.5f;
	float y = (m_DrawData.rect.bottom - m_DrawData.rect.top) * 0.5f;
	m_DrawData.origin = { x, y };
}


