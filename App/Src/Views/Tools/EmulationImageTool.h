#pragma once
#include "Tool.h"
#include "Utility.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <backends/imgui_impl_dx12.h>

struct EmulationImageTool : public Tool
{
public:
	EmulationImageTool();
	~EmulationImageTool() override;

	void Render() override;
	inline std::string Name() const override { return "Image"; }
	inline Type GetType() const override { return _GetType(); }
	static inline Type _GetType() { return EMU_IMG; }

	void OnOutput();
	void SetMemory(const std::vector<bf_mem_t>* memory) { m_Memory = memory; }

private:
	//void CreateImage(const std::vector<u8>& rgbaPixels);
	/*void CreateImage2();
	void UpdateImage(const std::vector<u32>& rgbaPixels);

	void CreateImage3();
	void UpdateTexture2(const std::vector<u32>& rgbaPixels);
	void CleanupTexture();*/


private:
	const std::vector<bf_mem_t>* m_Memory = nullptr;
	std::array<RGBA, 256> m_Buffer;

	bool m_332 = false, m_Smooth = false;

	/*ImTextureID tex_id;
	ID3D12Resource* tex_resources = nullptr;
	D3D12_GPU_DESCRIPTOR_HANDLE my_texture_srv_gpu_handle = { 0 };
	D3D12_CPU_DESCRIPTOR_HANDLE my_texture_srv_cpu_handle = { 0 };*/
};