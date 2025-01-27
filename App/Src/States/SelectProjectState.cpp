#include "SelectProjectState.h"
#include "App.h"
#include "OpenProjectState.h"

#include <imgui.h>

#include <format>
#include <filesystem>
#include <iostream>


// Extra functions to add deletion support to ImGuiSelectionBasicStorage
struct ExampleSelectionWithDeletion : ImGuiSelectionBasicStorage
{
	// Find which item should be Focused after deletion.
	// Call _before_ item submission. Retunr an index in the before-deletion item list, your item loop should call SetKeyboardFocusHere() on it.
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
	template<typename ITEM_TYPE>
	void ApplyDeletionPostLoop(ImGuiMultiSelectIO* ms_io, ImVector<ITEM_TYPE>& items, int item_curr_idx_to_select)
	{
		// Rewrite item list (delete items) + convert old selection index (before deletion) to new selection index (after selection).
		// If NavId was not part of selection, we will stay on same item.
		ImVector<ITEM_TYPE> new_items;
		new_items.reserve(items.Size - Size);
		int item_next_idx_to_select = -1;
		for (int idx = 0; idx < items.Size; idx++)
		{
			if (!Contains(GetStorageIdFromIndex(idx)))
				new_items.push_back(items[idx]);
			if (item_curr_idx_to_select == idx)
				item_next_idx_to_select = new_items.Size - 1;
		}
		items.swap(new_items);

		// Update selection
		Clear();
		if (item_next_idx_to_select != -1 && ms_io->NavIdSelected)
			SetItemSelected(GetStorageIdFromIndex(item_next_idx_to_select), true);
	}
};

static const char* ExampleNames[] =
{
	"Artichoke", "Arugula", "Asparagus", "Avocado", "Bamboo Shoots", "Bean Sprouts", "Beans", "Beet", "Belgian Endive", "Bell Pepper",
	"Bitter Gourd", "Bok Choy", "Broccoli", "Brussels Sprouts", "Burdock Root", "Cabbage", "Calabash", "Capers", "Carrot", "Cassava",
	"Cauliflower", "Celery", "Celery Root", "Celcuce", "Chayote", "Chinese Broccoli", "Corn", "Cucumber"
};


SelectProjectState:: SelectProjectState() { std::cout << "Select Created\n"; };
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
	static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnClickVoid;

	// Initialize default list with 1000 items.
	// Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
	static ImVector<int> items;
	static int items_next_id = 0;
	if (items_next_id == 0) { for (int n = 0; n < 1000; n++) { items.push_back(items_next_id++); } }
	static ExampleSelectionWithDeletion selection;
	static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu

	ImGui::Text("Selection size: %d/%d", selection.Size, items.Size);

	const float items_height = ImGui::GetTextLineHeightWithSpacing();
	ImGui::SetNextWindowContentSize(ImVec2(0.0f, items.Size * items_height));
	if (ImGui::BeginChild("##Basket", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
	{
		ImVec2 color_button_sz(ImGui::GetFontSize(), ImGui::GetFontSize());

		ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, items.Size);
		selection.ApplyRequests(ms_io);

		const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
		const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;
		request_deletion_from_menu = false;

		ImGuiListClipper clipper;
		clipper.Begin(items.Size);
		if (item_curr_idx_to_focus != -1)
			clipper.IncludeItemByIndex(item_curr_idx_to_focus); // Ensure focused item is not clipped.
		if (ms_io->RangeSrcItem != -1)
			clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.

		while (clipper.Step())
		{
			const int item_begin = clipper.DisplayStart;
			const int item_end = clipper.DisplayEnd;
			for (int n = item_begin; n < item_end; n++)
			{
				const int item_id = items[n];
				const char* item_category = ExampleNames[item_id % IM_ARRAYSIZE(ExampleNames)];
				std::string label = std::format("Object {:05}: {}", item_id, item_category);

				// IMPORTANT: for deletion refocus to work we need object ID to be stable,
				// aka not depend on their index in the list. Here we use our persistent item_id
				// instead of index to build a unique ID that will persist.
				// (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
				ImGui::PushID(item_id);

				// Emit a color button, to test that Shift+LeftArrow landing on an item that is not part
				// of the selection scope doesn't erroneously alter our selection.
				if (show_color_button)
				{
					ImU32 dummy_col = (ImU32)((unsigned int)n * 0xC250B74B) | IM_COL32_A_MASK;
					ImGui::ColorButton("##", ImColor(dummy_col), ImGuiColorEditFlags_NoTooltip, color_button_sz);
					ImGui::SameLine();
				}

				// Submit item
				bool item_is_selected = selection.Contains((ImGuiID)n);
				bool item_is_open = false;
				ImGui::SetNextItemSelectionUserData(n);
				ImGui::Selectable(label.c_str(), item_is_selected, ImGuiSelectableFlags_None);
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
							payload_items.push_back(item_id);
						else
							while (selection.GetNextSelectedItem(&it, &id))
								payload_items.push_back((int)id);
						ImGui::SetDragDropPayload("MULTISELECT_DEMO_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
					}

					// Display payload content in tooltip
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					const int* payload_items = (int*)payload->Data;
					const int payload_count = (int)payload->DataSize / (int)sizeof(int);
					if (payload_count == 1)
						ImGui::Text("Object %05d: %s", payload_items[0], ExampleNames[payload_items[0] % IM_ARRAYSIZE(ExampleNames)]);
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
						int a = 0; // TODO: 
					ImGui::EndDisabled();
					ImGui::Selectable("Close");
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}
		}

		// Apply multi-select requests
		ms_io = ImGui::EndMultiSelect();
		selection.ApplyRequests(ms_io);
		if (want_delete)
			selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);
	}
	ImGui::EndChild();
}

