#pragma once
#include "Utility.h"
#include "ViewModels/FileViewModel.h"
#include "State.h"
#include "Views/EditorView.h"
#include "Views/FileView.h"

// TODO: States create the Model, ViewModel, View and link them together.
//  this allows to reuse each component if necessary and to give initial values for testing.
class FileState : public State
{
public:
	static constexpr PathType Type = PathType::FILE;

	FileState(const fs::path& workdir);
	~FileState() override;

	void Render() override;

private:
	FileView m_View;
};