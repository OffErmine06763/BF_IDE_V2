#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

struct Shortcut
{
	const int Chord;
	const char* Label;
};
// GLOBAL
const Shortcut GS_CloseApp = { ImGuiMod_Ctrl | ImGuiMod_Alt | ImGuiKey_W, "Ctrl+Alt+W" };
// EDITOR
const Shortcut 
	ES_CloseAll = { ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_W, "Ctrl+Shift+W" }, 
	ES_CloseOne = { ImGuiMod_Ctrl | ImGuiKey_W, "Ctrl+W" },
	ES_Rename	= { ImGuiMod_Ctrl | ImGuiKey_R, "Ctrl+R" },
	ES_Save		= { ImGuiMod_Ctrl | ImGuiKey_S, "Ctrl+S" };