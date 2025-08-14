#include "Emulator.h"

BFC::CompilerError Emulator::Start(BFC::CompilationParams p)
{
	using namespace BFC;

	auto error = p.Validate();
	if (error)
		return error;

	Reset();

	for (const fs::path& path : p.tgts)
	{
		auto expTokens = Compiler::Tokenize(path);
		if (!expTokens.success())
			return expTokens._getU();
		TokenizeResult tokens = expTokens._consumeE();


		auto expParse = Compiler::Parse(std::move(tokens));
		if (!expParse.success())
			return expParse._getU();
		TU ast = expParse._consumeE();


		auto expAnal = Compiler::Analyze(ast);
		if (expAnal.has_value())
			return expAnal.value();


		if (p.optimize)
			Compiler::Optimize(ast);


		m_IRs.push_back({ .IR = Compiler::Intermediate(std::move(ast)), .Path = path });

		const auto& code = m_IRs.crbegin()->IR.code;
		for (size_t i = 0; i < code.size(); i++)
			if (code[i].type == IR_LABEL || code[i].type == IR_LOOP)
				m_Functions[code[i].ID] = { .IRIdx = m_IRs.size() - 1, .InstrIdx = i };

		if (p.IsMainFile(path))
			m_CurrentIR.push(m_IRs.size() - 1);
	}

	m_CurrentInstruction.push(0);
	m_Running = true;

	return { CompilerError::NONE, "" };
}

void Emulator::Step(StepParams& params)
{
	if (!m_Running)
		return;

	if (m_WantsInput)
	{
		m_Memory[m_Address] = params.I;
		m_WantsInput = false;
	}


	const auto& ir = m_IRs[m_CurrentIR.top()];
	auto& i = m_CurrentInstruction.top();
	
	BFC::IRInstruction current = ir.IR.code[i];
	i++;
	switch (current.type)
	{
	case BFC::IR_GOTO:
		m_CurrentInstruction.push(m_Functions[current.ID].InstrIdx);
		m_CurrentIR.push(m_Functions[current.ID].IRIdx);
		break;
	case BFC::IR_I:
		params.InputRequested = true;
		m_WantsInput = true;
		if (current.count > m_Counter)
		{
			m_Counter++;
			i--;
		}
		else m_Counter = 0;
		break;
	case BFC::IR_O:
		params.OutputGenerated = true;
		params.O = m_Memory[m_Address];
		if (current.count > m_Counter)
		{
			m_Counter++;
			i--;
		}
		else m_Counter = 0;
		break;
	case BFC::IR_LEFT:
		m_Address -= current.count + 1;
		break;
	case BFC::IR_RIGHT:
		m_Address += current.count + 1;
		break;
	case BFC::IR_DEC:
		m_Memory[m_Address] -= current.count + 1;
		break;
	case BFC::IR_INC:
		m_Memory[m_Address] += current.count + 1;
		break;
	case BFC::IR_LOOP:
	case BFC::IR_LABEL: // noop
		break;
	case BFC::IR_RET:
		m_CurrentInstruction.pop();
		m_CurrentIR.pop();
		break;
	case BFC::IR_JZ:
		// jumping to the loop end, shouldn't change the IR
		if (m_Memory[m_Address] == 0)
			i = m_Functions[current.ID].InstrIdx;
		break;
	case BFC::IR_JMP:
		// jumping to the loop start, shouldn't change the IR
		i = m_Functions[current.ID].InstrIdx;
		break;
	}

	if (ir.IR.code.size() == i)
	{
		m_Running = false;

		if (m_CurrentIR.size() != 1)
		{
			params.Error = EOF_REACHED;
			params.ErrorDescription = "Unexpected reach of EOF in file "s + ir.Path.filename().string();
		}
	}
}

void Emulator::Stop()
{
	m_Running = false;
}

void Emulator::Reset()
{
	m_Counter = 0;
	m_Address = 0;
	m_Memory.resize(BF_MEMSIZE, 0);
	std::memset(m_Memory.data(), 0, sizeof(bf_mem_t) * m_Memory.size());
	m_CurrentInstruction = {};
	m_CurrentIR = {};
	m_Running = false;
	m_WantsInput = false;
}



//Emulator::Emulator(const fs::path& file, const consumer<const std::string&>& ocb, const callable& icb, const callable& tcb)
//	: thread([this, file]() { EmulateFile(file); }), m_OutputCB(ocb), m_InputCB(icb), m_TerminationCB(tcb)
//{
//}
//
//void Emulator::EmulateFile(const fs::path& file)
//{
//	LOG_COMP("Starting Emulation of " << file.filename() << '\n');
//
//	std::ifstream in(file);
//	std::string content = { std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>() };
//	in.close();
//
//	m_Running = true;
//	m_Memory = { 0 };
//	std::stack<uint64_t> open;
//	for (uint64_t i = 0; i < content.length(); i++) {
//		std::this_thread::sleep_for(EmulationSleep); // TODO: allow different emulation modes, like as fast as possible / slow that highlights the current instruction
//
//		std::unique_lock<std::mutex> lock(m_Mutex);
//		if (!m_Running)	break;
//
//		switch (content[i])
//		{
//		case BF_INC: m_Memory[m_Address]++; break; // TODO: warning for overflows
//		case BF_DEC: m_Memory[m_Address]--; break;
//		case BF_OPN: open.push(i); break;
//		case BF_CLS:
//			if (m_Memory[m_Address] != 0) i = open.top();
//			else open.pop();
//			break;
//		case BF_MVR: m_Address = (m_Address + 1) % BF_MEMSIZE; break; // TODO: we want pacman? or error?
//		case BF_MVL: m_Address = (m_Address - 1) % BF_MEMSIZE; break;
//		case BF_OUT:
//			m_OutputCB(std::format("{}", m_Memory[m_Address]));
//			break;
//		case BF_INP:
//			m_WantInput = true;
//			m_InputCB();
//			m_InputCondition.wait(lock, [this]() { return !m_WantInput; });
//			break;
//		default: break;
//		}
//	}
//
//	LOG_COMP("Done Emulating " << file.filename() << '\n');
//
//	m_Running = false;
//	m_Done = true;
//	m_WantInput = false;
//	m_TerminationCB();
//}
//
//void Emulator::Stop()
//{
//	std::lock_guard<std::mutex> lock(m_Mutex);
//	m_Running = false;
//	m_WantInput = false;
//	m_InputCondition.notify_all();
//}
//
//void Emulator::GiveInput(bf_mem_t input)
//{
//	std::lock_guard<std::mutex> lock(m_Mutex);
//	if (!m_WantInput) return;
//
//	m_Memory[m_Address] = input;
//	m_WantInput = false;
//	m_InputCondition.notify_all();
//}
