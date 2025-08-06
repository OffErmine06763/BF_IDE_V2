#pragma once
#include "State.h"
#include "Utility.h"

#include <filesystem>
#include <functional>

// TODO: barra di ricerca

class OpenProjectState : public State
{
public:
	OpenProjectState();
	~OpenProjectState() override;

	void Render() override;

private:
	void RenderDir(const fs::path& dir);
	void RenderOptions();
	void RenderForms();

	// void RenderCreateFolderForm(const fs::path& p);
	void DeletePath(const fs::path& path);

	fs::path m_Selected;

	bool m_CreatingFolder = false, m_CreatingFile = false, m_CreatingProj = false,
		 m_FailedDelete = false, m_WantDelete = false;

	bool m_RequestText = true;
	char m_RequestedText[256] = "";
	std::function<void(void)> m_OnCompletion;
};