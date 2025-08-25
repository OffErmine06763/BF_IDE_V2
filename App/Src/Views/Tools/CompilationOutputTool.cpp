#include "CompilationOutputTool.h"

#include <imgui.h>

void CompilationOutputTool::Render()
{
	if (m_Output)
		ImGui::TextUnformatted(m_Output->str().c_str());
}