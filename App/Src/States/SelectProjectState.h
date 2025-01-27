#pragma once
#include "State.h"

#include <iostream>


class SelectProjectState : public State
{
public:
	SelectProjectState();
	~SelectProjectState() override;

	void Render() override;

private:
	void RenderFav();
	void RenderRec();
};