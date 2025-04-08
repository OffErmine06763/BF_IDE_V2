#include "Emulator.h"

#include <fstream>
#include <string>
#include <stack>
#include <format>

#include <imgui.h>


Emulator::Emulator(const fs::path& file, std::string& output)
	: thread([this, file]() { EmulateFile(file); }), m_Output(&output)
{ }

void Emulator::EmulateFile(const fs::path& file)
{
	std::ifstream in(file);
	std::string content = { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
	in.close();

	m_Running = true;
	std::stack<uint32_t> open;
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
			m_Output->append(std::format("{}", m_Memory[m_Address]));
			break;
		case BF_INP: 
			m_WantInput = true;
			m_InputCondition.wait(lock, [this]() { return !m_WantInput; });
			break;
		default: break;
		}
	}
	m_Running = false;
	m_Done = true;
	// TODO: notify the outside
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
	m_Memory[m_Address] = input;
	m_WantInput = false;
	m_InputCondition.notify_all();
}