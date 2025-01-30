#include "SelectProjectState.h"
#include "OpenProjectState.h"
#include "App.h"

#include "MyImGuiWidgets.h"

#include <format>
#include <iostream>

namespace fs = std::filesystem;

#define to static_cast

// Extra functions to add deletion support to ImGuiSelectionBasicStorage
struct ExampleSelectionWithDeletion : ImGuiSelectionBasicStorage
{
	// Find which item should be Focused after deletion.
	// Call _before_ item submission. Return an index in the before-deletion item list, your item loop should call SetKeyboardFocusHere() on it.
	// The subsequent ApplyDeletionPostLoop() code will use it to apply Selection.
	// - We cannot provide this logic in core Dear ImGui because we don't have access to selection data.
	// - We don't actually manipulate the ImVector<> here, only in ApplyDeletionPostLoop(), but using similar API for consistency and flexibility.
	// - Important: Deletion only works if the underlying ImGuiID for your items are stable: aka not depend on their index, but on e.g. item id/ptr.
	// FIXME-MULTISELECT: Doesn't take account of the possibility focus target will be moved during deletion. Need refocus or scroll offset.
	int ApplyDeletionPreLoop(ImGuiMultiSelectIO* ms_io, int items_count)
	{
		if (Size == 0)
			return -1;

		// If focused item is not selected...
		const int focused_idx = (int)ms_io->NavIdItem;  // Index of currently focused item
		if (ms_io->NavIdSelected == false)  // This is merely a shortcut, == Contains(adapter->IndexToStorage(items, focused_idx))
		{
			ms_io->RangeSrcReset = true;    // Request to recover RangeSrc from NavId next frame. Would be ok to reset even when NavIdSelected==true, but it would take an extra frame to recover RangeSrc when deleting a selected item.
			return focused_idx;             // Request to focus same item after deletion.
		}

		// If focused item is selected: land on first unselected item after focused item.
		for (int idx = focused_idx + 1; idx < items_count; idx++)
			if (!Contains(GetStorageIdFromIndex(idx)))
				return idx;

		// If focused item is selected: otherwise return last unselected item before focused item.
		for (int idx = std::min(focused_idx, items_count) - 1; idx >= 0; idx--)
			if (!Contains(GetStorageIdFromIndex(idx)))
				return idx;

		return -1;
	}

	// Rewrite item list (delete items) + update selection.
	// - Call after EndMultiSelect()
	// - We cannot provide this logic in core Dear ImGui because we don't have access to your items, nor to selection data.
	void ApplyDeletionPostLoop(ImGuiMultiSelectIO* ms_io, std::vector<SelectProjectState::Entry>& items, int item_curr_idx_to_select)
	{
		// Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
		// If NavId was not part of selection, we will stay on same item.
		std::vector<SelectProjectState::Entry> new_items;
		new_items.reserve(items.size() - Size);
		int item_next_idx_to_select = -1;
		for (int idx = 0; idx < items.size(); idx++)
		{
			if (!Contains(GetStorageIdFromIndex(idx))) // if selection ! contains ID
				new_items.push_back(items[idx]);
			else
				App::GetHistory().Remove(items[idx].Path);
			if (item_curr_idx_to_select == idx)
				item_next_idx_to_select = to<int>(new_items.size() - 1);
		}
		items.swap(new_items);

		// Update selection
		Clear();
		if (item_next_idx_to_select != -1 && ms_io->NavIdSelected)
			SetItemSelected(GetStorageIdFromIndex(item_next_idx_to_select), true);
	}

