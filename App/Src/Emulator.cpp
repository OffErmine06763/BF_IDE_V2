#include "Emulator.h"

#include <fstream>
#include <string>
#include <stack>
#include <format>

#include <imgui.h>


Emulator::Emulator(const fs::path& file, const ocb_t& ocb, const icb_t& icb, const tcb_t& tcb)
	: thread([this, file]() { EmulateFile(file); }), m_OutputCB(ocb), m_InputCB(icb), m_TerminationCB(tcb)
{ }

void Emulator::EmulateFile(const fs::path& file)
{
	std::ifstream in(file);
	std::string content = { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	in.close();

	m_Running = true;
	std::stack<uint64_t> open;
	for (uint64_t i = 0; i < content.length(); i++) {
		if (!m_Running)
			break;

		std::unique_lock<std::mutex> lock(m_Mutex);
		char c = content[i];
		switch (c)
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
	m_Running = false;
	m_Done = true;
	// TODO: notify the outside
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
	if (!m_WantInput) return;

	std::lock_guard<std::mutex> lock(m_Mutex);
	m_Memory[m_Address] = input;
	m_WantInput = false;
	m_InputCondition.notify_all();
}