#pragma once
#include <memory>

class State;

class App
{
public:
	static void Init();

	static void Render(bool* done);
	static inline void RequestClose() { Instance->m_IsOpen = false; }

private:
	App();
	~App();
	static std::unique_ptr<App> Instance;

	void _Render();

	std::unique_ptr<State> m_State;

	bool m_IsOpen = true;

public:
	friend struct std::default_delete<App>;
};