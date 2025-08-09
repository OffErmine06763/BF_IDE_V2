#pragma once
#include "Utility.h"
#include <thread>
#include <mutex>
#include <array>
#include <condition_variable>
#include <functional>
 

class Emulator : public std::thread
{
public:
	Emulator(const fs::path& file, const consumer<const std::string&>& ocb, const callable& icb, const callable& tcb);

	void Stop();

	inline bool Done() const { return m_Done; }
	inline bool WantsInput() const { return m_WantInput; }

	void GiveInput(bf_mem_t input);

private:
	void EmulateFile(const fs::path& file);

	std::mutex m_Mutex;
	std::condition_variable m_InputCondition;

	bool m_Running = false, m_WantInput = false, m_Done = false;

	uint32_t m_Address = 0;
	std::array<bf_mem_t, BF_MEMSIZE> m_Memory = { 0 };

	consumer<const std::string&> m_OutputCB;
	callable m_InputCB;
	callable m_TerminationCB;
};
