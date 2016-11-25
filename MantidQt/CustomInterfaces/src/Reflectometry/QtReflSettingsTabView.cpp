#include "MantidQtCustomInterfaces/Reflectometry/QtReflSettingsTabView.h"
#include "MantidQtCustomInterfaces/Reflectometry/QtReflSettingsView.h"
#include "MantidQtCustomInterfaces/Reflectometry/ReflSettingsTabPresenter.h"

namespace MantidQt {
namespace CustomInterfaces {

//----------------------------------------------------------------------------------------------
/** Constructor
* @param parent :: [input] The parent of this widget
*/
QtReflSettingsTabView::QtReflSettingsTabView(QWidget *parent) {

  UNUSED_ARG(parent);
  initLayout();

  m_presenter.reset(new ReflSettingsTabPresenter());
}

//----------------------------------------------------------------------------------------------
/** Destructor
*/
QtReflSettingsTabView::~QtReflSettingsTabView() {}

/**
Initialise the interface
*/
void QtReflSettingsTabView::initLayout() {
  m_ui.setupUi(this);

  QtReflSettingsView *settings_1 = new QtReflSettingsView(this);
  m_ui.toolbox->addItem(settings_1, "Group 1");

  QtReflSettingsView *settings_2 = new QtReflSettingsView(this);
  m_ui.toolbox->addItem(settings_2, "Group 2");
}

/** Returns the presenter managing this view
* @return :: A pointer to the presenter
*/
IReflSettingsTabPresenter *QtReflSettingsTabView::getPresenter() const {

  return m_presenter.get();
}

} // namespace CustomInterfaces
} // namespace Mantid