void SelectProjectState::RenderRec()
{
	ImGui::Text("Recent");

	// Options
	static bool show_color_button = true;
	static ImGuiMultiSelectFlags flags = ImGuiMultiSelectFlags_ClearOnEscape | ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_ClearOnClickVoid;

	// Initialize default list with 1000 items.
	// Use default selection.Adapter: Pass index to SetNextItemSelectionUserData(), store index in Selection
	static ImVector<int> items;
	static int items_next_id = 0;
	if (items_next_id == 0) { for (int n = 0; n < 1000; n++) { items.push_back(items_next_id++); } }
	static ExampleSelectionWithDeletion selection;
	static bool request_deletion_from_menu = false; // Queue deletion triggered from context menu

	ImGui::Text("Selection size: %d/%d", selection.Size, items.Size);

	const float items_height = ImGui::GetTextLineHeightWithSpacing();
	ImGui::SetNextWindowContentSize(ImVec2(0.0f, items.Size * items_height));
	if (ImGui::BeginChild("##Baske2", ImVec2(-FLT_MIN, ImGui::GetFontSize() * 20), ImGuiChildFlags_FrameStyle | ImGuiChildFlags_ResizeY))
	{
		ImVec2 color_button_sz(ImGui::GetFontSize(), ImGui::GetFontSize());

		ImGuiMultiSelectIO* ms_io = ImGui::BeginMultiSelect(flags, selection.Size, items.Size);
		selection.ApplyRequests(ms_io);

		const bool want_delete = (ImGui::Shortcut(ImGuiKey_Delete, ImGuiInputFlags_Repeat) && (selection.Size > 0)) || request_deletion_from_menu;
		const int item_curr_idx_to_focus = want_delete ? selection.ApplyDeletionPreLoop(ms_io, items.Size) : -1;
		request_deletion_from_menu = false;

		ImGuiListClipper clipper;
		clipper.Begin(items.Size);
		if (item_curr_idx_to_focus != -1)
			clipper.IncludeItemByIndex(item_curr_idx_to_focus); // Ensure focused item is not clipped.
		if (ms_io->RangeSrcItem != -1)
			clipper.IncludeItemByIndex((int)ms_io->RangeSrcItem); // Ensure RangeSrc item is not clipped.

		while (clipper.Step())
		{
			const int item_begin = clipper.DisplayStart;
			const int item_end = clipper.DisplayEnd;
			for (int n = item_begin; n < item_end; n++)
			{
				const int item_id = items[n];
				const char* item_category = ExampleNames[item_id % IM_ARRAYSIZE(ExampleNames)];
				std::string label = std::format("Object {:05}: {}", item_id, item_category);

				// IMPORTANT: for deletion refocus to work we need object ID to be stable,
				// aka not depend on their index in the list. Here we use our persistent item_id
				// instead of index to build a unique ID that will persist.
				// (If we used PushID(index) instead, focus wouldn't be restored correctly after deletion).
				ImGui::PushID(item_id);

				// Emit a color button, to test that Shift+LeftArrow landing on an item that is not part
				// of the selection scope doesn't erroneously alter our selection.
				if (show_color_button)
				{
					ImU32 dummy_col = (ImU32)((unsigned int)n * 0xC250B74B) | IM_COL32_A_MASK;
					ImGui::ColorButton("##", ImColor(dummy_col), ImGuiColorEditFlags_NoTooltip, color_button_sz);
					ImGui::SameLine();
				}

				// Submit item
				bool item_is_selected = selection.Contains((ImGuiID)n);
				bool item_is_open = false;
				ImGui::SetNextItemSelectionUserData(n);
				ImGui::Selectable(label.c_str(), item_is_selected, ImGuiSelectableFlags_None);
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
							payload_items.push_back(item_id);
						else
							while (selection.GetNextSelectedItem(&it, &id))
								payload_items.push_back((int)id);
						ImGui::SetDragDropPayload("MULTISELECT_DEMO_ITEMS", payload_items.Data, (size_t)payload_items.size_in_bytes());
					}

					// Display payload content in tooltip
					const ImGuiPayload* payload = ImGui::GetDragDropPayload();
					const int* payload_items = (int*)payload->Data;
					const int payload_count = (int)payload->DataSize / (int)sizeof(int);
					if (payload_count == 1)
						ImGui::Text("Object %05d: %s", payload_items[0], ExampleNames[payload_items[0] % IM_ARRAYSIZE(ExampleNames)]);
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
						int a = 0; // TODO: 
					ImGui::EndDisabled();
					ImGui::Selectable("Close");
					ImGui::EndPopup();
				}

				ImGui::PopID();
			}
		}

		// Apply multi-select requests
		ms_io = ImGui::EndMultiSelect();
		selection.ApplyRequests(ms_io);
		if (want_delete)
			selection.ApplyDeletionPostLoop(ms_io, items, item_curr_idx_to_focus);
	}
	ImGui::EndChild();
}