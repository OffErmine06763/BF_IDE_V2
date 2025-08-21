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

	void SetMemory(const std::vector<bf_mem_t>* memory, const u32* addr) { m_Memory = memory; m_Address = addr; }

private:
	const std::vector<bf_mem_t>* m_Memory = nullptr;
	const u32* m_Address = nullptr;
};