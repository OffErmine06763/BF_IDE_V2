#include "EditView.h"
#include "Shortcuts.h"
#include "Tools/TreeTool.h"

#include <imgui.h>
#include <imgui_internal.h>


// TODO: save view state when exit if the user opened a project, restore when opening the same project (store in History), but not for file/folder
// TODO: ranaming a file open in the editor, from explorer, doesn't update the name in the view

EditView::EditView(EditModel* model, EditorModel* editor)
	: m_EditorView(editor), m_VM(this, model, editor)
{
	LOG_GRAPHICS("EditView Created\n");
}
EditView::~EditView()
{
	LOG_GRAPHICS("EditView Destroyed\n");
}

void EditView::Init()
{
	m_DockspaceID = ImGui::GetID("EditMainWindowDockspace");
	ImGui::DockBuilderRemoveNode(m_DockspaceID);
	ImGui::DockBuilderAddNode(m_DockspaceID, ImGuiDockNodeFlags_DockSpace);
	ImGui::DockBuilderSetNodeSize(m_DockspaceID, ImVec2(800, 600));

	m_DockIDLeft = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Left, 0.25f, nullptr, &m_DockIDCenter);

	// Dock "ChildWindow" into the left side
	//ImGui::DockBuilderDockWindow("SidebarLeft", m_DockIDLeft);

	// Optionally dock another window on the right
	//ImGui::DockBuilderDockWindow("EditorViewDocuments", m_DockIDCenter);

	ImGui::DockBuilderFinish(m_DockspaceID);
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

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(m_DockspaceID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
	
	RenderEditor();
	RenderSidebars();
	RenderEmulation();

	// if (!m_CanEmulate && m_Emulator != nullptr && m_Emulator->Done())
	// {
	// 	m_Emulator->join();
	// 	m_CanEmulate = true;
	// 	m_Emulator = nullptr;
	// 	m_Editor.Lock(false);
	// }
}

void EditView::ProcessShortcuts()
{
	if (ImGui::IsKeyChordPressed(BFS_Emulate.Chord))
		m_VM.StartEmulation();
	else if (ImGui::IsKeyChordPressed(BFS_StopEmulation.Chord))
		m_VM.StopEmulation();
	if (ImGui::IsKeyChordPressed(AS_ToolTree.Chord))
	{
		TreeTool* tool = new TreeTool(m_VM.GetWorkDir());
		tool->SubscribeSelect([this](const fs::path& path) { m_VM.OpenFile(path); });
		OpenToolView(tool, ToolPosition::LEFT);
	}
}
void EditView::RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("App"))
		{
			if (ImGui::MenuItem("Home"))
				m_VM.GoHome();
			if (ImGui::MenuItem("Close", GS_CloseApp.Label))
				m_VM.CloseApp();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Run"))
		{
			if (ImGui::MenuItem("Compile Current"))
				m_VM.Compile(CompilationTarget::CURRENT);
			if (ImGui::MenuItem("Compile Open"))
				m_VM.Compile(CompilationTarget::OPEN);
			if (ImGui::MenuItem("Compile Folder"))
				m_VM.Compile(CompilationTarget::FOLDER);
			if (ImGui::MenuItem("Run", BFS_Emulate.Label, nullptr, m_CanEmulate))
				m_VM.StartEmulation();
			if (ImGui::MenuItem("Stop", BFS_StopEmulation.Label, nullptr, !m_CanEmulate))
				m_VM.StopEmulation();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Tree", AS_ToolTree.Label))
			{
				TreeTool* tool = new TreeTool(m_VM.GetWorkDir());
				tool->SubscribeSelect([this](const fs::path& path) { m_VM.OpenFile(path); });
				OpenToolView(tool, ToolPosition::LEFT);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void EditView::RenderEmulation() {
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
		{
			// NOTE: must guarantee m_EmuWantsInput happens before any EmulationWantsInput(true)
			m_EmuWantsInput = false;
			m_VM.EmulationInput(m_EmuInput);
		}
	}

	// release the lock after sending the input to the VM as another requested input will be set after rendering/sending.
	m_EmuMutex.unlock();

	ImGui::End();
}
void EditView::RenderEditor()
{
	m_EditorView.Render(m_DockIDCenter);
}
void EditView::RenderSidebars()
{
	if (m_LeftSidebarTool)
	{
		bool open = true;
		ImGui::SetNextWindowDockID(m_DockIDLeft, ImGuiCond_Appearing);
		bool visible = ImGui::Begin((m_LeftSidebarTool->Name() + "##SidebarLeft").c_str(), &open);

		if (!open)
			delete m_LeftSidebarTool.release();
		else
			m_LeftSidebarTool->Render();

		ImGui::End();
	}
}



void EditView::OpenEmulationTab(bool open)
{
	m_EmuTabOpen = open;
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
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuWantsInput = wants;
	if (wants)
	{
		m_EmuInput = 0;
		m_EmuFocusInput = true;
	}
}




void EditView::OpenToolView(Tool* tool, ToolPosition pos)
{
	m_LeftSidebarTool.reset(tool);
}
