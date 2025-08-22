#include "EditView.h"
#include "Shortcuts.h"
#include "Tools/TreeTool.h"
#include "Tools/MemoryTool.h"
#include "Tools/EmulationIOTool.h"
#include "Tools/EmulationImageTool.h"

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
	if (ImGui::IsKeyChordPressed(AS_ToolEmuIO.Chord))
		ToggleEmuIOTool();
	if (ImGui::IsKeyChordPressed(AS_ToolEmuImg.Chord))
		ToggleEmuImgTool();
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
			if (ImGui::MenuItem("Emulation IO", AS_ToolEmuIO.Label))
				ToggleEmuIOTool();
			if (ImGui::MenuItem("Emulation Img", AS_ToolEmuImg.Label))
				ToggleEmuImgTool();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}
void EditView::RenderEditor()
{
	m_EditorView.Render(m_DockIDCenter);
}
void EditView::RenderSidebars()
{
	for (auto it = m_Tools.begin(); it != m_Tools.end(); true)
	{
		bool open = true;
		if (it->Position == ToolPosition::LEFT)
		{
			ImGui::SetNextWindowDockID(m_DockIDLeft, ImGuiCond_Appearing);
			bool visible = ImGui::Begin((it->ToolPtr->Name() + "##SidebarLeft").c_str(), &open);
		}
		else if (it->Position == ToolPosition::RIGHT)
		{
			ImGui::SetNextWindowDockID(m_DockIDRight, ImGuiCond_Appearing);
			bool visible = ImGui::Begin((it->ToolPtr->Name() + "##SidebarRight").c_str(), &open);
		}
		else
		{
			ImGui::SetNextWindowDockID(m_DockIDBottom, ImGuiCond_Appearing);
			bool visible = ImGui::Begin((it->ToolPtr->Name() + "##SidebarBottom").c_str(), &open);
		}

		if (open)
		{
			it->ToolPtr->Render();
			it++;
		}
		else it = m_Tools.erase(it);

		ImGui::End();
	}
}


void EditView::EmulationStarted()
{
	m_CanEmulate = false;
}
void EditView::EmulationStopped()
{
	m_CanEmulate = true;
}



bool EditView::CloseTool(Tool::Type type)
{
	for (auto it = m_Tools.begin(); it != m_Tools.end(); it++)
	{
		if (it->ToolPtr->GetType() == type)
		{
			if (it->OnDestroy)
				it->OnDestroy();
			m_Tools.erase(it);
			return true;
		}
	}
	return false;
}
EditView::ToolIterator EditView::GetTool(Tool::Type type)
{
	for (auto it = m_Tools.begin(); it != m_Tools.end(); it++)
		if (it->ToolPtr->GetType() == type)
			return it;
	return m_Tools.end();
}
bool EditView::IsToolOpen(Tool::Type type)
{
	for (auto it = m_Tools.begin(); it != m_Tools.end(); it++)
		if (it->ToolPtr->GetType() == type)
			return true;
	return false;
}

void EditView::OpenToolView(Tool* tool, ToolPosition pos)
{
	m_Tools.push_back({ std::unique_ptr<Tool>(tool), pos });
}

void EditView::ToggleTreeView()
{
	if (CloseTool(TreeTool::_GetType()))
		return;
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
	if (CloseTool(MemoryTool::_GetType()))
		return;
	else
	{
		MemoryTool* tool = new MemoryTool();
		tool->SetMemory(&m_VM.GetEmulationMemory(), m_VM.GetEmulationAddress());
		OpenToolView(tool, ToolPosition::BOTTOM);
	}
}
void EditView::ToggleEmuIOTool()
{
	if (CloseTool(EmulationIOTool::_GetType()))
		return;
	else _OpenEmuIOTool();
}
void EditView::ToggleEmuImgTool()
{
	if (CloseTool(EmulationImageTool::_GetType()))
		return;
	else
	{
		EmulationImageTool* tool = new EmulationImageTool();
		tool->SetMemory(&m_VM.GetEmulationMemory());
		tool->SubscribeEmulationInput([this](bf_mem_t in) { m_VM.EmulationInput(in); });
		tool->SubscribeToggleRendering([this](bool rendering)
			{
				m_AllowStdEmuInput = !rendering;
				auto io = GetTool(EmulationIOTool::_GetType());
				((EmulationIOTool*)(io->ToolPtr.get()))->AllowInput(m_AllowStdEmuInput);
			});
		m_AllowStdEmuInput = !tool->IsRendering();

		listener_id id1 = m_VM.SubEmuOutput([this, tool](bf_mem_t out) { tool->OnOutput(out); });
		listener_id id2 = m_VM.SubEmuWantInput([this, tool]() { tool->EmulationWantsInput(); });
		listener_id id3 = m_VM.SubEmuInput([this, tool](bf_mem_t in) { tool->EmulationInput(in); });
		
		OpenToolView(tool, ToolPosition::RIGHT);
		
		m_Tools.rbegin()->OnDestroy = [this, id1, id2, id3]()
			{
				m_VM.UnsubEmuOutput(id1);
				m_VM.UnsubEmuWantInput(id2);
				m_VM.UnsubEmuInput(id3);
				m_AllowStdEmuInput = true;
			};
	}
}

void EditView::OpenEmuIOTool(bool open)
{
	if (open && IsToolOpen(EmulationIOTool::_GetType()))
		return;
	if (!open && !IsToolOpen(EmulationIOTool::_GetType()))
		return;

	_OpenEmuIOTool();
}
void EditView::_OpenEmuIOTool()
{
	EmulationIOTool* tool = new EmulationIOTool();

	// Query the emulation output before setting the listeners.
	// They are called at the end of every frame, on the main thread,
	// which means that any output generated in the meantime will be notified.
	tool->SetOutput(m_VM.GetEmulationOutput());
	tool->SubscribeInput([this, tool](bf_mem_t in)
		{
			if (m_AllowStdEmuInput) m_VM.EmulationInput(in);
		});
	tool->AllowInput(m_AllowStdEmuInput);

	listener_id id1 = m_VM.SubEmuOutput([this, tool](bf_mem_t out) { tool->OnEmulationOutput(out); });
	listener_id id2 = m_VM.SubEmuWantInput([this, tool]() { tool->OnEmulationInputRequested(); });
	listener_id id3 = m_VM.SubEmuTerminated([this, tool]() { tool->OnEmulationTerminated(); });
	listener_id id4 = m_VM.SubEmuStarted([this, tool]() { tool->OnEmulationStarted(); });

	OpenToolView(tool, ToolPosition::BOTTOM);

	m_Tools.rbegin()->OnDestroy = [this, id1, id2, id3, id4]()
		{
			m_VM.UnsubEmuOutput(id1);
			m_VM.UnsubEmuWantInput(id2);
			m_VM.UnsubEmuTerminated(id3);
			m_VM.UnsubEmuStarted(id4);
		};
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