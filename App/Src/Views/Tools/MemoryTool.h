#pragma once
#include "Tool.h"

class MemoryTool : public Tool
{
public:
	MemoryTool() = default;
	~MemoryTool() override = default;

	void Render() override;
	std::string Name() const override { return "Memory"; }
};