#pragma once
#include "Utility.h"
#include <thread>
#include <mutex>
#include <array>
#include <condition_variable>
 

class Emulator : public std::thread
{
public:
	Emulator(const fs::path& file, std::string& output);

	void Stop();

	inline bool Done() { return m_Done; }
	inline bool WantsInput() { return m_WantInput; }

	void GiveInput(bf_mem_t input);
	void Lock(bool lock) { lock ? m_Mutex.lock() : m_Mutex.unlock(); }

private:
	void EmulateFile(const fs::path& file);


	std::mutex m_Mutex;
	std::thread m_Thread;
	std::condition_variable m_InputCondition;

	bool m_Running = false, m_WantInput = false, m_Done = false;
	std::string* m_Output;

	uint32_t m_Address = 0;
	std::array<bf_mem_t, BF_MEMSIZE> m_Memory = { 0 };
};