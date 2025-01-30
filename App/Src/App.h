#pragma once
#include "Utility.h"

#include <memory>
#include <filesystem>


class State;

class App
{
public:
	static void Init();

	static void Render(bool* done);
	static inline void RequestClose() { Instance->m_IsOpen = false; }
	template <class S, typename... Args, std::enable_if_t<std::is_base_of_v<State, S>, bool> = true>
	static void RequestNewState(Args&&... args);
	static void RequestOpenPath(const fs::path& path);
	static History& GetHistory() { return Instance->m_History; }

private:
	App();
	~App();
	static std::unique_ptr<App> Instance;

	void _Render();

	std::unique_ptr<State> m_State = nullptr, m_NextState = nullptr;

	bool m_IsOpen = true, m_NewStateRequested = false;

	History m_History;

public:
	friend struct std::default_delete<App>;
};



template<class S, typename... Args, std::enable_if_t<std::is_base_of_v<State, S>, bool>>
void App::RequestNewState(Args&&... args)
{
	Instance->m_NextState = std::make_unique<S>(std::forward<Args>(args)...);
	Instance->m_NewStateRequested = true;
}
