#include "EditView.h"
#include "Shortcuts.h"

#include <imgui.h>
#include <imgui_internal.h>


EditView::EditView(EditModel* model, EditorModel* editor)
	: m_EditorView(editor), m_VM(this, model, editor), m_TreeRoot({ .Path = m_VM.GetWorkDir(), .Directory = true })
{
}
EditView::~EditView()
{ }

void EditView::Render()
{
	ProcessShortcuts();
	RenderMainMenu();

	/* TAG: Toolbar
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	m_EditorSize = viewport->WorkSize, m_EditorPos = viewport->WorkPos;

	RenderTools();
	*/

	static bool initialized = false;
	if (!initialized)
	{
		initialized = true;

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
		OpenToolView(ToolView::TREE, ToolPosition::LEFT);
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
			if (ImGui::MenuItem("Compile Current"))
				m_VM.Compile(CompilationTarget::CURRENT);
			if (ImGui::MenuItem("Compile Open"))
				m_VM.Compile(CompilationTarget::OPEN);
			if (ImGui::MenuItem("Run", BFS_Emulate.Label, nullptr, m_CanEmulate))
				m_VM.StartEmulation();
			if (ImGui::MenuItem("Stop", BFS_StopEmulation.Label, nullptr, !m_CanEmulate))
				m_VM.StopEmulation();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Tree"))
				OpenToolView(ToolView::TREE, ToolPosition::LEFT);
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
	if (m_LeftSidebarView != ToolView::NONE)
	{
		std::string tool_name;
		switch (m_LeftSidebarView)
		{
		case ToolView::TREE: tool_name = "Tree"; break;
		default: break;
		}

		bool open = true;
		ImGui::SetNextWindowDockID(m_DockIDLeft, ImGuiCond_Appearing);
		bool visible = ImGui::Begin((tool_name + "##SidebarLeft").c_str(), &open);
		
		if (!open)
			m_LeftSidebarView = ToolView::NONE;
		else
		{
			switch (m_LeftSidebarView)
			{
			case ToolView::TREE:
				RenderTreeTool();
				break;
			default:
				break;
			}
		}
		ImGui::End();
	}
}


void EditView::RenderTreeEntry(TreeEntry& entry)
{
	static const ImGuiTreeNodeFlags dir_flags  = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	static const ImGuiTreeNodeFlags file_flags = dir_flags | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet;
	
	if (entry.Directory)
	{
		const bool node_open = ImGui::TreeNodeEx(entry.Path.string().c_str(), dir_flags, entry.Path.filename().string().c_str());
		
		if (entry.Collapsed && node_open)
			m_TreeCacheCounter = 144;
		entry.Collapsed = !node_open;
				
		if (ImGui::BeginPopupContextItem()) // right click
		{
			if (ImGui::MenuItem("Show in File Explorer"))
				ShowInExplorer(entry.Path);
			ImGui::EndPopup();
		}

		if (node_open)
		{
			for (TreeEntry& child : entry.Children)
				RenderTreeEntry(child);
			ImGui::TreePop();
		}
	}
	else
	{
		ImGui::TreeNodeEx(entry.Path.string().c_str(), file_flags, entry.Path.filename().string().c_str());
		
		if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			m_VM.OpenFile(entry.Path);
		}

		if (ImGui::BeginPopupContextItem()) // right click
		{
			if (ImGui::MenuItem("Show in File Explorer"))
				ShowInExplorer(entry.Path);
			ImGui::EndPopup();
		}
	}
}
void EditView::RenderTreeTool()
{
	RenderTreeEntry(m_TreeRoot);
	
	m_TreeCacheCounter++;
	if (m_TreeCacheCounter < 144)
		return;
	m_TreeCacheCounter = 0;

	CacheDirectoryTree(m_TreeRoot);
}
void EditView::CacheDirectoryTree(TreeEntry& parent)
{
	if (parent.Collapsed)
		return;

	std::vector<TreeEntry> newchildren;
	for (const fs::path& p : fs::directory_iterator(parent.Path, fs::directory_options::skip_permission_denied))
	{
		TreeEntry entry;
		if (parent.Map.contains(p))
		{
			TreeEntry& old = parent.Children[parent.Map[p]];
			if (old.Directory == fs::is_directory(p))
				entry = old;
			else
				entry = { .Path = p, .Directory = !old.Directory };
		}
		else
			entry = { .Path = p, .Directory = fs::is_directory(p) };
		
		if (entry.Directory && !entry.Collapsed)
			CacheDirectoryTree(entry);
		newchildren.push_back(entry);
	}
	parent.Children = std::move(newchildren);
	parent.Map.clear();
	for (size_t i = 0; i < parent.Children.size(); i++)
		parent.Map.insert({ parent.Children[i].Path, i });
}


void EditView::OpenToolView(ToolView view, ToolPosition pos)
{
	// TODO: unused pos
	m_LeftSidebarView = view;
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
