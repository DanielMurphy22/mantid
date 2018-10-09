from __future__ import (absolute_import, division, print_function)
from qtpy import QtWidgets
from PyQt4 import QtGui

import mantid.simpleapi as simpleapi
import mantid
import mantid.api as api
from mantid.api import ITableWorkspace

from Muon.GUI.Common.home_instrument_widget.home_instrument_widget_model import InstrumentWidgetModel
from Muon.GUI.Common.home_instrument_widget.home_instrument_widget_view import InstrumentWidgetView
from Muon.GUI.Common.home_instrument_widget.home_instrument_widget_presenter import InstrumentWidgetPresenter

from Muon.GUI.Common.home_grouping_widget.home_grouping_widget_model import HomeGroupingWidgetModel
from Muon.GUI.Common.home_grouping_widget.home_grouping_widget_view import HomeGroupingWidgetView
from Muon.GUI.Common.home_grouping_widget.home_grouping_widget_presenter import HomeGroupingWidgetPresenter

from Muon.GUI.Common.home_plot_widget.home_plot_widget_model import HomePlotWidgetModel
from Muon.GUI.Common.home_plot_widget.home_plot_widget_view import HomePlotWidgetView
from Muon.GUI.Common.home_plot_widget.home_plot_widget_presenter import HomePlotWidgetPresenter

from Muon.GUI.Common.home_runinfo_widget.home_runinfo_widget_model import HomeRunInfoWidgetModel
from Muon.GUI.Common.home_runinfo_widget.home_runinfo_widget_view import HomeRunInfoWidgetView
from Muon.GUI.Common.home_runinfo_widget.home_runinfo_widget_presenter import HomeRunInfoWidgetPresenter

from Muon.GUI.Common.home_tab.home_tab_model import HomeTabModel
from Muon.GUI.Common.home_tab.home_tab_view import HomeTabView
from Muon.GUI.Common.home_tab.home_tab_presenter import HomeTabPresenter

from Muon.GUI.Common.muon_context import MuonContext

if __name__ == "__main__":
    # add dead time tables
    correctTable = simpleapi.CreateEmptyTableWorkspace()
    incorrectTable = simpleapi.CreateEmptyTableWorkspace()

    correctTable.addColumn("int", "spectrum",0)
    correctTable.addColumn("float", "dead-time",0)
    for i in range(96):
        correctTable.addRow([i, 0.01])

    print(mantid.api.AnalysisDataServiceImpl.Instance().getObjectNames())


    import sys

    context = MuonContext()

    app = QtWidgets.QApplication(sys.argv)

    obj = QtGui.QWidget()

    inst_view = InstrumentWidgetView(obj)
    grp_view = HomeGroupingWidgetView(obj)
    plot_view = HomePlotWidgetView(obj)
    run_info_view = HomeRunInfoWidgetView(obj)

    ui = InstrumentWidgetPresenter(inst_view, InstrumentWidgetModel(muon_data=context))
    #ui.show()



    ui2 = HomeGroupingWidgetPresenter(grp_view, HomeGroupingWidgetModel(muon_data = context))
    #ui2.show()

    ui3 = HomePlotWidgetPresenter(plot_view, HomePlotWidgetModel())
    #ui3.show()

    ui4 = HomeRunInfoWidgetPresenter(run_info_view, HomeRunInfoWidgetModel(muon_data = context))
    #ui4.show()



    tab_view = HomeTabView(parent=None,
                           instrument_widget=inst_view,
                           grouping_widget=grp_view,
                           plot_widget=plot_view,
                           run_info_widget=run_info_view)
    tab_model = HomeTabModel(muon_data = context)

    ui_tab = HomeTabPresenter(tab_view, tab_model, subwidgets=[ui, ui2, ui3, ui4])

    ui.instrumentNotifier.add_subscriber(ui_tab.instrumentObserver)

    ui_tab.show()

    sys.exit(app.exec_())
