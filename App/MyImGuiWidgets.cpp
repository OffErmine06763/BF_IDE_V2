#include "MyImGuiWidgets.h"


static void BoxSelectDeactivateDrag(ImGuiBoxSelectState* bs)
{
    ImGuiContext& g = *GImGui;
    bs->IsActive = bs->IsStarting = false;
    if (g.ActiveId == bs->ID)
    {
        IMGUI_DEBUG_LOG_SELECTION("[selection] BeginBoxSelect() 0X%08X: Deactivate\n", bs->ID);
        ImGui::ClearActiveID();
    }
    bs->ID = 0;
}

static void DebugLogMultiSelectRequests(const char* function, const ImGuiMultiSelectIO* io)
{
    ImGuiContext& g = *GImGui;
    IM_UNUSED(function);
    for (const ImGuiSelectionRequest& req : io->Requests)
    {
        if (req.Type == ImGuiSelectionRequestType_SetAll)    IMGUI_DEBUG_LOG_SELECTION("[selection] %s: Request: SetAll %d (= %s)\n", function, req.Selected, req.Selected ? "SelectAll" : "Clear");
        if (req.Type == ImGuiSelectionRequestType_SetRange)  IMGUI_DEBUG_LOG_SELECTION("[selection] %s: Request: SetRange %" IM_PRId64 "..%" IM_PRId64 " (0x%" IM_PRIX64 "..0x%" IM_PRIX64 ") = %d (dir %d)\n", function, req.RangeFirstItem, req.RangeLastItem, req.RangeFirstItem, req.RangeLastItem, req.Selected, req.RangeDirection);
    }
}

static void BoxSelectPreStartDrag(ImGuiID id, ImGuiSelectionUserData clicked_item)
{
    ImGuiContext& g = *GImGui;
    ImGuiBoxSelectState* bs = &g.BoxSelectState;
    bs->ID = id;
    bs->IsStarting = true; // Consider starting box-select.
    bs->IsStartedFromVoid = (clicked_item == ImGuiSelectionUserData_Invalid);
    bs->IsStartedSetNavIdOnce = bs->IsStartedFromVoid;
    bs->KeyMods = g.IO.KeyMods;
    bs->StartPosRel = bs->EndPosRel = ImGui::WindowPosAbsToRel(g.CurrentWindow, g.IO.MousePos);
    bs->ScrollAccum = ImVec2(0.0f, 0.0f);
}

static ImRect CalcScopeRect(ImGuiMultiSelectTempData* ms, ImGuiWindow* window)
{
    ImGuiContext& g = *GImGui;
    if (ms->Flags & ImGuiMultiSelectFlags_ScopeRect)
    {
        // Warning: this depends on CursorMaxPos so it means to be called by EndMultiSelect() only
        return ImRect(ms->ScopeRectMin, ImMax(window->DC.CursorMaxPos, ms->ScopeRectMin));
    }
    else
    {
        // When a table, pull HostClipRect, which allows us to predict ClipRect before first row/layout is performed. (#7970)
        ImRect scope_rect = window->InnerClipRect;
        if (g.CurrentTable != NULL)
            scope_rect = g.CurrentTable->HostClipRect;

        // Add inner table decoration (#7821) // FIXME: Why not baking in InnerClipRect?
        scope_rect.Min = ImMin(scope_rect.Min + ImVec2(window->DecoInnerSizeX1, window->DecoInnerSizeY1), scope_rect.Max);
        return scope_rect;
    }
}

namespace ImGui
{
    ImGuiMultiSelectIO* MyBeginMultiSelect(ImGuiMultiSelectFlags flags, int selection_size, int items_count)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;

        if (++g.MultiSelectTempDataStacked > g.MultiSelectTempData.Size)
            g.MultiSelectTempData.resize(g.MultiSelectTempDataStacked, ImGuiMultiSelectTempData());
        ImGuiMultiSelectTempData* ms = &g.MultiSelectTempData[g.MultiSelectTempDataStacked - 1];
        IM_STATIC_ASSERT(offsetof(ImGuiMultiSelectTempData, IO) == 0); // Clear() relies on that.
        g.CurrentMultiSelect = ms;
        if ((flags & (ImGuiMultiSelectFlags_ScopeWindow | ImGuiMultiSelectFlags_ScopeRect)) == 0)
            flags |= ImGuiMultiSelectFlags_ScopeWindow;
        if (flags & ImGuiMultiSelectFlags_SingleSelect)
            flags &= ~(ImGuiMultiSelectFlags_BoxSelect2d | ImGuiMultiSelectFlags_BoxSelect1d);
        if (flags & ImGuiMultiSelectFlags_BoxSelect2d)
            flags &= ~ImGuiMultiSelectFlags_BoxSelect1d;

        // FIXME: Workaround to the fact we override CursorMaxPos, meaning size measurement are lost. (#8250)
        // They should perhaps be stacked properly?
        if (ImGuiTable* table = g.CurrentTable)
            if (table->CurrentColumn != -1)
                TableEndCell(table); // This is currently safe to call multiple time. If that properly is lost we can extract the "save measurement" part of it.

