#pragma once
#include "Tool.h"
#include "Utility.h"
#include "Events/Event.h"


class CompilationOutputTool : public Tool
{
public:
	CompilationOutputTool() = default;
	~CompilationOutputTool() override = default;

	void Reset();

	void Render() override;

	inline std::string Name() const override { return "Compilation Output"; }
	inline Type GetType() const override { return _GetType(); }
	static inline Type _GetType() { return COMP_OUT; }

	void SetOutput(const std::stringbuf* out) { m_Output = out; }

private:
	const std::stringbuf* m_Output;
};