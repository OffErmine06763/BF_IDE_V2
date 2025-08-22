#pragma once
#include "Utility.h"
#include "History.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "d3d12_stuff.h"

class State;

template <typename S, std::enable_if_t<std::is_base_of_v<State, S>, bool> = true>
concept StateType = true;

// TODO: Razer Chroma SDK Wyvern to set lighting effects on razer devices
class App
{
public:
	struct DXData
	{
		ID3D12Device* Device;
		ExampleDescriptorHeapAllocator* Allocator;
		ID3D12DescriptorHeap* SrvDescHeap;
		ID3D12GraphicsCommandList* CommandList;
	};

public:
	static void Init(DXData data);
	static void Stop();

	static void Render(bool* done);
	static inline void RequestClose() { Instance->m_IsOpen = false; }
	template <StateType S, typename... Args>
	static void RequestNewState(Args&&... args);
	static void RequestOpenPath(const fs::path& path);
	static History& GetHistory() { return Instance->m_History; }

	/// Run the lambda on the UI/main thread, at the end of each frame
	static void ScheduleTask(callable cb);

private:
	App();
	~App();
	static std::unique_ptr<App> Instance;

	void _Render();
	void ProcessGlobalShortcuts();

	std::unique_ptr<State> m_State = nullptr, m_NextState = nullptr;
	bool m_IsOpen = true, m_NewStateRequested = false;

	std::mutex m_TaskMutex;
	std::vector<callable> m_Tasks;

	std::mutex m_DXTaskMutex;
	std::vector<callable> m_DXTasks;
	std::mutex m_DXResourceTaskMutex;
	std::vector<callable> m_DXResourceTasks;

	History m_History;

public:
	friend struct std::default_delete<App>;

	static const fs::path HistoryPath;
	
	static DXData _DXData;
};



template<StateType S, typename... Args>
void App::RequestNewState(Args&&... args)
{
	LOG_APP("Next state " << S::Name << '\n');
	Instance->m_NextState = std::make_unique<S>(std::forward<Args>(args)...);
	Instance->m_NewStateRequested = true;
}