        // FIXME: BeginFocusScope()
        const ImGuiID id = window->IDStack.back();
        ms->Clear();
        ms->FocusScopeId = id;
        ms->Flags = flags;
        ms->IsFocused = (ms->FocusScopeId == g.NavFocusScopeId);
        ms->BackupCursorMaxPos = window->DC.CursorMaxPos;
        ms->ScopeRectMin = window->DC.CursorMaxPos = window->DC.CursorPos;
        PushFocusScope(ms->FocusScopeId);
        if (flags & ImGuiMultiSelectFlags_ScopeWindow) // Mark parent child window as navigable into, with highlight. Assume user will always submit interactive items.
            window->DC.NavLayersActiveMask |= 1 << ImGuiNavLayer_Main;

        // Use copy of keyboard mods at the time of the request, otherwise we would requires mods to be held for an extra frame.
        ms->KeyMods = g.NavJustMovedToId ? (g.NavJustMovedToIsTabbing ? 0 : g.NavJustMovedToKeyMods) : g.IO.KeyMods;
        if (flags & ImGuiMultiSelectFlags_NoRangeSelect)
            ms->KeyMods &= ~ImGuiMod_Shift;

        // Bind storage
        ImGuiMultiSelectState* storage = g.MultiSelectStorage.GetOrAddByKey(id);
        storage->ID = id;
        storage->LastFrameActive = g.FrameCount;
        storage->LastSelectionSize = selection_size;
        storage->Window = window;
        ms->Storage = storage;

        // Output to user
        ms->IO.Requests.resize(0);
        ms->IO.RangeSrcItem = storage->RangeSrcItem;
        ms->IO.NavIdItem = storage->NavIdItem;
        ms->IO.NavIdSelected = (storage->NavIdSelected == 1) ? true : false;
        ms->IO.ItemsCount = items_count;

        // Clear when using Navigation to move within the scope
        // (we compare FocusScopeId so it possible to use multiple selections inside a same window)
        bool request_clear = false;
        bool request_select_all = false;
        if (g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == ms->FocusScopeId && g.NavJustMovedToHasSelectionData)
        {
            if (ms->KeyMods & ImGuiMod_Shift)
                ms->IsKeyboardSetRange = true;
            if (ms->IsKeyboardSetRange)
                IM_ASSERT(storage->RangeSrcItem != ImGuiSelectionUserData_Invalid); // Not ready -> could clear?
            if ((ms->KeyMods & (ImGuiMod_Ctrl | ImGuiMod_Shift)) == 0 && (flags & (ImGuiMultiSelectFlags_NoAutoClear | ImGuiMultiSelectFlags_NoAutoSelect)) == 0)
                request_clear = true;
        }
        else if (g.NavJustMovedFromFocusScopeId == ms->FocusScopeId)
        {
            // Also clear on leaving scope (may be optional?)
            if ((ms->KeyMods & (ImGuiMod_Ctrl | ImGuiMod_Shift)) == 0 && (flags & (ImGuiMultiSelectFlags_NoAutoClear | ImGuiMultiSelectFlags_NoAutoSelect)) == 0)
                request_clear = true;
        }

        // Box-select handling: update active state.
        ImGuiBoxSelectState* bs = &g.BoxSelectState;
        if (flags & (ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_BoxSelect2d))
        {
            ms->BoxSelectId = GetID("##BoxSelect");
            if (BeginBoxSelect(CalcScopeRect(ms, window), window, ms->BoxSelectId, flags))
                request_clear |= bs->RequestClear;
        }

        if (ms->IsFocused)
        {
            // Shortcut: Clear selection (Escape)
            // - Only claim shortcut if selection is not empty, allowing further presses on Escape to e.g. leave current child window.
            // - Box select also handle Escape and needs to pass an id to bypass ActiveIdUsingAllKeyboardKeys lock.
            if (flags & ImGuiMultiSelectFlags_ClearOnEscape)
            {
                if (selection_size != 0 || bs->IsActive)
                    if (Shortcut(ImGuiKey_Escape, ImGuiInputFlags_None, bs->IsActive ? bs->ID : 0))
                    {
                        request_clear = true;
                        if (bs->IsActive)
                            BoxSelectDeactivateDrag(bs);
                    }
            }

            // Shortcut: Select all (CTRL+A)
            if (!(flags & ImGuiMultiSelectFlags_SingleSelect) && !(flags & ImGuiMultiSelectFlags_NoSelectAll))
                if (Shortcut(ImGuiMod_Ctrl | ImGuiKey_A))
                    request_select_all = true;
        }

        if (request_clear || request_select_all)
        {
            MultiSelectAddSetAll(ms, request_select_all);
            if (!request_select_all)
                storage->LastSelectionSize = 0;
        }
        ms->LoopRequestSetAll = request_select_all ? 1 : request_clear ? 0 : -1;
        ms->LastSubmittedItem = ImGuiSelectionUserData_Invalid;

        if (g.DebugLogFlags & ImGuiDebugLogFlags_EventSelection)
            DebugLogMultiSelectRequests("BeginMultiSelect", &ms->IO);

