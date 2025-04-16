#include "FileState.h"
#include "Shortcuts.h"

#include <imgui.h>



FileState::FileState(const fs::path& workdir)
	: m_View(workdir)
{
	dbg << "FileState::FileState m_WorkDir = " << workdir << '\n';
}
FileState::~FileState()
{
	dbg << "FileState::~FileState\n";
}

void FileState::Render()
{
	m_View.Render();
}