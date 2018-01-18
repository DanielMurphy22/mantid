#include "EnggDiffGSASFittingViewQtWidget.h"

namespace MantidQt {
namespace CustomInterfaces {

EnggDiffGSASFittingViewQtWidget::EnggDiffGSASFittingViewQtWidget() {
  m_ui.setupUi(this);
}

void EnggDiffGSASFittingViewQtWidget::displayLatticeParams(
    const Mantid::API::ITableWorkspace_sptr latticeParams) const {
  throw std::runtime_error("Not yet implemented");
}

void EnggDiffGSASFittingViewQtWidget::displayRwp(const double rwp) const {
  throw std::runtime_error("Not yet implemented");
}

std::string EnggDiffGSASFittingViewQtWidget::getFocusedFileName() const {
  throw std::runtime_error("Not yet implemented");
}

std::string EnggDiffGSASFittingViewQtWidget::getGSASIIProjectPath() const {
  throw std::runtime_error("Not yet implemented");
}

std::string EnggDiffGSASFittingViewQtWidget::getInstrumentFileName() const {
  throw std::runtime_error("Not yet implemented");
}

std::string EnggDiffGSASFittingViewQtWidget::getPathToGSASII() const {
  throw std::runtime_error("Not yet implemented");
}

double EnggDiffGSASFittingViewQtWidget::getPawleyDMin() const {
  throw std::runtime_error("Not yet implemented");
}

double EnggDiffGSASFittingViewQtWidget::getPawleyNegativeWeight() const {
  throw std::runtime_error("Not yet implemented");
}

std::vector<std::string>
EnggDiffGSASFittingViewQtWidget::getPhaseFileNames() const {
  throw std::runtime_error("Not yet implemented");
}

GSASRefinementMethod
EnggDiffGSASFittingViewQtWidget::getRefinementMethod() const {
  throw std::runtime_error("Not yet implemented");
}

std::pair<int, size_t>
EnggDiffGSASFittingViewQtWidget::getSelectedRunLabel() const {
  throw std::runtime_error("Not yet implemented");
}

void EnggDiffGSASFittingViewQtWidget::plotCurve(
    const std::vector<boost::shared_ptr<QwtData>> &curve) {
  throw std::runtime_error("Not yet implemented");
}

void EnggDiffGSASFittingViewQtWidget::resetCanvas() {
  throw std::runtime_error("Not yet implemented");
}

void EnggDiffGSASFittingViewQtWidget::setEnabled(const bool enabled) {
  m_ui.lineEdit_runFile->setEnabled(enabled);
  m_ui.pushButton_browseRunFile->setEnabled(enabled);
  m_ui.pushButton_loadRun->setEnabled(enabled);

  m_ui.lineEdit_instParamsFile->setEnabled(enabled);
  m_ui.pushButton_browseInstParams->setEnabled(enabled);

  m_ui.lineEdit_phaseFiles->setEnabled(enabled);
  m_ui.pushButton_browsePhaseFiles->setEnabled(enabled);

  m_ui.lineEdit_gsasProjPath->setEnabled(enabled);
  m_ui.pushButton_gsasProjPath->setEnabled(enabled);

  m_ui.lineEdit_gsasHome->setEnabled(enabled);
  m_ui.pushButton_browseGSASHome->setEnabled(enabled);

  m_ui.comboBox_refinementMethod->setEnabled(enabled);

  m_ui.lineEdit_pawleyDMin->setEnabled(enabled);
  m_ui.lineEdit_pawleyNegativeWeight->setEnabled(enabled);

  m_ui.checkBox_showRefinementResults->setEnabled(enabled);
}

bool EnggDiffGSASFittingViewQtWidget::showRefinementResultsSelected() const {
  throw std::runtime_error("Not yet implemented");
}

void EnggDiffGSASFittingViewQtWidget::updateRunList(
    const std::vector<std::pair<int, size_t>> &runLabels) {
  throw std::runtime_error("Not yet implemented");
}

void EnggDiffGSASFittingViewQtWidget::userWarning(
    const std::string &warningDescription) const {
  throw std::runtime_error("Not yet implemented");
}

} // CustomInterfaces
} // MantidQt