        return &ms->IO;
    }


    ImGuiMultiSelectIO* MyEndMultiSelect()
    {
        ImGuiContext& g = *GImGui;
        ImGuiMultiSelectTempData* ms = g.CurrentMultiSelect;
        ImGuiMultiSelectState* storage = ms->Storage;
        ImGuiWindow* window = g.CurrentWindow;
        IM_ASSERT_USER_ERROR(ms->FocusScopeId == g.CurrentFocusScopeId, "EndMultiSelect() FocusScope mismatch!");
        IM_ASSERT(g.CurrentMultiSelect != NULL && storage->Window == g.CurrentWindow);
        IM_ASSERT(g.MultiSelectTempDataStacked > 0 && &g.MultiSelectTempData[g.MultiSelectTempDataStacked - 1] == g.CurrentMultiSelect);

        ImRect scope_rect = CalcScopeRect(ms, window);
        if (ms->IsFocused)
        {
            // We currently don't allow user code to modify RangeSrcItem by writing to BeginIO's version, but that would be an easy change here.
            if (ms->IO.RangeSrcReset || (ms->RangeSrcPassedBy == false && ms->IO.RangeSrcItem != ImGuiSelectionUserData_Invalid)) // Can't read storage->RangeSrcItem here -> we want the state at begining of the scope (see tests for easy failure)
            {
                IMGUI_DEBUG_LOG_SELECTION("[selection] EndMultiSelect: Reset RangeSrcItem.\n"); // Will set be to NavId.
                storage->RangeSrcItem = ImGuiSelectionUserData_Invalid;
            }
            if (ms->NavIdPassedBy == false && storage->NavIdItem != ImGuiSelectionUserData_Invalid)
            {
                IMGUI_DEBUG_LOG_SELECTION("[selection] EndMultiSelect: Reset NavIdItem.\n");
                storage->NavIdItem = ImGuiSelectionUserData_Invalid;
                storage->NavIdSelected = -1;
            }

            if ((ms->Flags & (ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_BoxSelect2d)) && GetBoxSelectState(ms->BoxSelectId))
                EndBoxSelect(scope_rect, ms->Flags);
        }

        if (ms->IsEndIO == false)
            ms->IO.Requests.resize(0);

        // Clear selection when clicking void?
        // We specifically test for IsMouseDragPastThreshold(0) == false to allow box-selection!
        // The InnerRect test is necessary for non-child/decorated windows.
        bool scope_hovered = IsWindowHovered() && window->InnerRect.Contains(g.IO.MousePos);
        if (scope_hovered && (ms->Flags & ImGuiMultiSelectFlags_ScopeRect))
            scope_hovered &= scope_rect.Contains(g.IO.MousePos);
        if (scope_hovered && g.HoveredId == 0 && g.ActiveId == 0)
        {
            if (ms->Flags & (ImGuiMultiSelectFlags_BoxSelect1d | ImGuiMultiSelectFlags_BoxSelect2d))
            {
                if (!g.BoxSelectState.IsActive && !g.BoxSelectState.IsStarting && g.IO.MouseClickedCount[0] == 1)
                {
                    BoxSelectPreStartDrag(ms->BoxSelectId, ImGuiSelectionUserData_Invalid);
                    FocusWindow(window, ImGuiFocusRequestFlags_UnlessBelowModal);
                    SetHoveredID(ms->BoxSelectId);
                    if (ms->Flags & ImGuiMultiSelectFlags_ScopeRect)
                        SetNavID(0, ImGuiNavLayer_Main, ms->FocusScopeId, ImRect(g.IO.MousePos, g.IO.MousePos)); // Automatically switch FocusScope for initial click from void to box-select.
                }
            }

            if (ms->Flags & ImGuiMultiSelectFlags_ClearOnClickVoid)
            {
                if (IsMouseReleased(0) && IsMouseDragPastThreshold(0) == false && g.IO.KeyMods == ImGuiMod_None)
                {
                    MultiSelectAddSetAll(ms, false);
                    ms->Storage->RangeSrcItem = ms->Storage->NavIdItem;
                }
            }
        }

        // Courtesy nav wrapping helper flag
        if (ms->Flags & ImGuiMultiSelectFlags_NavWrapX)
        {
            IM_ASSERT(ms->Flags & ImGuiMultiSelectFlags_ScopeWindow); // Only supported at window scope
            ImGui::NavMoveRequestTryWrapping(ImGui::GetCurrentWindow(), ImGuiNavMoveFlags_WrapX);
        }

        // Unwind
        window->DC.CursorMaxPos = ImMax(ms->BackupCursorMaxPos, window->DC.CursorMaxPos);
        PopFocusScope();

        if (g.DebugLogFlags & ImGuiDebugLogFlags_EventSelection)
            DebugLogMultiSelectRequests("EndMultiSelect", &ms->IO);

        ms->FocusScopeId = 0;
        ms->Flags = ImGuiMultiSelectFlags_None;
        g.CurrentMultiSelect = (--g.MultiSelectTempDataStacked > 0) ? &g.MultiSelectTempData[g.MultiSelectTempDataStacked - 1] : NULL;

        return &ms->IO;
    }
}