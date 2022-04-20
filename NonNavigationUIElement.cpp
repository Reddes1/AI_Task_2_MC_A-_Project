#include "NonNavigationUIElement.h"

using namespace DirectX;

void NonNavigationUI::Render(DirectX::SpriteBatch& sb, DirectX::DescriptorHeap* heap)
{
	if (m_Enabled)
	{
		//Draw Main UI Element first
		m_UISprite.Draw(sb, heap);

		//Draw through each string,
		SimpleMath::Vector2 origin = { 0, 0 };
		for (int i(0); i < m_DrawStrings.size(); ++i)
		{
			CalculateStringJustifyPosition(
				m_Fonts[i],
				(short)(m_StringJustifications[i]),
				m_DrawStrings[i],
				origin
			);

			//Draw each string according to their settings
			m_Fonts[i]->DrawString(
				&sb,
				m_DrawStrings[i].c_str(),
				m_TextPositions[i],
				m_TextColours[i],
				0.f,
				origin,
				1.f
			);
		}
	}
}

void NonNavigationUI::ResetDrawStrings()
{
	m_DrawStrings = m_TextStrings;
}

void NonNavigationUI::ChangeStringByIndex(std::string& msg, unsigned int index)
{
	m_DrawStrings[index] = msg;
}

void NonNavigationUI::AppendToStringByIndex(std::string& msg, unsigned int index)
{
	m_DrawStrings[index].append(msg);
}

void NonNavigationUI::AppendToStringByIndex(std::stringstream& msg, unsigned int index)
{
	m_DrawStrings[index].append(msg.str());
}

void NonNavigationUI::ReserveContainerSpace(int count)
{
	m_TextStrings.reserve(count);
	m_DrawStrings.reserve(count);
	m_TextColours.reserve(count);
	m_TextPositions.reserve(count);
	m_Fonts.reserve(count);
}

void NonNavigationUI::Release()
{
	m_TextPositions.clear();
	m_TextStrings.clear();
}

