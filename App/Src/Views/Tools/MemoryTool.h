#pragma once
#include "Tool.h"
#include "Utility.h"


class MemoryTool : public Tool
{
public:
	MemoryTool() = default;
	~MemoryTool() override = default;

	void Render() override;

	inline std::string Name() const override { return "Memory"; }
	inline Type GetType() const override { return _GetType(); }
	static inline Type _GetType() { return MEMORY; }

	void SetMemory(const std::vector<bf_mem_t>* memory) { m_Memory = memory; }

private:
	const std::vector<bf_mem_t>* m_Memory = nullptr;
};