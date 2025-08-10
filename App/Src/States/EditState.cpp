#include "EditState.h"
#include "App.h"

#include "MyImGuiWidgets.h"



// NOTE: the state owns the models and the main view
//       models do not own other MVVM components
//       views own the view model, and potentially sub-views
//       this way, when changing state, every MVVM component should be destroyed.



EditState::EditState(const fs::path& workdir)
	: m_Workdir(workdir), m_Model(workdir, &m_Editor), m_View(&m_Model, &m_Editor)
{
	LOG_APP("EditState Created\n");
}

EditState::~EditState()
{
	LOG_APP("EditState Destroyed\n");
}

