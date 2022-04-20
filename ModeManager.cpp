#include "ModeManager.h"

#include <assert.h>

bool ModeManager::SwitchMode(size_t id)
{
	//Check there are modes loaded
	assert(!m_Modes.empty());
	//Check through the modes to find a valid mode index
	for (auto& a : m_Modes)
	{
		if (a->GetModeID() == id)
		{
			m_NextModeIndex = static_cast<int>(id);
			return true;
		}
	}
	//Failed to find mode
	assert(false);
	return false;
}

ModeManager::ModeManager()
{
	//Reserve a starting amount
	m_Modes.reserve(8);
}

void ModeManager::Update(const GameTimer& gt)
{
	//Check to see if we have a pending mode shift
	if (m_NextModeIndex != m_CurrentModeIndex)
		
		//If the current index is in valid or if the current mode has returned a true from its Exit Call
		if (m_CurrentModeIndex == -1 || m_Modes[m_CurrentModeIndex]->Exit())
		{
			//Shift to the new mode index and call its Enter Function
			m_CurrentModeIndex = m_NextModeIndex;
			m_Modes[m_CurrentModeIndex]->Enter();
		}
	//Update the current mode
	m_Modes[m_CurrentModeIndex]->Update(gt);
}

void ModeManager::Render(PassData& data)
{
	//Check that we are attempting to render a valid index
	if (m_CurrentModeIndex >= 0 && m_CurrentModeIndex < (int)m_Modes.size())
		m_Modes[m_CurrentModeIndex]->Render(data);
}

void ModeManager::ProcessKey(char key)
{
	//Check that we are attempting to render a valid index
	if (m_CurrentModeIndex >= 0 && m_CurrentModeIndex < (int)m_Modes.size())
		m_Modes[m_CurrentModeIndex]->ProcessKey(key);
}

void ModeManager::AddMode(ModeInterface* p)
{
	assert(p);
	m_Modes.push_back(p);
}

void ModeManager::Release()
{
	for (size_t i = 0; i < m_Modes.size(); ++i)
		delete m_Modes[i];
	m_Modes.clear();
}