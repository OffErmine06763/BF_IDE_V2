#pragma once
#include <string>

class Tool
{
public:
	virtual ~Tool() = default;

	virtual void Render() {}
	virtual std::string Name() const { return "Tool"; }

protected:
	Tool() = default;
};