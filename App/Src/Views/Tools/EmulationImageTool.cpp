#include "EmulationImageTool.h"
#include "App.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx12.h>


static inline u32 RGB332toRGBA(u8 c)
{
	u8 r = (c >> 5) & 0x07;   // 3 bits
	u8 g = (c >> 2) & 0x07;   // 3 bits
	u8 b = (c >> 0) & 0x03;   // 2 bits

	// Expand to 8-bit range
	r = (r * 255) / 7;
	g = (g * 255) / 7;
	b = (b * 255) / 3;

	return (0xFF << 24) | (r << 16) | (g << 8) | b;  // ARGB or RGBA depending on DXGI_FORMAT
}


EmulationImageTool::EmulationImageTool()
{
	m_Buffer.fill({ 255, 255, 255, 255 });
}
EmulationImageTool::~EmulationImageTool()
{
}


void BoxBlur(std::array<RGBA, 256>& pixels, int width, int height, int kernelSize = 3) {
	std::array<RGBA, 256> result;
	int radius = kernelSize / 2;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
			int count = 0;

			for (int ky = -radius; ky <= radius; ky++) {
				for (int kx = -radius; kx <= radius; kx++) {
					int nx = x + kx;
					int ny = y + ky;

					if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
						int index = ny * width + nx;
						sumR += pixels[index].r;
						sumG += pixels[index].g;
						sumB += pixels[index].b;
						sumA += pixels[index].a;
						count++;
					}
				}
			}

			int index = y * width + x;
			result[index].r = sumR / count;
			result[index].g = sumG / count;
			result[index].b = sumB / count;
			result[index].a = sumA / count;
		}
	}
	pixels = result;
}
void GaussianBlur(std::array<RGBA, 256>& pixels, int width, int height, float sigma = 1.0f) {
	std::array<RGBA, 256> result;
	int kernelSize = 5;
	int radius = kernelSize / 2;

	// Create Gaussian kernel
	std::vector<std::vector<float>> kernel(kernelSize, std::vector<float>(kernelSize));
	float sum = 0.0f;

	for (int y = -radius; y <= radius; y++) {
		for (int x = -radius; x <= radius; x++) {
			float value = exp(-(x * x + y * y) / (2 * sigma * sigma)) / (2 * std::numbers::pi * sigma * sigma);
			kernel[y + radius][x + radius] = value;
			sum += value;
		}
	}

	// Normalize kernel
	for (int y = 0; y < kernelSize; y++) {
		for (int x = 0; x < kernelSize; x++) {
			kernel[y][x] /= sum;
		}
	}

	// Apply kernel
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f, sumA = 0.0f;

			for (int ky = -radius; ky <= radius; ky++) {
				for (int kx = -radius; kx <= radius; kx++) {
					int nx = x + kx;
					int ny = y + ky;

					if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
						int index = ny * width + nx;
						float weight = kernel[ky + radius][kx + radius];
						sumR += pixels[index].r * weight;
						sumG += pixels[index].g * weight;
						sumB += pixels[index].b * weight;
						sumA += pixels[index].a * weight;
					}
				}
			}

			int index = y * width + x;
			result[index].r = to<u8>(std::clamp(sumR, 0.0f, 255.0f));
			result[index].g = to<u8>(std::clamp(sumG, 0.0f, 255.0f));
			result[index].b = to<u8>(std::clamp(sumB, 0.0f, 255.0f));
			result[index].a = to<u8>(std::clamp(sumA, 0.0f, 255.0f));
		}
	}

	pixels = result;
}


void EmulationImageTool::OnOutput(bf_mem_t out)
{
	if (!m_Memory) return;

	m_LastOutput = out;
	m_AdvanceFrame = true;

	if (!m_Rendering) return;
	
	for (u64 y = 0; y < 16; y++)
	{
		for (u64 x = 0; x < 16; x++)
		{
			u8 c = (*m_Memory)[x + y * 16];

			ImU32 color;
			if (m_332)
			{
				u8 r = (c >> 5) & 0x07;   // 3 bits
				u8 g = (c >> 2) & 0x07;   // 3 bits
				u8 b = (c >> 0) & 0x03;   // 2 bits

				// Expand to 8-bit range
				r = (r * 255) / 7;
				g = (g * 255) / 7;
				b = (b * 255) / 3;
				color = IM_COL32(r, g, b, 255);
			}
			else
				color = IM_COL32(c, c, c, 255);

			m_Buffer[x + y * 16] = color;
		}
	}

	if (m_Smoothing == BOX)
		BoxBlur(m_Buffer, 16, 16);
	else if (m_Smoothing == GAUSS)
		GaussianBlur(m_Buffer, 16, 16);
}
void EmulationImageTool::EmulationWantsInput()
{
	if (!m_Rendering) return;
	if (!m_AdvanceFrame) return;

	m_AdvanceFrame = false;
	bf_mem_t out = m_LastOutput;
	std::this_thread::sleep_for(5ms);
	// shouldn't raise events in event handlers -> schedule
	App::ScheduleTask([this, out]() { m_InputEvent.Notify(out); });
}
void EmulationImageTool::EmulationInput(bf_mem_t in)
{
	m_AdvanceFrame = false;
}


void EmulationImageTool::Render()
{
	if (ImGui::Checkbox("Render", &m_Rendering))
	{
		m_ToggleRenderingEvent.Notify(m_Rendering);
		if (m_AdvanceFrame && m_Rendering)
			m_InputEvent.Notify(m_LastOutput);
	}
	
	if (m_Rendering)
	{
		ImGui::Checkbox("RGB332", &m_332);
		ImGui::SameLine();
		if (ImGui::BeginCombo("Smooth", m_SmoothingLabels[m_Smoothing].c_str()))
		{
			for (Smoothing s = NONE; s < MAX; s = to<Smoothing>(s + 1))
			{
				const bool is_selected = (m_Smoothing == s);
				if (ImGui::Selectable(m_SmoothingLabels[s].c_str(), is_selected))
					m_Smoothing = s;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}


		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 p0 = ImGui::GetCursorScreenPos();
		static constexpr u64 size = 20;

		for (u64 y = 0; y < 16; y++)
		{
			for (u64 x = 0; x < 16; x++)
			{
				draw_list->AddRectFilled(ImVec2(p0.x, p0.y), ImVec2(p0.x + size, p0.y + size), m_Buffer[x + y * 16]);
				p0 = ImVec2(p0.x + size, p0.y);
			}
			p0 = ImVec2(p0.x - size * 16, p0.y + size);
		}
	}
}