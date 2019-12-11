# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from __future__ import (absolute_import, division, print_function)
import numpy as np

from mantid.api import AnalysisDataService
import matplotlib.pyplot as plt

legend_text_size = 7


class PlotWidgetModel(object):

    @staticmethod
    def do_single_plot(ax, workspaces, errors, nums, kw, plot_kwargs):
        plot_fn = ax.errorbar if errors else ax.plot
        for ws in workspaces:
            for num in nums:
                plot_kwargs[kw] = num
                plot_fn(ws, **plot_kwargs)

    def __init__(self):
        self._plotted_workspaces = []
        self._plotted_workspaces_inverse_binning = {}
        self._plotted_fit_workspaces = []
        self.tiled_plot_positions = {}
        self.plotted_group = ''
        self._number_of_axes = 1

    # ------------------------------------------------------------------------------------------------------------------
    # Properties
    # ------------------------------------------------------------------------------------------------------------------

    @property
    def number_of_axes(self):
        """
        This property is needed as the view will generate an even number of axes, which can exceed the number of axes
        required by the model
        """
        return self._number_of_axes

    @property
    def plotted_workspaces(self):
        """
        A list of the plotted workspaces in the figure
        """
        return self._plotted_workspaces

    # NOTE: Not sure if this property is necessary, or used for anything
    @property
    def plotted_workspaces_inverse_binning(self):
        return self._plotted_workspaces_inverse_binning

    @property
    def plotted_fit_workspaces(self):
        """
        A list of the plotted fit workspaces in the figure
        """
        return self._plotted_fit_workspaces

    # ------------------------------------------------------------------------------------------------------------------
    # Setters
    # ------------------------------------------------------------------------------------------------------------------

    @number_of_axes.setter
    def number_of_axes(self, number_of_axes):
        if number_of_axes > 1:
            self._number_of_axes = number_of_axes
        else:
            self._number_of_axes = 1

    @plotted_workspaces.setter
    def plotted_workspaces(self, workspaces):
        self._plotted_workspaces = workspaces

    @plotted_workspaces_inverse_binning.setter
    def plotted_workspaces_inverse_binning(self, value):
        self._plotted_workspaces_inverse_binning = value

    @plotted_fit_workspaces.setter
    def plotted_fit_workspaces(self, workspaces):
        self._plotted_fit_workspaces = workspaces

    # ------------------------------------------------------------------------------------------------------------------
    # Helper to add and remove from stored workspace lists
    # ------------------------------------------------------------------------------------------------------------------

    def add_workspace_to_plotted_workspaces(self, workspace_name):
        if workspace_name not in self.plotted_workspaces:
            self._plotted_workspaces.append(workspace_name)

    def remove_workspaces_from_plotted_workspaces(self, workspace_name):
        if workspace_name in self.plotted_workspaces:
            self._plotted_workspaces.remove(workspace_name)

    def add_workspace_to_plotted_fit_workspaces(self, workspace_name):
        if workspace_name not in self.plotted_fit_workspaces:
            self._plotted_fit_workspaces.append(workspace_name)

    def remove_workspaces_from_plotted_fit_workspaces(self, workspace_name):
        if workspace_name in self.plotted_fit_workspaces:
            self._plotted_fit_workspaces.remove(workspace_name)

    # ------------------------------------------------------------------------------------------------------------------
    # Plotting
    # ------------------------------------------------------------------------------------------------------------------

    def plot_workspace(self, workspace, wksp_indices, ax, errors, plot_kwargs):
        """
            Plot workspace
            :param workspace: Workspace to plot
            :param ax: Axis to plot the workspace on
            :param wksp_indices: Indices to plot
            :param errors: Whether to plot errors ( true or false)
            :param plot_kwargs: Keywords to the mantid axis plotting
            :return:
            """
        kw, nums = 'wkspIndex', wksp_indices
        self.do_single_plot(ax, workspace, errors, nums, kw, plot_kwargs)
        ax.legend(prop=dict(size=legend_text_size))

    def add_workspace_to_plot(self, ax, workspace_name, workspace_index, label):
        """
            Adds a plot line to the specified subplot
            :param ax: Axis to plot the workspace on
            :param workspace_name: Name of workspace to get plot data from
            :param workspace_index: workspace index to plot from workspace
            :param label: Legend label for workspace, if none use default generated label
            :return:
            """
        # check workspace exists
        try:
            workspaces = AnalysisDataService.Instance().retrieveWorkspaces([workspace_name], unrollGroups=True)
        except RuntimeError:
            return

        self.plot_workspace(workspaces, [workspace_index], ax, False,
                            plot_kwargs={'distribution': True, 'zorder': 4,
                                         'autoscale_on_update': False, 'label': label})
        self._update_legend(ax)

    def remove_workspace_from_plot(self, workspace_name, axes):
        """
        Remo
        :param workspace_name: Name of workspace to remove from plot
        :param axes: the axes which may contain the workspace
        :return:
        """
        try:
            workspace = AnalysisDataService.Instance().retrieve(workspace_name)
        except RuntimeError:
            return
        for i in range(self.number_of_axes):
            ax = axes[i]
            ax.remove_workspace_artists(workspace)
            self._update_legend(ax)

        self.plotted_workspaces = [item for item in self.plotted_workspaces if item != workspace_name]
        self.plotted_fit_workspaces = [item for item in self.plotted_fit_workspaces if item != workspace_name]
        if workspace_name in self.plotted_workspaces_inverse_binning:
            self.plotted_workspaces_inverse_binning.pop(workspace_name)

    def clear_plot_model(self, axes):
        self._remove_all_data_workspaces_from_plot(axes)
        self.plotted_workspaces = []
        self.plotted_workspaces_inverse_binning = {}
        self.plotted_fit_workspaces = []

    def set_x_lim(self, domain, axes):
        if domain == "Time":
            ymin, ymax = self._get_autoscale_y_limits(axes, 0, 15)
            plt.setp(axes, xlim=[0, 15.0], ylim=[ymin, ymax])
        if domain == "Frequency":
            ymin, ymax = self._get_autoscale_y_limits(axes, 0, 50)
            plt.setp(axes, xlim=[0, 50.0], ylim=[ymin, ymax])

    def _get_autoscale_y_limits(self, axes, xmin, xmax):
        new_bottom = 1e9
        new_top = -1e9
        for i in range(self.number_of_axes):
            axis = axes[i]
            axis.set_xlim(left=xmin, right=xmax)
            xlim = axis.get_xlim()
            ylim = np.inf, -np.inf
            for line in axis.lines:
                x, y = line.get_data()
                start, stop = np.searchsorted(x, xlim)
                y_within_range = y[max(start - 1, 0):(stop + 1)]
                ylim = min(ylim[0], np.nanmin(y_within_range)), max(ylim[1], np.nanmax(y_within_range))

            new_bottom_i = ylim[0] * 1.3 if ylim[0] < 0.0 else ylim[0] * 0.7
            new_top_i = ylim[1] * 1.3 if ylim[1] > 0.0 else ylim[1] * 0.7
            if new_bottom_i < new_bottom:
                new_bottom = new_bottom_i
            if new_top_i > new_top:
                new_top = new_top_i

        return new_bottom, new_top

    def _remove_all_data_workspaces_from_plot(self, axes):
        workspaces_to_remove = self.plotted_workspaces
        for i, workspace in enumerate(workspaces_to_remove):
            self.remove_workspace_from_plot(workspace, axes)
        workspaces_to_remove = self.plotted_fit_workspaces
        for workspace in workspaces_to_remove:
            self.remove_workspace_from_plot(workspace, axes)

    def _update_legend(self, ax):
        handles, _ = ax.get_legend_handles_labels()
        if handles:
            ax.legend(prop=dict(size=legend_text_size))
        else:
            ax.legend("")
