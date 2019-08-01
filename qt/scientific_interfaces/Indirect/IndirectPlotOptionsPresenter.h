// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2019 ISIS Rutherford Appleton Laboratory UKRI,
//     NScD Oak Ridge National Laboratory, European Spallation Source
//     & Institut Laue - Langevin
// SPDX - License - Identifier: GPL - 3.0 +
#ifndef MANTIDQTCUSTOMINTERFACES_INDIRECTPLOTOPTIONSPRESENTER_H_
#define MANTIDQTCUSTOMINTERFACES_INDIRECTPLOTOPTIONSPRESENTER_H_

#include "IndirectPlotOptionsModel.h"
#include "IndirectPlotOptionsView.h"
#include "IndirectTab.h"

#include "DllConfig.h"

#include <Poco/NObserver.h>

#include <QObject>

namespace MantidQt {
namespace CustomInterfaces {

class MANTIDQT_INDIRECT_DLL IndirectPlotOptionsPresenter : public QObject {
  Q_OBJECT

public:
  IndirectPlotOptionsPresenter(IndirectPlotOptionsView *view,
                               IndirectTab *parent,
                               PlotWidget const &plotType = PlotWidget::Spectra,
                               std::string const &fixedIndices = "");
  /// Used by the unit tests so that the view and model can be mocked
  IndirectPlotOptionsPresenter(IndirectPlotOptionsView *view,
                               IndirectPlotOptionsModel *model,
                               PlotWidget const &plotType = PlotWidget::Spectra,
                               std::string const &fixedIndices = "");
  ~IndirectPlotOptionsPresenter() override;

  void setWorkspaces(std::vector<std::string> const &workspaces);
  void clearWorkspaces();

private slots:
  void workspaceChanged(std::string const &workspaceName);
  void indicesChanged(std::string const &indices);
  void plotSpectra();
  void plotBins();
  void plotContour();
  void plotTiled();

private:
  void setupPresenter(PlotWidget const &plotType,
                      std::string const &fixedIndices);
  void watchADS(bool on);

  void setPlotting(bool plotting);
  void setOptionsEnabled(bool enable);

  void onWorkspaceRemoved(Mantid::API::WorkspacePreDeleteNotification_ptr nf);
  void
  onWorkspaceReplaced(Mantid::API::WorkspaceBeforeReplaceNotification_ptr nf);

  void setWorkspace(std::string const &plotWorkspace);
  void setIndices();

  // Observers for ADS Notifications
  Poco::NObserver<IndirectPlotOptionsPresenter,
                  Mantid::API::WorkspacePreDeleteNotification>
      m_wsRemovedObserver;
  Poco::NObserver<IndirectPlotOptionsPresenter,
                  Mantid::API::WorkspaceBeforeReplaceNotification>
      m_wsReplacedObserver;

  IndirectPlotOptionsView *m_view;
  std::unique_ptr<IndirectPlotOptionsModel> m_model;
};

} // namespace CustomInterfaces
} // namespace MantidQt

#endif /* MANTIDQTCUSTOMINTERFACES_INDIRECTPLOTOPTIONSPRESENTER_H_ */