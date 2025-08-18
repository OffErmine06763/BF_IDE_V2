#include "EditView.h"
#include "Shortcuts.h"
#include "Tools/TreeTool.h"
#include "Tools/MemoryTool.h"

#include <imgui.h>
#include <imgui_internal.h>


// TODO: save view state when exit if the user opened a project, restore when opening the same project (store in History), but not for file/folder
// TODO: renaming a file open in the editor, from explorer, doesn't update the name in the view

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

	m_DockIDLeft   = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Left, 0.25f, nullptr, &m_DockIDCenter);
	m_DockIDRight  = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Right, 0.25f, nullptr, &m_DockIDCenter);
	m_DockIDBottom = ImGui::DockBuilderSplitNode(m_DockspaceID, ImGuiDir_Down, 0.25f, nullptr, &m_DockIDCenter);

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

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::DockSpaceOverViewport(m_DockspaceID, viewport, ImGuiDockNodeFlags_PassthruCentralNode);
	
	RenderEditor();
	RenderSidebars();
	RenderEmulation();

	if (!m_PathToDelete.empty())
		RenderDeleteConfirmationUI();
	else if (!m_PathToNew.empty())
		RenderNewFileUI();
}

void EditView::ProcessShortcuts()
{
	if (ImGui::IsKeyChordPressed(BFS_Emulate.Chord))
		m_VM.StartEmulation(CompilationTarget::FOLDER);
	else if (ImGui::IsKeyChordPressed(BFS_StopEmulation.Chord))
		m_VM.StopEmulation();
	if (ImGui::IsKeyChordPressed(BFS_Compile.Chord))
		m_VM.Compile(CompilationTarget::FOLDER);
	if (ImGui::IsKeyChordPressed(AS_ToolTree.Chord))
		ToggleTreeView();
	if (ImGui::IsKeyChordPressed(AS_ToolMemory.Chord))
		ToggleMemoryTool();
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
			if (ImGui::BeginMenu("Compile"))
			{
				if (ImGui::MenuItem("Compile Current"))
					m_VM.Compile(CompilationTarget::CURRENT);
				if (ImGui::MenuItem("Compile Open"))
					m_VM.Compile(CompilationTarget::OPEN);
				if (ImGui::MenuItem("Compile Folder", BFS_Compile.Label))
					m_VM.Compile(CompilationTarget::FOLDER);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Emulate"))
			{
				if (ImGui::MenuItem("Emulate Current", nullptr, nullptr, m_CanEmulate))
					m_VM.StartEmulation(CompilationTarget::CURRENT);
				if (ImGui::MenuItem("Emulate Open", nullptr, nullptr, m_CanEmulate))
					m_VM.StartEmulation(CompilationTarget::OPEN);
				if (ImGui::MenuItem("Emulate Folder", BFS_Emulate.Label, nullptr, m_CanEmulate))
					m_VM.StartEmulation(CompilationTarget::FOLDER);
				if (ImGui::MenuItem("Stop", BFS_StopEmulation.Label, nullptr, !m_CanEmulate))
					m_VM.StopEmulation();
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Tree", AS_ToolTree.Label))
				ToggleTreeView();
			if (ImGui::MenuItem("Memory", AS_ToolMemory.Label))
				ToggleMemoryTool();
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

		if (open)   m_LeftSidebarTool->Render();
		else delete m_LeftSidebarTool.release();

		ImGui::End();
	}
	if (m_RightSidebarTool)
	{
		bool open = true;
		ImGui::SetNextWindowDockID(m_DockIDRight, ImGuiCond_Appearing);
		bool visible = ImGui::Begin((m_RightSidebarTool->Name() + "##SidebarRight").c_str(), &open);

		if (open)   m_RightSidebarTool->Render();
		else delete m_RightSidebarTool.release();

		ImGui::End();
	}
	if (m_BottomSidebarTool)
	{
		bool open = true;
		ImGui::SetNextWindowDockID(m_DockIDBottom, ImGuiCond_Appearing);
		bool visible = ImGui::Begin((m_BottomSidebarTool->Name() + "##SidebarBottom").c_str(), &open);

		if (open)   m_BottomSidebarTool->Render();
		else delete m_BottomSidebarTool.release();

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
void EditView::EmulationOutputChanged(bf_mem_t o)
{
	std::lock_guard<std::mutex> lock(m_EmuMutex);
	m_EmuOutput.push_back(o);
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
	switch (pos)
	{
	case EditView::ToolPosition::LEFT:
		m_LeftSidebarTool.reset(tool);
		break;
	case EditView::ToolPosition::RIGHT:
		m_RightSidebarTool.reset(tool);
		break;
	case EditView::ToolPosition::BOTTOM:
		m_BottomSidebarTool.reset(tool);
		break;
	default:
		break;
	}
}
void EditView::ToggleTreeView()
{
	if (m_LeftSidebarTool && m_LeftSidebarTool->GetType() == TreeTool::_GetType())
		m_LeftSidebarTool.release();
	else
	{
		TreeTool* tool = new TreeTool(m_VM.GetWorkDir());
		tool->SubscribeSelect([this](const fs::path& path) { m_VM.OpenFile(path); });
		tool->SubscribeCompile([this](const fs::path& path) { m_VM.Compile({ path }); });
		tool->SubscribeDelete([this](const fs::path& path) { m_PathToDelete = path; });
		tool->SubscribeNew([this](const fs::path& path) { m_PathToNew = path; });
		OpenToolView(tool, ToolPosition::LEFT);
	}
}
void EditView::ToggleMemoryTool()
{
	if (m_BottomSidebarTool && m_BottomSidebarTool->GetType() == MemoryTool::_GetType())
		m_BottomSidebarTool.release();
	else
	{
		MemoryTool* tool = new MemoryTool();
		tool->SetMemory(&m_VM.GetEmulationMemory());
		OpenToolView(tool, ToolPosition::BOTTOM);
	}
}



void EditView::RenderDeleteConfirmationUI()
{
	if (!ImGui::IsPopupOpen("Sure?"))
		ImGui::OpenPopup("Sure?");
	if (ImGui::BeginPopupModal("Sure?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Are you sure to delete the selected element?");

		ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
		if (ImGui::Button("Yes", button_size))
		{
			m_VM.DeletePath(m_PathToDelete);
			m_PathToDelete.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No", button_size))
		{
			m_PathToDelete.clear();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void EditView::RenderNewFileUI()
{
	if (!ImGui::IsPopupOpen("New"))
		ImGui::OpenPopup("New");
	if (ImGui::BeginPopupModal("New", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		static char data[256] = "";
		if (ImGui::InputText("Name", data, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			fs::path newpath = fs::is_directory(m_PathToNew) ? m_PathToNew / data : m_PathToNew.parent_path() / data;
			std::ofstream(newpath).close();
			memset(data, '\0', 256);
			m_PathToNew.clear();
		}
		if (ImGui::Button("Close"))
		{
			memset(data, '\0', 256);
			m_PathToNew.clear();
			ImGui::CloseCurrentPopup();
		}
		
		ImGui::EndPopup();
	}
}