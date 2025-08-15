#pragma once
#include "Utility.h"
#include <Compiler.h>
#include "Events/Event.h"

#include "IR.h"

class Emulator
{
public:
	enum RuntimeError : u8
	{
		NONE, EOF_REACHED
	};
	struct StepParams
	{
		bf_mem_t I = 0;
		bf_mem_t O = 0;
		bool InputRequested = false;
		bool OutputGenerated = false;
		RuntimeError Error = NONE;
		std::string ErrorDescription = "";
	};

public:
	Emulator() { Reset(); }

	BFC::CompilerError Start(BFC::CompilationParams p);	
	void Step(StepParams& params);
	void Stop();
	void Reset();

	bool Running() const { return m_Running; }

	u32 GetAddress() const { return m_Address; }
	bool WantsInput() const { return m_WantsInput; }
	bf_mem_t GetMemory(const u32 address) const { return m_Memory[address]; }
	std::vector<bf_mem_t>& GetMemory() { return m_Memory; }
	bf_mem_t SetMemory(const u32 address, bf_mem_t value) { m_Memory[address] = value; }

public:
	static constexpr u32 MEM_SIZE = BF_MEMSIZE;

private:
	bool m_WantsInput = false, m_Running = false;

	u32 m_Address = 0;
	std::vector<bf_mem_t> m_Memory;

	struct FunctionData
	{
		size_t IRIdx, InstrIdx;
	};
	struct IRData
	{
		BFC::IR IR;
		fs::path Path;
		std::map<u32, size_t> Loops;
		std::map<u32, FunctionData> Functions;
	};
	std::vector<IRData> m_IRs;
	std::stack<size_t> m_CurrentIR, m_CurrentInstruction;
	u32 m_Counter = 0;
	size_t m_MainInd = -1;
};

/*
setup:
	code
	ret

main:
	setup()

	custom code
	fn()
	custom code

	exit()



fn:
	custom code

*/