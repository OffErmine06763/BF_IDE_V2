#include "OpenProjectState.h"
#include "App.h"
#include "WorkingState.h"

#include <imgui.h>

#include <format>
#include <ranges>
#include <fstream>
#include <iostream>

// TODO: WINDOWS ONLY
#define NOMINMAX
#include <windows.h>
#include <shlobj.h>


static constexpr auto DI_FLAG = fs::directory_options::skip_permission_denied;

static inline bool IsBF(const fs::path& path) { return path.extension().string() == ".bf"; }
static inline bool IsProj(const fs::path& path) { return path.extension().string() == ".bfproj"; }
static inline bool IsProjFile(const fs::path& p) { return !fs::is_directory(p) && p.extension().string() == ".bfproj"; }


OpenProjectState::OpenProjectState() { dbg << "Open Created\n"; }
OpenProjectState::~OpenProjectState() { dbg << "Open Destroyed\n"; }

void OpenProjectState::Render()
{
	static const fs::path root = fs::current_path().root_path();
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	ImGui::Begin("FileSystem", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

	RenderOptions();
	ImGui::BeginChild(ImGui::GetID("FileSystem Tree"));
	RenderDir(root);
	ImGui::EndChild();

	ImGui::End();
	
	if (m_WantDelete)
	{
		DeletePath(m_Selected);
		m_WantDelete = false;
	}

	RenderForms();
}


void OpenProjectState::DeletePath(const fs::path& path)
{ 
	std::wstring widePath = path.wstring();

	wchar_t filePath[MAX_PATH];
	wcscpy_s(filePath, widePath.c_str());
	filePath[widePath.size() + 1] = L'\0';  // Ensure double-null termination

	SHFILEOPSTRUCT fileOp = { 0 };
	fileOp.wFunc = FO_DELETE;  // Delete operation
	fileOp.pFrom = filePath;
	fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

	if (SHFileOperation(&fileOp) != 0) 
		m_FailedDelete = true;
}


void OpenProjectState::RenderDir(const fs::path& dir)
{
	for (const fs::path& path : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied))
	{
		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		const bool is_selected = m_Selected.compare(path) == 0;
		if (is_selected)
			node_flags |= ImGuiTreeNodeFlags_Selected;
		
		if (fs::is_directory(path))
		{
			const bool node_open = ImGui::TreeNodeEx(path.string().c_str(), node_flags, path.filename().string().c_str());
			if ((ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen())
				m_Selected = path;
			if (ImGui::BeginPopupContextItem()) // right click
			{
				if (ImGui::Selectable("Create File"))   m_CreatingFile = true;
				if (ImGui::Selectable("Create Folder")) m_CreatingFolder = true;
				if (ImGui::Selectable("Create Proj"))	m_CreatingProj = true;
				if (ImGui::Selectable("Delete Folder")) m_WantDelete = true;
				if (ImGui::Selectable("Open Folder"))   App::RequestOpenPath(m_Selected);
				auto count = stdr::count_if(fs::directory_iterator(m_Selected, DI_FLAG), IsProjFile);
				if (count == 1 && ImGui::Selectable("Open Project"))
					App::RequestOpenPath(*stdr::find_if(fs::directory_iterator(m_Selected, DI_FLAG), IsProjFile));

				ImGui::EndPopup();
			}
			if (node_open)
			{
				RenderDir(path);
				ImGui::TreePop();
			}
		}
		else
		{
			node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen; // ImGuiTreeNodeFlags_Bullet
			ImGui::TreeNodeEx(path.string().c_str(), node_flags, path.filename().string().c_str());
			if ((ImGui::IsItemClicked() || ImGui::IsItemClicked(ImGuiMouseButton_Right)) && !ImGui::IsItemToggledOpen())
				m_Selected = path;
			if (ImGui::BeginPopupContextItem()) // right click
			{
				if (ImGui::Selectable("Delete File"))		m_WantDelete = true;
				if (IsBF(path))
					if (ImGui::Selectable("Open File"))		App::RequestOpenPath(m_Selected);
				else if (IsProj(path))
					if (ImGui::Selectable("Open Project"))	App::RequestOpenPath(m_Selected);

				ImGui::EndPopup();
			}
		}
	}
}
void OpenProjectState::RenderOptions()
{
	if (m_Selected.empty())
		ImGui::BeginDisabled();


	// Generic buttons
	if (ImGui::Button("Create Folder"))  m_CreatingFolder = true;
	ImGui::SameLine();
	if (ImGui::Button("Create File"))	 m_CreatingFile = true;
	ImGui::SameLine();
	if (ImGui::Button("Create Project")) m_CreatingProj = true;
	
	if (m_Selected.empty())
	{
		ImGui::Button("Open");
		ImGui::EndDisabled();
		return;
	}

	// Buttons related to current selection
	if (fs::is_directory(m_Selected))
	{
		if (ImGui::Button("Open Folder"))
			App::RequestOpenPath(m_Selected);

		auto count = stdr::count_if(fs::directory_iterator(m_Selected, DI_FLAG), IsProjFile);
		if (count == 1)
		{
			ImGui::SameLine();
			if (ImGui::Button("Open Project"))
			{
				App::RequestOpenPath(*stdr::find_if(fs::directory_iterator(m_Selected, DI_FLAG), IsProjFile));
			}
		}
	}
	else if (IsBF(m_Selected))
	{
		if (ImGui::Button("Open File"))
			App::RequestOpenPath(m_Selected);
	}
	else if (IsProj(m_Selected))
	{
		if (ImGui::Button("Open Project"))
			App::RequestOpenPath(m_Selected);
	}
	else
	{
		ImGui::BeginDisabled();
		ImGui::Button("Open");
		ImGui::EndDisabled();
	}
}
void OpenProjectState::RenderForms()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	if (m_CreatingFile)
	{
		const ImVec2 size = { viewport->WorkSize.x * 0.5f, viewport->WorkSize.y * 0.5f };
		const ImVec2 center = { viewport->WorkPos.x + viewport->WorkSize.x / 2, viewport->WorkPos.y + viewport->WorkSize.y / 2 };
		const ImVec2 pos = { center.x - size.x / 2, center.y - size.y / 2 };
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		ImGui::Begin("Fill me Senpai", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetKeyboardFocusHere();

		static char data[256] = "";
		if (ImGui::InputText("Name", data, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			fs::path newpath = fs::is_directory(m_Selected) ? m_Selected / data : m_Selected.parent_path() / data;
			std::ofstream(newpath).close();
			m_Selected = newpath;

			memset(data, '\0', 256);
			m_CreatingFile = false;
		}

		ImGui::End();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mouse = ImGui::GetMousePos();
			if (mouse.x < pos.x || mouse.y < pos.y || mouse.x > pos.x + size.x || mouse.y > pos.y + size.y)
			{
				memset(data, '\0', 256);
				m_CreatingFile = false;
			}
		}
	}
	else if (m_CreatingFolder)
	{
		const ImVec2 size = { viewport->WorkSize.x * 0.5f, viewport->WorkSize.y * 0.5f };
		const ImVec2 center = { viewport->WorkPos.x + viewport->WorkSize.x / 2, viewport->WorkPos.y + viewport->WorkSize.y / 2 };
		const ImVec2 pos = { center.x - size.x / 2, center.y - size.y / 2 };
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		ImGui::Begin("Fill me Senpai", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetKeyboardFocusHere();

		static char data[256] = "";
		if (ImGui::InputText("Name", data, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			fs::path newpath = fs::is_directory(m_Selected) ? m_Selected / data : m_Selected.parent_path() / data;
			if (fs::create_directories(newpath))
				m_Selected = newpath;

			memset(data, '\0', 256);
			m_CreatingFolder = false;
		}

		ImGui::End();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mouse = ImGui::GetMousePos();
			if (mouse.x < pos.x || mouse.y < pos.y || mouse.x > pos.x + size.x || mouse.y > pos.y + size.y)
			{
				memset(data, '\0', 256);
				m_CreatingFolder = false;
			}
		}
	}
	else if (m_CreatingProj)
	{
		const ImVec2 size = { viewport->WorkSize.x * 0.5f, viewport->WorkSize.y * 0.5f };
		const ImVec2 center = { viewport->WorkPos.x + viewport->WorkSize.x / 2, viewport->WorkPos.y + viewport->WorkSize.y / 2 };
		const ImVec2 pos = { center.x - size.x / 2, center.y - size.y / 2 };
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		ImGui::Begin("Fill me Senpai", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetKeyboardFocusHere();

		static char data[256] = "";
		if (ImGui::InputText("Name", data, 256, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			fs::path newpath = fs::is_directory(m_Selected) ? m_Selected / data : m_Selected.parent_path() / data;
			if (fs::create_directories(newpath))
			{
				m_Selected = newpath;
				std::ofstream(newpath / (""s + data + ".bfproj")).close();
				App::RequestOpenPath(m_Selected);
			}

			memset(data, '\0', 256);
			m_CreatingProj = false;
		}

		ImGui::End();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mouse = ImGui::GetMousePos();
			if (mouse.x < pos.x || mouse.y < pos.y || mouse.x > pos.x + size.x || mouse.y > pos.y + size.y)
			{
				memset(data, '\0', 256);
				m_CreatingProj = false;
			}
		}
	}
	else if (m_FailedDelete)
	{
		const ImVec2 size = { viewport->WorkSize.x * 0.5f, viewport->WorkSize.y * 0.5f };
		const ImVec2 center = { viewport->WorkPos.x + viewport->WorkSize.x / 2, viewport->WorkPos.y + viewport->WorkSize.y / 2 };
		const ImVec2 pos = { center.x - size.x / 2, center.y - size.y / 2 };
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		ImGui::Begin("Fill me Senpai", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

		ImGui::Text("Failed to delete element");

		ImGui::End();

		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			ImVec2 mouse = ImGui::GetMousePos();
			if (mouse.x < pos.x || mouse.y < pos.y || mouse.x > pos.x + size.x || mouse.y > pos.y + size.y)
				m_CreatingProj = false;
		}
	}
}



// static inline void StartCreateFolder()	{ ImGui::OpenPopup("createfolder_popup"); }

//void OpenProjectState::RenderCreateFolderForm(const fs::path& path)
//{
//	if (ImGui::BeginPopup("createfolder_popup"))
//	{
//		static char data[256] = "";
//		if (ImGui::InputText("Name", data, 256, ImGuiInputTextFlags_EnterReturnsTrue))
//		{
//			fs::path newpath = fs::is_directory(path) ? path / data : path.parent_path() / data;
//			if (fs::create_directories(newpath))
//				m_Selected = newpath;
//
//			memset(data, '\0', 256);
//			ImGui::CloseCurrentPopup();
//		}
//		ImGui::EndPopup();
//	}
//}