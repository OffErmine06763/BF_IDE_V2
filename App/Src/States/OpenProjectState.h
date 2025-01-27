#pragma once
#include "State.h"

#include <filesystem>
#include <functional>


class OpenProjectState : public State
{
public:
	OpenProjectState();
	~OpenProjectState() override;

	void Render() override;

private:
	void RenderDir(const std::filesystem::path& dir);
	void RenderOptions();
	void RenderForms();

	// void RenderCreateFolderForm(const std::filesystem::path& p);
	void DeletePath(const std::filesystem::path& path);

	std::filesystem::path m_Selected;

	bool m_CreatingFolder = false, m_CreatingFile = false, m_CreatingProj = false,
		 m_FailedDelete = false, m_WantDelete = false;

	bool m_RequestText = true;
	char m_RequestedText[256] = "";
	std::function<void(void)> m_OnCompletion;
};