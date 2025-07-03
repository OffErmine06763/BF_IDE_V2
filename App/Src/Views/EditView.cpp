#include "EditView.h"
#include "Shortcuts.h"

#include <imgui.h>



EditView::EditView(EditModel* model, EditorModel* editor)
	: m_EditorView(editor), m_VM(this, model, editor)
{
	dbg << "EditView::EditView m_WorkDir = " << m_VM.GetWorkDir() << '\n';
}
EditView::~EditView()
{
	dbg << "EditView::~EditView m_WorkDir = " << m_VM.GetWorkDir() << '\n';
}

void EditView::Render()
{
	ProcessShortcuts();
	RenderMainMenu();

	/* TAG: Toolbar
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	m_EditorSize = viewport->WorkSize, m_EditorPos = viewport->WorkPos;

	RenderTools();
	*/

	RenderEditor();
	RenderEmulation();

	// if (!m_CanEmulate && m_Emulator != nullptr && m_Emulator->Done())
	// {
	// 	m_Emulator->join();
	// 	m_CanEmulate = true;
	// 	m_Emulator = nullptr;
	// 	m_Editor.Lock(false);
	// }
}
void EditView::OpenEmulationTab()
{
	m_EmuTabOpen = true;
}
void EditView::CloseEmulationTab()
{
	m_EmuTabOpen = false;
}
void EditView::EmulationStarted()
{
	m_CanEmulate = false;
	m_EmuInput = 0;
	m_EmuOutput.clear();
}
void EditView::EmulationStopped()
{
	m_CanEmulate = true;
	m_EmuWantsInput = false;
}
void EditView::EmulationOutputChanged()
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuOutput = m_VM.GetEmulationOutput();
}
void EditView::EmulationWantsInput(bool wants)
{
	m_EmuWantsInput = wants;
	if (wants)
	{
		m_EmuInput = 0;
		m_EmuFocusInput = true;
	}
}
void EditView::ProcessShortcuts()
{
	if (ImGui::IsKeyChordPressed(BFS_Emulate.Chord))
		m_VM.StartEmulation();
	else if (ImGui::IsKeyChordPressed(BFS_StopEmulation.Chord))
		m_VM.StopEmulation();
}
void EditView::RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::MenuItem("Close", GS_CloseApp.Label))
				m_VM.CloseApp();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Run"))
		{
			if (ImGui::MenuItem("Compile"))
				dbg << "Compiling: " << m_VM.GetWorkDir() << '\n';
			if (ImGui::MenuItem("Run", BFS_Emulate.Label, nullptr, m_CanEmulate))
				m_VM.StartEmulation();
			if (ImGui::MenuItem("Stop", BFS_StopEmulation.Label, nullptr, !m_CanEmulate))
				m_VM.StopEmulation();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void EditView::RenderEmulation() {
	// TAG: Toolbar 
	// WorkingState::RenderEmulation();
	if (!m_EmuTabOpen)
		return;

	// TODO: add button to reopen the tab
	static ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos({ viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y / 2 });
	ImGui::SetNextWindowSize({ viewport->WorkSize.x, viewport->WorkSize.y / 2 });

	bool wantclose = true;
	ImGui::Begin("Emulation", &wantclose, flags);
	if (!wantclose)
		m_VM.CloseEmulationTab();

	m_EmuMutex.lock();
	ImGui::Text(m_EmuOutput.c_str());
	m_EmuMutex.unlock();

	if (m_EmuWantsInput)
	{
		ImGui::Text(">>"); ImGui::SameLine();
		if (m_EmuFocusInput)
		{
			ImGui::SetKeyboardFocusHere();
			m_EmuFocusInput = false;
		}
		ImGui::InputScalar("##input", ImGuiDataType_U8, &m_EmuInput, nullptr, nullptr, nullptr);
		if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::Button("Confirm"))
			m_VM.EmulationInput(m_EmuInput);
	}

	ImGui::End();
}
void EditView::RenderEditor()
{
	m_EditorView.Render();
}