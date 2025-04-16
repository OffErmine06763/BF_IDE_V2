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
	using ocb_t = std::function<void(const std::string&)>;
	using icb_t = std::function<void(void)>;
	using tcb_t = std::function<void(void)>;

public:
	Emulator(const fs::path& file, const ocb_t& ocb, const icb_t& icb, const tcb_t& tcb);

	void Stop();

	inline bool Done() const { return m_Done; }
	inline bool WantsInput() const { return m_WantInput; }

	void GiveInput(bf_mem_t input);
	void Lock(bool lock) { lock ? m_Mutex.lock() : m_Mutex.unlock(); }

private:
	void EmulateFile(const fs::path& file);

	std::mutex m_Mutex;
	std::condition_variable m_InputCondition;

	bool m_Running = false, m_WantInput = false, m_Done = false;

	uint32_t m_Address = 0;
	std::array<bf_mem_t, BF_MEMSIZE> m_Memory = { 0 };

	ocb_t m_OutputCB;
	icb_t m_InputCB;
	tcb_t m_TerminationCB;
};