from __future__ import (absolute_import, division, print_function)

from Muon.GUI.Common import thread_model
import Muon.GUI.Common.run_string_utils as runUtils
import Muon.GUI.Common.muon_file_utils as fileUtils


class LoadRunWidgetPresenter(object):

    def __init__(self, view, model):
        self._view = view
        self._model = model
        self._load_thread = None

        self._load_multiple_runs = False

        # TODO : Replace lines below with code to get the instrument
        self._instrument = "EMU"
        self._view.set_current_instrument(self._instrument)

        self._set_connections()

    def _set_connections(self):
        self._view.on_load_current_run_clicked(self.handle_load_current_run)
        self._view.on_increment_run_clicked(self.handle_increment_run)
        self._view.on_decrement_run_clicked(self.handle_decrement_run)
        self._view.on_run_edit_changed(self.handle_run_changed_by_user)

    def show(self):
        self._view.show()

    def cancel(self):
        if self._load_thread is not None:
            self._load_thread.cancel()

    def create_load_thread(self, algorithm):
        return thread_model.ThreadModel(algorithm)

    def get_current_instrument(self):
        return self._instrument

    def handle_run_changed_by_user(self):
        print("User has updated runs!")
        run_string = self._view.get_run_edit_text()
        run_list = runUtils.run_string_to_list(run_string)
        self._model.set_loaded_runs(run_list)
        self._view.set_run_edit_text(runUtils.run_list_to_string(run_list))
        # TODO : load the selected runs...

    def handle_load_current_run(self):
        self._view.disable_loading()
        print("Loading current run")
        # TODO : Add instrument directory to config service

        current_run_file = fileUtils.get_current_run_filename(self.get_current_instrument())

        self._load_thread = self.create_load_thread(self._model)
        self._load_thread.threadWrapperSetUp(self.disable_loading, self.handle_load_thread_finished_current_run)
        self._load_thread.loadData([current_run_file])
        self.clear_loaded_data()
        self._load_thread.start()

    def handle_load_thread_finished_current_run(self):
        self._view.enable_loading()

        run = self._model.get_run_list()[0]
        self._model.set_loaded_runs([run])
        self.set_run_edit_from_list([run])
        self._model.current_run = run

        self._load_thread.threadWrapperTearDown(self.disable_loading, self.handle_load_thread_finished_current_run)
        self._load_thread.deleteLater()
        self._load_thread = None

    def handle_load_thread_start(self, filenames):
        self._view.disable_loading()

        self._load_thread = self.create_load_thread(self._model)
        self._load_thread.threadWrapperSetUp(self.disable_loading, self.handle_load_thread_finished)
        print("Got to here 1")
        self._load_thread.loadData(filenames)
        self._load_thread.start()

    def handle_load_thread_finished(self):

        self._load_thread.threadWrapperTearDown(self.disable_loading, self.handle_load_thread_finished)
        self._load_thread.deleteLater()
        self._load_thread = None

        run_list = self._model.get_run_list()
        print("Got to here 2")
        self._model.set_loaded_runs(run_list)
        self.set_run_edit_from_list(run_list)

        self._view.enable_loading()

    def disable_loading(self):
        self._view.disable_load_buttons()

    def enable_loading(self):
        self._view.enable_load_buttons()

    def set_run_edit_from_list(self, run_list):
        run_string = runUtils.run_list_to_string(run_list)
        self._view.set_run_edit_text(run_string)
        self._model.set_loaded_runs(run_list)

    def get_current_runs(self):
        return self._model._loaded_runs

    def handle_increment_run(self):
        run_list = self.get_current_runs()
        if run_list is None or len(run_list) == 0:
            return
        if len(run_list) == 1:
            run_list = [runUtils.increment_run(run_list[0])]
        else:
            run_list = runUtils.increment_run_list(run_list)

        # TODO : implement loading of new run
        new_run = max(run_list)

        if self._load_multiple_runs == False:
            self.clear_loaded_data()

        file_name = fileUtils.file_path_for_instrument_and_run(self.get_current_instrument(), new_run)
        print(file_name)
        self.handle_load_thread_start([file_name])

    def handle_decrement_run(self):
        run_list = self.get_current_runs()
        if run_list is None or len(run_list) == 0:
            return
        if len(run_list) == 1:
            run_list = [runUtils.decrement_run(run_list[0])]
        else:
            run_list = runUtils.decrement_run_list(run_list)

        # TODO : implement loading of new run
        new_run = min(run_list)

        if self._load_multiple_runs == False:
            self.clear_loaded_data()

        instr = self.get_current_instrument()
        file_name = fileUtils.file_path_for_instrument_and_run(instr, new_run)
        print(file_name)
        self.handle_load_thread_start([file_name])

    def set_new_instrument(self):
        # TODO : implement
        pass

    def clear_loaded_data(self):
        self._view.set_run_edit_text("")
        self._model.clear_loaded_data()

    def enable_multiple_files(self, enabled):
        self._load_multiple_runs = enabled

    def get_loaded_filenames(self):
        return self._model.loaded_filenames

    @property
    def workspaces(self):
        return self._model.loaded_workspaces

    @property
    def runs(self):
        return self._model.loaded_runs

    def update_model_and_view(self, run_list):
        print("ARG!")
        self.clear_loaded_data()
        self.set_run_edit_from_list(run_list)