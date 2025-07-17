#include "Emulator.h"

#include <fstream>
#include <string>
#include <stack>
#include <format>


Emulator::Emulator(const fs::path& file, const consumer<const std::string&>& ocb, const callable& icb, const callable& tcb)
	: thread([this, file]() { EmulateFile(file); }), m_OutputCB(ocb), m_InputCB(icb), m_TerminationCB(tcb)
{ }

void Emulator::EmulateFile(const fs::path& file)
{
	LOG_COMP("Starting Emulation of " << file.filename() << '\n');

	std::ifstream in(file);
	std::string content = { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	in.close();

	m_Running = true;
	m_Memory = { 0 };
	std::stack<uint64_t> open;
	for (uint64_t i = 0; i < content.length(); i++) {
		std::this_thread::sleep_for(EmulationSleep); // TODO: allow different emulation modes, like as fast as possible / slow that highlights the current instruction
		
		std::unique_lock<std::mutex> lock(m_Mutex);
		if (!m_Running)	break;

		switch (content[i])
		{
		case BF_INC: m_Memory[m_Address]++; break; // TODO: warning for overflows
		case BF_DEC: m_Memory[m_Address]--; break;
		case BF_OPN: open.push(i); break;
		case BF_CLS:
			if (m_Memory[m_Address] != 0) i = open.top();
			else open.pop();
			break;
		case BF_MVR: m_Address = (m_Address + 1) % BF_MEMSIZE; break; // TODO: we want pacman? or error?
		case BF_MVL: m_Address = (m_Address - 1) % BF_MEMSIZE; break;
		case BF_OUT: 
			m_OutputCB(std::format("{}", m_Memory[m_Address]));
			break;
		case BF_INP: 
			m_WantInput = true;
			m_InputCB();
			m_InputCondition.wait(lock, [this]() { return !m_WantInput; });
			break;
		default: break;
		}
	}
	
	LOG_COMP("Done Emulating " << file.filename() << '\n');
	
	m_Running = false;
	m_Done = true;
	m_WantInput = false;
	m_TerminationCB();
}

void Emulator::Stop()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	m_Running = false;
	m_WantInput = false;
	m_InputCondition.notify_all();
}

void Emulator::GiveInput(bf_mem_t input) 
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	if (!m_WantInput) return;

	m_Memory[m_Address] = input;
	m_WantInput = false;
	m_InputCondition.notify_all();
}