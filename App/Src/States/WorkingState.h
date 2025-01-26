#pragma once
#include "State.h"
#include "Editor.h"


class WorkingState : public State 
{
public:
	WorkingState() = default;
	~WorkingState() override = default;

	void Render() override;

private:
	void RenderMainMenu();
	void RenderEditor();


	bool m_WantRedock = false;
	Editor m_Editor;

};