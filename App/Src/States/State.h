#pragma once

class State
{
public:
	virtual ~State() = default;

	virtual void Render() {};

	static constexpr auto Name = "State";

protected:
	State() = default;

};