	int ApplyMovePreLoop(ImGuiMultiSelectIO* ms_io, int items_count)
	{
		if (Size == 0)
			return -1;

		// If focused item is not selected...
		const int focused_idx = (int)ms_io->NavIdItem;  // Index of currently focused item
		if (ms_io->NavIdSelected == false)  // This is merely a shortcut, == Contains(adapter->IndexToStorage(items, focused_idx))
		{
			ms_io->RangeSrcReset = true;    // Request to recover RangeSrc from NavId next frame. Would be ok to reset even when NavIdSelected==true, but it would take an extra frame to recover RangeSrc when deleting a selected item.
			return focused_idx;             // Request to focus same item after deletion.
		}

		// If focused item is selected: land on first unselected item after focused item.
		for (int idx = focused_idx + 1; idx < items_count; idx++)
			if (!Contains(GetStorageIdFromIndex(idx)))
				return idx;

		// If focused item is selected: otherwise return last unselected item before focused item.
		for (int idx = std::min(focused_idx, items_count) - 1; idx >= 0; idx--)
			if (!Contains(GetStorageIdFromIndex(idx)))
				return idx;

		return -1;
	}
	void ApplyMovePostLoop(ImGuiMultiSelectIO* ms_io, std::vector<SelectProjectState::Entry>& src, std::vector<SelectProjectState::Entry>& dst, int item_curr_idx_to_select, bool data)
	{
		// Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
		// If NavId was not part of selection, we will stay on same item.
		std::vector<SelectProjectState::Entry> new_items;
		new_items.reserve(src.size() - Size);
		int item_next_idx_to_select = -1;
		for (int idx = 0; idx < src.size(); idx++)
		{
			if (!Contains(GetStorageIdFromIndex(idx))) // if selection ! contains ID
				new_items.push_back(src[idx]);
			else
			{
				dst.push_back(src[idx]);
				App::GetHistory().SetFavourite(src[idx].Path, data);
			}
			if (item_curr_idx_to_select == idx)
				item_next_idx_to_select = to<int>(new_items.size() - 1);
		}
		src.swap(new_items);

		// Update selection
		Clear();
		if (item_next_idx_to_select != -1 && ms_io->NavIdSelected)
			SetItemSelected(GetStorageIdFromIndex(item_next_idx_to_select), true);
	}
};


SelectProjectState:: SelectProjectState() 
{ 
	std::cout << "Select Created\n"; 
	CacheHistory();
};
SelectProjectState::~SelectProjectState() { std::cout << "Select Destroyed\n"; };

void SelectProjectState::Render()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);

	if (ImGui::Begin("Documents", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		if (ImGui::Button("Open/New Project"))
			App::RequestNewState<OpenProjectState>();

		RenderFav();
		RenderRec();
	}
	ImGui::End();
}

