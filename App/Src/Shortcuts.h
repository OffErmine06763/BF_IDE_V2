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
	ES_Save		= { ImGuiMod_Ctrl | ImGuiKey_S, "Ctrl+S" },
	ES_Explorer = { ImGuiMod_Ctrl | ImGuiKey_O, "Ctrl+O" };
// A perche' non mi viene in mente un bel nome
// TODO: change naming convention for shortcuts
const Shortcut
	AS_ToolTree   = { ImGuiMod_Ctrl | ImGuiKey_1, "Ctrl+1" },
	AS_ToolMemory = { ImGuiMod_Ctrl | ImGuiKey_2, "Ctrl+2" },
	AS_ToolEmuIO  = { ImGuiMod_Ctrl | ImGuiKey_3, "Ctrl+3" };
// BF
const Shortcut 
	BFS_Emulate       = { ImGuiMod_Ctrl | ImGuiKey_E, "Ctrl+E" },
	BFS_Compile       = { ImGuiMod_Ctrl | ImGuiKey_B, "Ctrl+B" },
	BFS_StopEmulation = { ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_E, "Ctrl+Shift+E" };
