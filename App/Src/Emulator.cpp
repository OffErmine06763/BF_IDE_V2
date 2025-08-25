#include "Emulator.h"

BFC::CompilerError Emulator::Start(BFC::CompilationParams p)
{
	using namespace BFC;

	auto error = p.Validate();
	if (error)
		return error;

	Reset();

	// ids of all externs, per file
	std::vector<hset<u32>> externs;
	// position of the definition of each export, per file
	std::vector<hmap<std::string, size_t>> exports;
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
		IRData& irdata = m_IRs[m_IRs.size() - 1];
		hset<u32> fileexterns;
		hmap<std::string, size_t> fileexports;
		const auto& code = m_IRs.crbegin()->IR.code;
		for (size_t i = 0; i < code.size(); i++)
		{
			if (code[i].type == IR_LABEL)
			{
				irdata.Functions[code[i].ID] = { .IRIdx = m_IRs.size() - 1, .InstrIdx = i };
				if (irdata.IR.exports.contains(irdata.IR.names.at(code[i].ID)))
					fileexports.insert({ irdata.IR.names.at(code[i].ID), i });
			}
			else if (code[i].type == IR_LOOP)
				irdata.Loops[code[i].ID] = i;
			else if (code[i].type == IR_GOTO && irdata.IR.externs.contains(irdata.IR.names.at(code[i].ID)))
				fileexterns.insert(code[i].ID);
		}
		externs.push_back(fileexterns);
		exports.push_back(fileexports);

		if (p.IsMainFile(path))
		{
			m_CurrentIR.push(m_IRs.size() - 1);
			m_MainInd = m_CurrentIR.top();
		}
	}

	for (size_t i = 0; i < m_IRs.size(); i++)
	{
		IRData& iri = m_IRs[i];
		hset<u32>& exti = externs[i];
		
		for (size_t j = 0; j < m_IRs.size() && !exti.empty(); j++)
		{
			if (i == j) continue;
			IRData& irj = m_IRs[j];
			hmap<std::string, size_t>& expj = exports[j];

			hset<u32> toerase;
			for (const u32 ext : exti)
			{
				if (irj.IR.exports.contains(iri.IR.names.at(ext)))
				{
					toerase.insert(ext);
					iri.Functions[ext] = { .IRIdx = j, .InstrIdx = expj.at(iri.IR.names.at(ext)) };
				}
			}

			for (const u32 e : toerase)
				exti.erase(e);
		}
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


	auto& ir = m_IRs[m_CurrentIR.top()];
	auto& i = m_CurrentInstruction.top();
	
	BFC::IRInstruction current = ir.IR.code[i];
	i++;
	switch (current.type)
	{
	case BFC::IR_GOTO:
		m_CurrentInstruction.push(ir.Functions.at(current.ID).InstrIdx);
		m_CurrentIR.push(ir.Functions.at(current.ID).IRIdx);
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
		break;
	case BFC::IR_LABEL: // noop
		break;
	case BFC::IR_RET:
		m_CurrentInstruction.pop();
		m_CurrentIR.pop();
		i = m_CurrentInstruction.top();
		ir = m_IRs[m_CurrentIR.top()];
		break;
	case BFC::IR_JZ:
		// jumping to the loop end, shouldn't change the IR
		if (m_Memory[m_Address] == 0)
			i = ir.Loops.at(current.ID);
		break;
	case BFC::IR_JMP:
		// jumping to the loop start, shouldn't change the IR
		i = ir.Loops.at(current.ID);
		break;
	}

	if (ir.IR.code.size() == i)
	{
		m_Running = false;

		if (m_CurrentIR.top() != m_MainInd)
		{
			params.Error = EOF_REACHED;
			params.ErrorDescription = "Unexpected reach of EOF in file "s + ir.Path.filename().string() + '\n';
		}
	}
}

void Emulator::Stop()
{
	m_Running = false;
}

void Emulator::Reset()
{
	m_WantsInput = false;
	m_Running = false;

	m_Address = 0;
	m_Memory.resize(BF_MEMSIZE, 0);
	std::memset(m_Memory.data(), 0, sizeof(bf_mem_t) * m_Memory.size());
	
	m_IRs.clear();
	m_CurrentIR = {};
	m_CurrentInstruction = {};
	m_Counter = 0;
	m_MainInd = 0;
}
