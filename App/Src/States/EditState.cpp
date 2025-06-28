#include "EditState.h"
#include "App.h"

#include "MyImGuiWidgets.h"



EditState::EditState(const fs::path& workdir)
	: m_Workdir(workdir), m_Model(workdir, &m_Editor), m_View(&m_Model, &m_Editor)
{
}

void EditState::Render()
{
	m_View.Render();
}