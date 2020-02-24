# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2020 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +

from Engineering.gui.engineering_diffraction.tabs.fitting.data_handling.data_widget import FittingDataWidget
from Engineering.gui.engineering_diffraction.tabs.fitting.plotting.plot_presenter import FittingPlotPresenter


class FittingPresenter(object):
    def __init__(self, view):
        self.view = view

        self.data_widget = FittingDataWidget(self.view, view=self.view.get_data_widget())
        self.plot_widget = FittingPlotPresenter(self.view, view=self.view.get_plot_widget())

        self.data_widget.presenter.plot_removed_notifier.add_subscriber(
            self.plot_widget.workspace_removed_observer)
        self.data_widget.presenter.plot_added_notifier.add_subscriber(
            self.plot_widget.workspace_added_observer)
        self.data_widget.presenter.all_plots_removed_notifier.add_subscriber(
            self.plot_widget.all_workspaces_removed_observer)
