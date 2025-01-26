#pragma once
#include "State.h"


class SelectProjectState : public State
{
public:
	SelectProjectState() = default;
	~SelectProjectState() override = default;

	void Render();

private:
	void RenderFav();
	void RenderRec();

};