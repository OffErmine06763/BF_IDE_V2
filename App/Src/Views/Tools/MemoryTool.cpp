#include "MemoryTool.h"

#include <imgui.h>
#include <imgui_internal.h>


static int InputHexFilter(ImGuiInputTextCallbackData* data)
{
	if (data->EventChar != 0)
	{
		if (std::isxdigit(data->EventChar) || data->EventChar == 'x' || data->EventChar == 'X')
			return 0;
		return 1;
	}
	return 0;
}


void MemoryTool::Render()
{
	if (!m_Memory)
	{
		ImGui::Text("Start emulation to inspect its memory");
		return;
	}

	// NOTE: now the memory size is static, if we allow customization, we must dynamically set address digits in the view, and change some u32 in u64
	static u32 find_address = -1;
	static bool find = false;
	
	{
		static char buf[7] = "0x";

		// NOTE: failed to prevent 0x deletion, and consequenly failed to prevent 'x' insertion anywhere but the second char
		ImGui::Text("Search");
		bool enter = ImGui::InputText("##SearchInput", buf, sizeof(buf), ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_EnterReturnsTrue, InputHexFilter);
		if (enter || ImGui::IsItemDeactivatedAfterEdit())
		{
			try {
				find_address = std::stoull(buf, nullptr, 16);
				find = true;
			}
			catch (...) {
				find_address = 0;
			}
		}
		ImGui::SameLine();
		ImGui::Text("Current Address %d", *m_Address);
	}


	static size_t bytes_per_row = 16 * 2;
	static size_t size = m_Memory->size();
	float line_height = ImGui::GetTextLineHeightWithSpacing();
	static ImColor grey = ImColor(150, 150, 150), selected = ImColor(255, 0, 0);
	static size_t rows = (size + bytes_per_row - 1) / bytes_per_row;


	// Print column offset
	ImGui::TextColored(grey, "Offset ");
	ImGui::SameLine();
	for (size_t n = 0; n < bytes_per_row; n++) {
		ImGui::TextColored(grey, "%02X ", (u32)n);
		ImGui::SameLine();
	}
	ImGui::NewLine();


	ImGui::BeginChild("MemoryTool_Memory");


	ImGuiListClipper clipper;
	clipper.Begin((int)rows, line_height);

	if (find)
	{
		find = false;
		int row = find_address / bytes_per_row;
		float y = row * line_height;
		ImGui::SetScrollY(y);
	}


	while (clipper.Step())
	{
		for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
		{
			size_t i = row * bytes_per_row;

			// Print row start address
			ImGui::TextColored(grey, "%04X:  ", (u32)i);
			ImGui::SameLine();

			// Print hex values
			for (size_t j = 0; j < bytes_per_row && (i + j) < size; j++)
			{
				if (i + j == find_address)
					ImGui::TextColored(selected, "%02X ", (*m_Memory)[i + j]);
				else
					ImGui::Text("%02X ", (*m_Memory)[i + j]);
				ImGui::SameLine();
			}
			ImGui::NewLine();
		}
	}

	clipper.End();
	ImGui::EndChild();
}
