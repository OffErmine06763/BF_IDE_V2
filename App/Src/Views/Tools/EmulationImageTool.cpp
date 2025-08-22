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

//static void CreateTexture(const u8* image_data, ID3D12Device* d3d_device, D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu_handle, ID3D12Resource** out_tex_resource)
//{
//	u64 image_width = 16, image_height = 16;
//
//	D3D12_HEAP_PROPERTIES props;
//	memset(&props, 0, sizeof(D3D12_HEAP_PROPERTIES));
//	props.Type = D3D12_HEAP_TYPE_DEFAULT;
//	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//
//	D3D12_RESOURCE_DESC desc;
//	ZeroMemory(&desc, sizeof(desc));
//	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	desc.Alignment = 0;
//	desc.Width = image_width;
//	desc.Height = image_height;
//	desc.DepthOrArraySize = 1;
//	desc.MipLevels = 1;
//	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	desc.SampleDesc.Count = 1;
//	desc.SampleDesc.Quality = 0;
//	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	ID3D12Resource* pTexture = NULL;
//	HRESULT hr = d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
//		D3D12_RESOURCE_STATE_COPY_DEST, NULL, IID_PPV_ARGS(&pTexture));
//	IM_ASSERT(SUCCEEDED(hr));
//
//	// Create a temporary upload resource to move the data in
//	UINT uploadPitch = (image_width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
//	UINT uploadSize = image_height * uploadPitch;
//	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	desc.Alignment = 0;
//	desc.Width = uploadSize;
//	desc.Height = 1;
//	desc.DepthOrArraySize = 1;
//	desc.MipLevels = 1;
//	desc.Format = DXGI_FORMAT_UNKNOWN;
//	desc.SampleDesc.Count = 1;
//	desc.SampleDesc.Quality = 0;
//	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	props.Type = D3D12_HEAP_TYPE_UPLOAD;
//	props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//
//	ID3D12Resource* uploadBuffer = NULL;
//	hr = d3d_device->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc,
//		D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS(&uploadBuffer));
//	IM_ASSERT(SUCCEEDED(hr));
//
//	// Write pixels into the upload resource
//	void* mapped = NULL;
//	D3D12_RANGE range = { 0, uploadSize };
//	hr = uploadBuffer->Map(0, &range, &mapped);
//	IM_ASSERT(SUCCEEDED(hr));
//	for (int y = 0; y < image_height; y++)
//		memcpy((void*)((uintptr_t)mapped + y * uploadPitch), image_data + y * image_width * 4, image_width * 4);
//	uploadBuffer->Unmap(0, &range);
//
//	// Copy the upload resource content into the real resource
//	D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
//	srcLocation.pResource = uploadBuffer;
//	srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//	srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	srcLocation.PlacedFootprint.Footprint.Width = image_width;
//	srcLocation.PlacedFootprint.Footprint.Height = image_height;
//	srcLocation.PlacedFootprint.Footprint.Depth = 1;
//	srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;
//
//	D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
//	dstLocation.pResource = pTexture;
//	dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
//	dstLocation.SubresourceIndex = 0;
//
//	D3D12_RESOURCE_BARRIER barrier = {};
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	barrier.Transition.pResource = pTexture;
//	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//
//	// Create a temporary command queue to do the copy with
//	ID3D12Fence* fence = NULL;
//	hr = d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
//	IM_ASSERT(SUCCEEDED(hr));
//
//	HANDLE event = CreateEvent(0, 0, 0, 0);
//	IM_ASSERT(event != NULL);
//
//	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
//	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//	queueDesc.NodeMask = 1;
//
//	ID3D12CommandQueue* cmdQueue = NULL;
//	hr = d3d_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&cmdQueue));
//	IM_ASSERT(SUCCEEDED(hr));
//
//	ID3D12CommandAllocator* cmdAlloc = NULL;
//	hr = d3d_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
//	IM_ASSERT(SUCCEEDED(hr));
//
//	ID3D12GraphicsCommandList* cmdList = NULL;
//	hr = d3d_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc, NULL, IID_PPV_ARGS(&cmdList));
//	IM_ASSERT(SUCCEEDED(hr));
//
//	cmdList->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
//	cmdList->ResourceBarrier(1, &barrier);
//
//	hr = cmdList->Close();
//	IM_ASSERT(SUCCEEDED(hr));
//
//	// Execute the copy
//	cmdQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
//	hr = cmdQueue->Signal(fence, 1);
//	IM_ASSERT(SUCCEEDED(hr));
//
//	// Wait for everything to complete
//	fence->SetEventOnCompletion(1, event);
//	WaitForSingleObject(event, INFINITE);
//
//	// Tear down our temporary command queue and release the upload resource
//	cmdList->Release();
//	cmdAlloc->Release();
//	cmdQueue->Release();
//	CloseHandle(event);
//	fence->Release();
//	uploadBuffer->Release();
//
//	// Create a shader resource view for the texture
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
//	ZeroMemory(&srvDesc, sizeof(srvDesc));
//	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	srvDesc.Texture2D.MipLevels = desc.MipLevels;
//	srvDesc.Texture2D.MostDetailedMip = 0;
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	d3d_device->CreateShaderResourceView(pTexture, &srvDesc, srv_cpu_handle);
//
//	// Return results
//	*out_tex_resource = pTexture;
//}
//static void DestroyTexture(ID3D12Resource** tex_resources)
//{
//	(*tex_resources)->Release();
//	*tex_resources = NULL;
//}
//
//void EmulationImageTool::CreateImage2()
//{
//	// Describe texture (empty for now)
//	D3D12_RESOURCE_DESC texDesc = {};
//	texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	texDesc.Alignment = 0;
//	texDesc.Width = 16;
//	texDesc.Height = 16;
//	texDesc.DepthOrArraySize = 1;
//	texDesc.MipLevels = 1;
//	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	texDesc.SampleDesc.Count = 1;
//	texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	// Default heap (GPU texture)
//	D3D12_HEAP_PROPERTIES defaultHeap = {};
//	defaultHeap.Type = D3D12_HEAP_TYPE_DEFAULT;
//	defaultHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	defaultHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//	defaultHeap.CreationNodeMask = 1;
//	defaultHeap.VisibleNodeMask = 1;
//
//	App::_DXData.Device->CreateCommittedResource(
//		&defaultHeap,
//		D3D12_HEAP_FLAG_NONE,
//		&texDesc,
//		D3D12_RESOURCE_STATE_COPY_DEST,
//		nullptr,
//		IID_PPV_ARGS(&tex_resources));
//
//	// Allocate SRV in ImGui’s heap
//	UINT descriptorSize = App::_DXData.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
//
//	App::_DXData.Allocator->Alloc(&my_texture_srv_cpu_handle, &my_texture_srv_gpu_handle);
//	//my_texture_srv_cpu_handle = App::_DXData.SrvDescHeap->GetCPUDescriptorHandleForHeapStart();
//	//my_texture_srv_gpu_handle = App::_DXData.SrvDescHeap->GetGPUDescriptorHandleForHeapStart();
//
//	// Slot 1 (slot 0 is usually ImGui font texture)
//	//my_texture_srv_cpu_handle.ptr += descriptorSize * 1;
//	//my_texture_srv_gpu_handle.ptr += descriptorSize * 1;
//
//	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
//	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
//	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
//	srvDesc.Texture2D.MipLevels = 1;
//
//	App::_DXData.Device->CreateShaderResourceView(tex_resources, &srvDesc, my_texture_srv_cpu_handle);
//}
//void EmulationImageTool::UpdateImage(const std::vector<u32>& rgbaPixels)
//{
//	// Step 2: Describe footprint for copy
//	D3D12_RESOURCE_DESC texDesc = tex_resources->GetDesc();
//	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
//	UINT numRows;
//	UINT64 rowSizeInBytes, totalBytes;
//	App::_DXData.Device->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &totalBytes);
//
//	// Step 3: Create temporary upload heap
//	ID3D12Resource* uploadResource = nullptr;
//
//	D3D12_HEAP_PROPERTIES uploadHeap = {};
//	uploadHeap.Type = D3D12_HEAP_TYPE_UPLOAD;
//	uploadHeap.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	uploadHeap.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//	uploadHeap.CreationNodeMask = 1;
//	uploadHeap.VisibleNodeMask = 1;
//
//	D3D12_RESOURCE_DESC bufferDesc = {};
//	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	bufferDesc.Alignment = 0;
//	bufferDesc.Width = totalBytes;
//	bufferDesc.Height = 1;
//	bufferDesc.DepthOrArraySize = 1;
//	bufferDesc.MipLevels = 1;
//	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
//	bufferDesc.SampleDesc.Count = 1;
//	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	bufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	App::_DXData.Device->CreateCommittedResource(
//		&uploadHeap,
//		D3D12_HEAP_FLAG_NONE,
//		&bufferDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&uploadResource));
//
//	// Step 4: Copy data into upload buffer
//	BYTE* mapped = nullptr;
//	D3D12_RANGE range = { 0, 0 };
//	uploadResource->Map(0, &range, reinterpret_cast<void**>(&mapped));
//	for (UINT y = 0; y < numRows; y++)
//	{
//		memcpy(mapped + footprint.Offset + y * footprint.Footprint.RowPitch,
//			(BYTE*)rgbaPixels.data() + y * (16 * sizeof(uint32_t)),
//			16 * sizeof(uint32_t));
//	}
//	uploadResource->Unmap(0, nullptr);
//
//	// Step 5: Issue copy command
//	D3D12_TEXTURE_COPY_LOCATION dst = {};
//	dst.pResource = tex_resources;
//	dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
//	dst.SubresourceIndex = 0;
//
//	D3D12_TEXTURE_COPY_LOCATION src = {};
//	src.pResource = uploadResource;
//	src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//	src.PlacedFootprint = footprint;
//
//	// Transition -> copy dest
//	D3D12_RESOURCE_BARRIER barrier = {};
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier.Transition.pResource = tex_resources;
//	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
//	App::_DXData.CommandList->ResourceBarrier(1, &barrier);
//
//	App::_DXData.CommandList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
//
//	// Transition -> shader resource
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	App::_DXData.CommandList->ResourceBarrier(1, &barrier);
//
//	// UploadResource will be released when GPU finishes — keep alive until execute
//	App::ScheduleDXResourceRelease([uploadResource]()
//		{
//			uploadResource->Release();
//		});
//}
//
//
//void MemcpySubresource(
//	_In_ const D3D12_MEMCPY_DEST* pDest,
//	_In_ const D3D12_SUBRESOURCE_DATA* pSrc,
//	SIZE_T RowSizeInBytes,
//	UINT NumRows,
//	UINT NumSlices)
//{
//	for (UINT z = 0; z < NumSlices; ++z)
//	{
//		BYTE* pDestSlice = reinterpret_cast<BYTE*>(pDest->pData) + pDest->SlicePitch * z;
//		const BYTE* pSrcSlice = reinterpret_cast<const BYTE*>(pSrc->pData) + pSrc->SlicePitch * z;
//
//		for (UINT y = 0; y < NumRows; ++y)
//		{
//			memcpy(pDestSlice + pDest->RowPitch * y,
//				pSrcSlice + pSrc->RowPitch * y,
//				RowSizeInBytes);
//		}
//	}
//}
//
//void UpdateSubresources(
//	ID3D12GraphicsCommandList* pCmdList,
//	ID3D12Resource* pDestinationResource,
//	ID3D12Resource* pIntermediate,
//	UINT64 IntermediateOffset,
//	UINT FirstSubresource,
//	UINT NumSubresources,
//	const D3D12_SUBRESOURCE_DATA* pSrcData)
//{
//	ID3D12Device* device;
//	pDestinationResource->GetDevice(IID_PPV_ARGS(&device));
//
//	// Get copyable footprints
//	std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts(NumSubresources);
//	std::vector<UINT> numRows(NumSubresources);
//	std::vector<UINT64> rowSizesInBytes(NumSubresources);
//	UINT64 totalBytes = 0;
//
//	const D3D12_RESOURCE_DESC& desc = pDestinationResource->GetDesc();
//	device->GetCopyableFootprints(
//		&desc,
//		FirstSubresource,
//		NumSubresources,
//		IntermediateOffset,
//		layouts.data(),
//		numRows.data(),
//		rowSizesInBytes.data(),
//		&totalBytes);
//
//	// Copy data to intermediate resource
//	BYTE* pData;
//	HRESULT hr = pIntermediate->Map(0, nullptr, reinterpret_cast<void**>(&pData));
//	if (FAILED(hr)) return;
//
//	for (UINT i = 0; i < NumSubresources; ++i)
//	{
//		D3D12_MEMCPY_DEST DestData = {
//			pData + layouts[i].Offset,
//			layouts[i].Footprint.RowPitch,
//			layouts[i].Footprint.RowPitch * numRows[i] };
//
//		MemcpySubresource(&DestData, &pSrcData[i],
//			static_cast<SIZE_T>(rowSizesInBytes[i]),
//			numRows[i], layouts[i].Footprint.Depth);
//	}
//	pIntermediate->Unmap(0, nullptr);
//
//	// Copy from intermediate to destination
//	for (UINT i = 0; i < NumSubresources; ++i)
//	{
//		D3D12_TEXTURE_COPY_LOCATION dst = {};
//		dst.pResource = pDestinationResource;
//		dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
//		dst.SubresourceIndex = i + FirstSubresource;
//
//		D3D12_TEXTURE_COPY_LOCATION src = {};
//		src.pResource = pIntermediate;
//		src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
//		src.PlacedFootprint = layouts[i];
//
//		pCmdList->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);
//	}
//
//	device->Release();
//}
//
//// Helper function to calculate required intermediate size
//UINT64 CalculateRequiredIntermediateSize(ID3D12Resource* resource, UINT firstSubresource, UINT numSubresources)
//{
//	D3D12_RESOURCE_DESC desc = resource->GetDesc();
//	ID3D12Device* device;
//	resource->GetDevice(IID_PPV_ARGS(&device));
//
//	UINT64 totalSize = 0;
//	device->GetCopyableFootprints(&desc, firstSubresource, numSubresources, 0, nullptr, nullptr, nullptr, &totalSize);
//	device->Release();
//
//	return totalSize;
//}
//
//// Function to create texture from RGBA data
//ID3D12Resource* CreateTextureFromRGBA(ID3D12Device* device,
//	ID3D12GraphicsCommandList* commandList,
//	const std::vector<u8>& data,
//	int width, int height)
//{
//	ID3D12Resource* texture = nullptr;
//
//	// Create texture resource
//	D3D12_HEAP_PROPERTIES heapProps = {};
//	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
//	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//	heapProps.CreationNodeMask = 1;
//	heapProps.VisibleNodeMask = 1;
//
//	D3D12_RESOURCE_DESC resourceDesc = {};
//	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
//	resourceDesc.Alignment = 0;
//	resourceDesc.Width = width;
//	resourceDesc.Height = height;
//	resourceDesc.DepthOrArraySize = 1;
//	resourceDesc.MipLevels = 1;
//	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//	resourceDesc.SampleDesc.Count = 1;
//	resourceDesc.SampleDesc.Quality = 0;
//	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
//	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	HRESULT hr = device->CreateCommittedResource(
//		&heapProps,
//		D3D12_HEAP_FLAG_NONE,
//		&resourceDesc,
//		D3D12_RESOURCE_STATE_COPY_DEST,
//		nullptr,
//		IID_PPV_ARGS(&texture));
//
//	if (FAILED(hr)) return nullptr;
//
//	// Create upload buffer
//	const UINT64 uploadBufferSize = CalculateRequiredIntermediateSize(texture, 0, 1);
//
//	ID3D12Resource* uploadBuffer = nullptr;
//	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
//	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
//	uploadHeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
//	uploadHeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
//	uploadHeapProps.CreationNodeMask = 1;
//	uploadHeapProps.VisibleNodeMask = 1;
//
//	D3D12_RESOURCE_DESC uploadBufferDesc = {};
//	uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	uploadBufferDesc.Alignment = 0;
//	uploadBufferDesc.Width = uploadBufferSize;
//	uploadBufferDesc.Height = 1;
//	uploadBufferDesc.DepthOrArraySize = 1;
//	uploadBufferDesc.MipLevels = 1;
//	uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
//	uploadBufferDesc.SampleDesc.Count = 1;
//	uploadBufferDesc.SampleDesc.Quality = 0;
//	uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	uploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	hr = device->CreateCommittedResource(
//		&uploadHeapProps,
//		D3D12_HEAP_FLAG_NONE,
//		&uploadBufferDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&uploadBuffer));
//
//	if (FAILED(hr))
//	{
//		texture->Release();
//		return nullptr;
//	}
//
//	// Copy data to upload buffer and then to texture
//	D3D12_SUBRESOURCE_DATA textureData = {};
//	textureData.pData = data.data();
//	textureData.RowPitch = width * 4; // 4 bytes per pixel (RGBA)
//	textureData.SlicePitch = textureData.RowPitch * height;
//
//	// Use UpdateSubresources helper (you might need to implement this if not available)
//	UpdateSubresources(commandList, texture, uploadBuffer, 0, 0, 1, &textureData);
//
//	// Transition to shader resource state
//	D3D12_RESOURCE_BARRIER barrier = {};
//	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
//	barrier.Transition.pResource = texture;
//	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//
//	commandList->ResourceBarrier(1, &barrier);
//
//	// Upload buffer will be released after command list execution
//	App::ScheduleDXResourceRelease([uploadBuffer]()
//		{
//			uploadBuffer->Release();
//		});
//
//	return texture;
//}
//
//void UpdateTexture(ID3D12Device* device,
//	ID3D12GraphicsCommandList* commandList,
//	ID3D12Resource* texture,
//	const std::vector<unsigned char>& newData,
//	int width, int height)
//{
//	// Create upload buffer
//	const UINT64 uploadBufferSize = CalculateRequiredIntermediateSize(texture, 0, 1);
//
//	ID3D12Resource* uploadBuffer = nullptr;
//	D3D12_HEAP_PROPERTIES uploadHeapProps = {};
//	uploadHeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
//
//	D3D12_RESOURCE_DESC uploadBufferDesc = {};
//	uploadBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
//	uploadBufferDesc.Width = uploadBufferSize;
//	uploadBufferDesc.Height = 1;
//	uploadBufferDesc.DepthOrArraySize = 1;
//	uploadBufferDesc.MipLevels = 1;
//	uploadBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
//	uploadBufferDesc.SampleDesc.Count = 1;
//	uploadBufferDesc.SampleDesc.Quality = 0;
//	uploadBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
//	uploadBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
//
//	HRESULT hr = device->CreateCommittedResource(
//		&uploadHeapProps,
//		D3D12_HEAP_FLAG_NONE,
//		&uploadBufferDesc,
//		D3D12_RESOURCE_STATE_GENERIC_READ,
//		nullptr,
//		IID_PPV_ARGS(&uploadBuffer));
//
//	if (FAILED(hr)) return;
//
//	// Transition texture to copy destination
//	D3D12_RESOURCE_BARRIER barrier1 = {};
//	barrier1.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier1.Transition.pResource = texture;
//	barrier1.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier1.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	barrier1.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
//	commandList->ResourceBarrier(1, &barrier1);
//
//	// Copy new data to texture
//	D3D12_SUBRESOURCE_DATA textureData = {};
//	textureData.pData = newData.data();
//	textureData.RowPitch = width * 4;
//	textureData.SlicePitch = textureData.RowPitch * height;
//
//	UpdateSubresources(commandList, texture, uploadBuffer, 0, 0, 1, &textureData);
//
//	// Transition back to shader resource
//	D3D12_RESOURCE_BARRIER barrier2 = {};
//	barrier2.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
//	barrier2.Transition.pResource = texture;
//	barrier2.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
//	barrier2.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
//	barrier2.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
//	commandList->ResourceBarrier(1, &barrier2);
//
//	uploadBuffer->Release();
//}




