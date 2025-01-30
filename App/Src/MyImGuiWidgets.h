#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui.h"
#include "imgui_internal.h"


static void BoxSelectDeactivateDrag(ImGuiBoxSelectState* bs);

static void DebugLogMultiSelectRequests(const char* function, const ImGuiMultiSelectIO* io);

static void BoxSelectPreStartDrag(ImGuiID id, ImGuiSelectionUserData clicked_item);

static ImRect CalcScopeRect(ImGuiMultiSelectTempData* ms, ImGuiWindow* window);

namespace ImGui
{
    ImGuiMultiSelectIO* MyBeginMultiSelect(ImGuiMultiSelectFlags flags, int selection_size, int items_count);


    ImGuiMultiSelectIO* MyEndMultiSelect();
}