void SelectProjectState::RenderFav()
{
	ImGui::Text("Favourites");

	// Options
	static bool show_color_button = true;
	static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid | ImGuiMultiSelectFlags_SelectOnClickRelease;

	// Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
	static ExampleSelectionWithDeletion selection;
	static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu
	static bool request_move_from_menu = false;

	ImGui::Text("Selection size: %d/%d", selection.Size, Fav.size());

	const float items_height = ImGui::GetTextLineHeightWithSpacing();
	ImGui::SetNextWindowContentSize(ImVec2(0.0f, Fav.size() * items_height));
	if (ImGui::BeginChild("##FavBasket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
	{
		ImVec2 color_button_sz(ImGui::GetFontSize(), ImGui::GetFontSize());

		ImGuiMultiSelectIO* ms_io = ImGui::MyBeginMultiSelect(flags, selection.Size, to<int>(Fav.size()));
		selection.ApplyRequests(ms_io);

		const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
		request_deletion_from_menu = false;
		const bool want_move = request_move_from_menu;
		request_move_from_menu = false;

		const int item_curr_idx_to_focus = want_delete 
			? selection.ApplyDeletionPreLoop(ms_io, to<int>(Fav.size())) 
			: want_move 
			? selection.ApplyMovePreLoop(ms_io, to<int>(Fav.size())) 
			: -1;

		ImGuiListClipper clipper;
		clipper.Begin(to<int>(Fav.size()));
		if (item_curr_idx_to_focus != -1)
			clipper.IncludeItemByIndex(item_curr_idx_to_focus); // Ensure focused item is not clipped.
		if (ms_io->RangeSrcItem != -1)
			clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.

		while (clipper.Step())
		{
			const int item_begin = clipper.DisplayStart;
			const int item_end = clipper.DisplayEnd;
			for (int i = item_begin; i < item_end; i++)
			{
				int n = Fav.size() - 1 - i;
				std::string label = Fav[n].Path.string();

				// IMPORTANT: for deletion refocus to work we need object ID to be stable,
				// aka not depend on their index in the list. Here we use our persistent item_id.
				// (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
				ImGui::PushID(Fav[n].ID);

				// TODO: Icon
				ImU32 dummy_col = (ImU32)((unsigned int)Fav[n].Type * 0xC250B74B) | IM_COL32_A_MASK;
				ImGui::ColorButton("##", ImColor(dummy_col), ImGuiColorEditFlags_NoTooltip, color_button_sz);
				ImGui::SameLine();

				// Submit item
				bool item_is_selected = selection.Contains((ImGuiID)n);
				bool item_is_open = false;
				ImGui::SetNextItemSelectionUserData(n);
				ImGui::Selectable(label.c_str(), item_is_selected, ImGuiSelectableFlags_None);
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					std::cout << "AAA\n";
				}
				ImGui::Spacing();

				// Focus (for after deletion)
				if (item_curr_idx_to_focus == n)
					ImGui::SetKeyboardFocusHere(-1);

				// Drag and Drop
				if (ImGui::BeginDragDropSource())
				{
					// Create payload with full selection OR single unselected item.
					// (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
					if (ImGui::GetDragDropPayload() == NULL)
					{
						ImVector<int> payload_items;
						void* it = NULL;
						ImGuiID id = 0;
						if (!item_is_selected)
							payload_items.push_back(n);
						else
							while (selection.GetNextSelectedItem(&it, &id))
								payload_items.push_back((int)id);
						ImGui::SetDragDropPayload("FAV_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
					}

					// Display payload content in tooltip
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					const int* payload_items = (int*)payload->Data;
					const int payload_count = (int)payload->DataSize / (int)sizeof(int);
					if (payload_count == 1)
						ImGui::Text("Object %05d: %s", payload_items[0], Fav[payload_items[0]].Path.string()); // TODO: the payload is the ID of the entry, not the index
					else
						ImGui::Text("Dragging %d objects", payload_count);

					ImGui::EndDragDropSource();
				}

				// Right-click: context menu
				if (ImGui::BeginPopupContextItem())
				{
					ImGui::BeginDisabled(selection.Size == 0);
					label = std::format("Delete {} item(s)###DeleteSelected", selection.Size);
					if (ImGui::Selectable(label.c_str()))
						request_deletion_from_menu = true;
					if (ImGui::Selectable("Remove from Favourites"))
						request_move_from_menu = true;
					ImGui::EndDisabled();
					ImGui::Selectable("Close");
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}
		}

		// Apply multi-select requests
		ms_io = ImGui::MyEndMultiSelect();
		selection.ApplyRequests(ms_io);
		if (want_delete)
			selection.ApplyDeletionPostLoop(ms_io, Fav, item_curr_idx_to_focus);
		else if (want_move)
			selection.ApplyMovePostLoop(ms_io, Fav, Recent, item_curr_idx_to_focus, false);
	}
	ImGui::EndChild();
}

void SelectProjectState::RenderRec()
{
	ImGui::Text("Recent");

	// Options
	static bool show_color_button = true;
	static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_ClearOnClickVoid | ImGuiMultiSelectFlags_SelectOnClickRelease;

	// Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
	static ExampleSelectionWithDeletion selection;
	static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu
	static bool request_move_from_menu = false;

	ImGui::Text("Selection size: %d/%d", selection.Size, Recent.size());

	const float items_height = ImGui::GetTextLineHeightWithSpacing();
	ImGui::SetNextWindowContentSize(ImVec2(0.0f, Recent.size() * items_height));
	if (ImGui::BeginChild("##RecBasket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
	{
		ImVec2 color_button_sz(ImGui::GetFontSize(), ImGui::GetFontSize());

		ImGuiMultiSelectIO* ms_io = ImGui::MyBeginMultiSelect(flags, selection.Size, to<int>(Recent.size()));
		selection.ApplyRequests(ms_io);

		const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
		request_deletion_from_menu = false;
		const bool want_move = request_move_from_menu;
		request_move_from_menu = false;

		const int item_curr_idx_to_focus = want_delete
			? selection.ApplyDeletionPreLoop(ms_io, to<int>(Fav.size()))
			: want_move
			? selection.ApplyMovePreLoop(ms_io, to<int>(Fav.size()))
			: -1;

		ImGuiListClipper clipper;
		clipper.Begin(to<int>(Recent.size()));
		if (item_curr_idx_to_focus != -1)
			clipper.IncludeItemByIndex(item_curr_idx_to_focus); // Ensure focused item is not clipped.
		if (ms_io->RangeSrcItem != -1)
			clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.

		while (clipper.Step())
		{
			const int item_begin = clipper.DisplayStart;
			const int item_end = clipper.DisplayEnd;
			for (int i = item_begin; i < item_end; i++)
			{
				int n = Recent.size() - 1 - i;
				std::string label = Recent[n].Path.string();

				// IMPORTANT: for deletion refocus to work we need object ID to be stable,
				// aka not depend on their index in the list. Here we use our persistent item_id.
				// (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
				ImGui::PushID(Recent[n].ID);

				// TODO: Icon
				ImU32 dummy_col = (ImU32)((unsigned int)Recent[n].Type * 0xC250B74B) | IM_COL32_A_MASK;
				ImGui::ColorButton("##", ImColor(dummy_col), ImGuiColorEditFlags_NoTooltip, color_button_sz);
				ImGui::SameLine();

				// Submit item
				bool item_is_selected = selection.Contains((ImGuiID)n);
				bool item_is_open = false;
				ImGui::SetNextItemSelectionUserData(n);
				ImGui::Selectable(label.c_str(), item_is_selected, ImGuiSelectableFlags_None);
				if (ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					std::cout << "AAA\n";
				}
				ImGui::Spacing();

				// Focus (for after deletion)
				if (item_curr_idx_to_focus == n)
					ImGui::SetKeyboardFocusHere(-1);

				// Drag and Drop
				if (ImGui::BeginDragDropSource())
				{
					// Create payload with full selection OR single unselected item.
					// (the later is only possible when using ImGuiMultiSelectFlags_SelectOnClickRelease)
					if (ImGui::GetDragDropPayload() == NULL)
					{
						ImVector<int> payload_items;
						void* it = NULL;
						ImGuiID id = 0;
						if (!item_is_selected)
							payload_items.push_back(n);
						else
							while (selection.GetNextSelectedItem(&it, &id))
								payload_items.push_back((int)id);
						ImGui::SetDragDropPayload("RECENT_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
					}

					// Display payload content in tooltip
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					const int* payload_items = (int*)payload->Data;
					const int payload_count = (int)payload->DataSize / (int)sizeof(int);
					if (payload_count == 1)
						ImGui::Text("Object %05d: %s", payload_items[0], Recent[payload_items[0]].Path.string()); // TODO: the payload is the ID of the entry, not the index
					else
						ImGui::Text("Dragging %d objects", payload_count);

					ImGui::EndDragDropSource();
				}

				// Right-click: context menu
				if (ImGui::BeginPopupContextItem())
				{
					ImGui::BeginDisabled(selection.Size == 0);
					label = std::format("Delete {} item(s)###DeleteSelected", selection.Size);
					if (ImGui::Selectable(label.c_str()))
						request_deletion_from_menu = true;
					if (ImGui::Selectable("Set as Favourite"))
						request_move_from_menu = true;
					ImGui::EndDisabled();
					ImGui::Selectable("Close");
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}
		}

		// Apply multi-select requests
		ms_io = ImGui::MyEndMultiSelect();
		selection.ApplyRequests(ms_io);
		if (want_delete)
			selection.ApplyDeletionPostLoop(ms_io, Recent, item_curr_idx_to_focus);
		else if (want_move)
			selection.ApplyMovePostLoop(ms_io, Recent, Fav, item_curr_idx_to_focus, true);
	}
	ImGui::EndChild();
}


void SelectProjectState::CacheHistory()
{
	const History& h = App::GetHistory();
	for (const History::Entry& e : h)
		e.Fav ? Fav.push_back({ CurrId++, GetType(e.Path), e.Path }) : Recent.push_back({ CurrId++, GetType(e.Path), e.Path });
}