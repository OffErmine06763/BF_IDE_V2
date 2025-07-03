#pragma once
#include "Utility.h"


class State;

template <typename S, std::enable_if_t<std::is_base_of_v<State, S>, bool> = true>
concept StateType = true;

// TODO: Razer Chroma SDK Wyvern to set lighting effects on razer devices
class App
{
public:
	static void Init();

	static void Render(bool* done);
	static inline void RequestClose() { Instance->m_IsOpen = false; }
	template <StateType S, typename... Args>
	static void RequestNewState(Args&&... args);
	static void RequestOpenPath(const fs::path& path);
	static History& GetHistory() { return Instance->m_History; }

private:
	App();
	~App();
	static std::unique_ptr<App> Instance;

	void _Render();
	void ProcessGlobalShortcuts();

	std::unique_ptr<State> m_State = nullptr, m_NextState = nullptr;

	bool m_IsOpen = true, m_NewStateRequested = false;

	History m_History;

public:
	friend struct std::default_delete<App>;

	static const fs::path HistoryPath;
};



template<StateType S, typename... Args>
void App::RequestNewState(Args&&... args)
{
	LOG_APP("Next state " << S::Name << '\n');
	Instance->m_NextState = std::make_unique<S>(std::forward<Args>(args)...);
	Instance->m_NewStateRequested = true;
}
