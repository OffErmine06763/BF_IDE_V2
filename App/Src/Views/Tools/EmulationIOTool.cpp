#include "EmulationIOTool.h"

#include <imgui.h>
#include <imgui_internal.h>


void EmulationIOTool::Reset()
{
	m_EmuInput = 0;
	m_EmuOutput.clear();
	m_EmuWantsInput = false;
	m_EmuFocusInput = false;
}

void EmulationIOTool::Render()
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	ImGui::TextUnformatted(m_EmuOutput.c_str());

	if (m_EmuWantsInput)
	{
		// TODO: weird behaviour when inputting chars instead of numbers
		ImGui::TextUnformatted(">>"); ImGui::SameLine();
		if (m_EmuFocusInput)
		{
			ImGui::SetKeyboardFocusHere();
			m_EmuFocusInput = false;
		}
		ImGui::InputScalar("##input", ImGuiDataType_U8, &m_EmuInput, nullptr, nullptr, nullptr);
		if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::Button("Confirm"))
		{
			// NOTE: must guarantee m_EmuWantsInput = false happens before any EmulationWantsInput(true)
			m_EmuWantsInput = false;
			m_InputEvent.Notify(m_EmuInput);
		}
	}
}


void EmulationIOTool::SetOutput(const std::string& out)
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuOutput = out;
}


void EmulationIOTool::OnEmulationOutput(bf_mem_t o)
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuOutput.push_back(o);
}
void EmulationIOTool::OnEmulationInputRequested()
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuWantsInput = true;
	m_EmuFocusInput = true;
	m_EmuInput = 0;
}
void EmulationIOTool::OnEmulationTerminated()
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuWantsInput = false;
	m_EmuFocusInput = false;
}
void EmulationIOTool::OnEmulationStarted()
{
	Reset();
}
