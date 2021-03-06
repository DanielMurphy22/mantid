// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#pragma once

#include "MantidQtWidgets/Common/AlgorithmSelectorWidget.h"
#include <QDockWidget>

class MantidUI;
class ApplicationWindow;
class QPushButton;
class QProgressBar;
class QHBoxLayout;

class AlgorithmDockWidget : public QDockWidget {
  Q_OBJECT
public:
  AlgorithmDockWidget(MantidUI *mui, ApplicationWindow *w);
public slots:
  void update();
  void updateProgress(void *alg, const double p, const QString &msg,
                      double estimatedTime, int progressPrecision);
  void algorithmStarted(void *alg);
  void algorithmFinished(void *alg);

protected:
  void showProgressBar();
  void hideProgressBar(void *alg);
  void updateDetailsButton() const;

  MantidQt::MantidWidgets::AlgorithmSelectorWidget *m_selector;
  QPushButton *m_runningButton;
  QProgressBar *m_progressBar;
  QHBoxLayout *m_runningLayout;
  QList<void *> m_algID;
  friend class MantidUI;

private:
  MantidUI *m_mantidUI;
};