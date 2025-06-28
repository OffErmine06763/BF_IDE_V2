#pragma once
#include "State.h"
#include "Utility.h"
#include "Models/EditorModel.h"
#include "Models/EditModel.h"
#include "Views/EditView.h"
#include "ViewModels/EditViewModel.h"


class EditState : public State
{
public:
	EditState(const fs::path& workdir);
	~EditState() override = default;

	void Render() override;

private:
	fs::path m_Workdir;

	EditorModel m_Editor;
	EditModel m_Model;
	EditView m_View;
};