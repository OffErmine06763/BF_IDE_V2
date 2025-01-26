#pragma once

class State
{
public:
	virtual ~State() = default;

	virtual void Render() {};

protected:
	State() = default;

};