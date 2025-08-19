#pragma once
#include <string>


class Tool
{
public:
	enum Type
	{
		NONE, TREE, MEMORY, EMU_IO
	};

public:
	virtual ~Tool() = default;

	virtual void Render() {}
	virtual inline std::string Name() const { return "Tool"; }
	virtual inline Type GetType() const { return NONE; }

protected:
	Tool() = default;
};