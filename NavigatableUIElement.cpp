#include "NavigatableUIElement.h"

using namespace DirectX;

NavigationalMenu::NavigationalMenu(std::shared_ptr<SpriteTexture> tex, int id)
	:EntityInterface(tex), m_ID(id)
{
	m_CursorHighlight.SetTexturePtr(tex);
}

NavigationalMenu::NavigationalMenu(std::shared_ptr<SpriteTexture> menuTex, std::shared_ptr<SpriteTexture> cursorTex)
	:EntityInterface(menuTex)
{
	m_CursorHighlight.SetTexturePtr(cursorTex);
}

void NavigationalMenu::Render(DirectX::SpriteBatch& batch, DirectX::DescriptorHeap* heap)
{
	if (m_Enabled)
	{
		//Draw Main UI Element first
		GetPrimarySprite().Draw(batch, heap);

		//Render Menu Items

		//Setup values
		SimpleMath::Vector2 origin = { 0, 0 };
		XMVECTOR pos = { 0, 0 };
		int count = 0;
		for (int x(0); x < m_IndexMax.x + 1; ++x)
		{
			for (int y(0); y < m_IndexMax.y + 1; ++y)
			{
				//Calculate menu positions
				pos = { m_TextAnchorPos.x + (m_Offsets.x * x), m_TextAnchorPos.y + (m_Offsets.y * y) };
				//Calculate string origin
				CalculateStringJustifyPosition(
					m_Fonts[count],
					(short)(m_StringJustifications[count]),
					m_MenuStrings[count],
					origin
				);

				//Draw each string according to their settings
				m_Fonts[count]->DrawString(
					&batch,
					m_MenuStrings[count].c_str(),
					pos,
					m_TextColours[count],
					0.f,
					origin,
					1.f
				);

				++count;

			}
		}

		//Finally Draw Cursor
		m_CursorHighlight.Draw(batch, heap);
	}
}

void NavigationalMenu::ResetCursor()
{
	m_ActiveIndex = m_IndexDefault;
	m_CursorHighlight.SetPosition(m_CursorAnchorPos.x, m_CursorAnchorPos.y);

}
void NavigationalMenu::ReserveContainerSpace(int count)
{
	m_MenuStrings.reserve(count);
	m_TextColours.reserve(count);
	m_Fonts.reserve(count);
}
void NavigationalMenu::MoveUp()
{
	//dec index
	--m_ActiveIndex.y;
	//Check if out of range
	if (m_ActiveIndex.y < 0)
		m_ActiveIndex.y = 0;

	//Set position
	m_CursorHighlight.SetPosition( m_CursorAnchorPos.x + (m_Offsets.x * m_ActiveIndex.x), 
		m_CursorAnchorPos.y + (m_Offsets.y * m_ActiveIndex.y));
}

void NavigationalMenu::MoveDown()
{
	//dec index
	++m_ActiveIndex.y;
	//Check if out of range
	if (m_ActiveIndex.y > m_IndexMax.y)
		m_ActiveIndex.y = m_IndexMax.y;

	//Set position
	m_CursorHighlight.SetPosition(m_CursorAnchorPos.x + (m_Offsets.x * m_ActiveIndex.x),
		m_CursorAnchorPos.y + (m_Offsets.y * m_ActiveIndex.y));
}

void NavigationalMenu::MoveLeft()
{
	//dec index
	--m_ActiveIndex.x;
	//Check if out of range
	if (m_ActiveIndex.x < 0)
		m_ActiveIndex.x = 0;

	//Set position
	m_CursorHighlight.SetPosition(m_CursorAnchorPos.x + (m_Offsets.x * m_ActiveIndex.x),
		m_CursorAnchorPos.y + (m_Offsets.y * m_ActiveIndex.y));
}

void NavigationalMenu::MoveRight()
{
	//dec index
	++m_ActiveIndex.x;
	//Check if out of range
	if (m_ActiveIndex.x > m_IndexMax.x)
		m_ActiveIndex.x = m_IndexMax.x;

	//Set position
	m_CursorHighlight.SetPosition(m_CursorAnchorPos.x + (m_Offsets.x * m_ActiveIndex.x),
		m_CursorAnchorPos.y + (m_Offsets.y * m_ActiveIndex.y));
}



void NavigationalMenu::Release()
{
	m_MenuStrings.clear();
}
