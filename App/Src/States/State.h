#pragma once

class State
{
public:
	virtual ~State() = default;

	virtual void Init() {}
	virtual void Render() {}

	static constexpr auto Name = "State"; // TODO: why is this here?

protected:
	State() = default;

};