EmulationImageTool::EmulationImageTool()
{
	m_Buffer.fill({ 255, 255, 255, 255 });
	/*App::ScheduleDXTask([this]()
		{
			std::vector<u8> rgbaPixels;
			rgbaPixels.resize(16 * 16 * 4, 255);
			tex_resources = CreateTextureFromRGBA(App::_DXData.Device, App::_DXData.CommandList, rgbaPixels, 16, 16);
		});*/
}
EmulationImageTool::~EmulationImageTool()
{
	/*if (tex_resources)
	{
		DestroyTexture(&tex_resources);
		App::D3D12Allocator->Free(my_texture_srv_cpu_handle, my_texture_srv_gpu_handle);
	}*/
	/*if (tex_resources)
	{
		tex_resources->Release();
		App::_DXData.Allocator->Free(my_texture_srv_cpu_handle, my_texture_srv_gpu_handle);
	}*/
}

void BoxBlur(std::array<RGBA, 256>& pixels, int width, int height, int kernelSize = 3) {
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
			pixels[index].r = sumR / count;
			pixels[index].g = sumG / count;
			pixels[index].b = sumB / count;
			pixels[index].a = sumA / count;
		}
	}
}

void EmulationImageTool::OnOutput()
{
	if (!m_Memory) return;

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

	if (m_Smooth)
		BoxBlur(m_Buffer, 16, 16);

	//App::ScheduleDXTask([this]()
	//	{
	//		std::vector<u8> rgbaPixels(16 * 16 * 4);
	//		for (u64 i = 0; i < 256; i++)
	//		{
	//			u8 c = (*m_Memory)[i];
	//			u8 r = (c >> 5) & 0x07;   // 3 bits
	//			u8 g = (c >> 2) & 0x07;   // 3 bits
	//			u8 b = (c >> 0) & 0x03;   // 2 bits

	//			// Expand to 8-bit range
	//			r = (r * 255) / 7;
	//			g = (g * 255) / 7;
	//			b = (b * 255) / 3;

	//			rgbaPixels[i * 4 + 0] = r;
	//			rgbaPixels[i * 4 + 1] = g;
	//			rgbaPixels[i * 4 + 2] = b;
	//			rgbaPixels[i * 4 + 3] = 255;
	//		}
	//		UpdateTexture(App::_DXData.Device, App::_DXData.CommandList, tex_resources, rgbaPixels, 16, 16);
	//	});
}

void EmulationImageTool::Render()
{
	ImGui::Checkbox("RGB332", &m_332);
	ImGui::SameLine();
	ImGui::Checkbox("Smooth", &m_Smooth);

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