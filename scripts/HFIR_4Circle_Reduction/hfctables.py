#pylint: disable=W0403,C0103,R0901,R0904
import numpy
import NTableWidget as tableBase

# UB peak information table
Peak_Integration_Table_Setup = [('Scan', 'int'),
                                ('Pt', 'str'),
                                ('Merged Workspace', 'str'),
                                ('H', 'float'),
                                ('K', 'float'),
                                ('L', 'float'),
                                ('Q_x', 'float'),
                                ('Q_y', 'float'),
                                ('Q_z', 'float'),
                                ('Intensity', 'float'),
                                ('Selected', 'checkbox')]


class IntegratePeaksTableWidget(tableBase.NTableWidget):
    """
    Extended table widget for peak integration
    """
    def __init__(self, parent):
        """
        :param parent:
        """
        tableBase.NTableWidget.__init__(self, parent)

        return

    def append_scan(self, info_tuple):
        """
        out_ws, target_frame, exp_no, scan_no, None
        :param info_tuple:
        :return: 2-tuple as boolean and error message
        """
        out_ws_name = info_tuple[0]
        # target_frame = info_tuple[1]
        # exp_no = info_tuple[2]
        scan_no = info_tuple[3]

        status, msg = self.append_row([scan_no, '', out_ws_name, 0., 0., 0., 0., 0., 0., 0, False])
        if status is False:
            msg = 'Unable to append row to peak integration table due to %s' % msg

        return status, msg

    def get_md_ws_name(self, row_index):
        """ Get MD workspace name
        :param row_index:
        :return:
        """
        j_col = Peak_Integration_Table_Setup.index(('Merged Workspace', 'str'))

        return self.get_cell_value(row_index, j_col)


    def get_scan_number(self, row_index):
        """ Get scan number of the row
        :param row_index:
        :return:
        """
        j_col = Peak_Integration_Table_Setup.index(('Scan', 'int'))

        return self.get_cell_value(row_index, j_col)

    def setup(self):
        """
        Init setup
        :return:
        """
        self.init_setup(Peak_Integration_Table_Setup)

        return

    def set_hkl(self, row_index, vec_hkl):
        """
        Set up HKL value to table
        :param row_index:
        :param vec_hkl:
        :return:
        """
        # check requirement
        assert isinstance(vec_hkl, list), 'Input HKL must be a list but not %s.' % str(type(vec_hkl))
        assert len(vec_hkl, 3)

        # locate
        index_h = Peak_Integration_Table_Setup.index(('H', 'float'))
        for j in xrange(3):
            col_index = j + index_h
            self.update_cell_value(row_index, col_index, vec_hkl[j])

        return

    def set_q(self, row_index, vec_q):
        """
        Set up Q value
        """
        assert len(vec_q, 3)

        # locate
        index_q_x = Peak_Integration_Table_Setup.index(('Q_x', 'float'))
        for j in xrange(3):
            col_index = j + index_q_x
            self.update_cell_value(row_index, col_index, vec_q[j])

        return


class UBMatrixTable(tableBase.NTableWidget):
    """
    Extended table for UB matrix
    """
    def __init__(self, parent):
        """

        :param parent:
        :return:
        """
        tableBase.NTableWidget.__init__(self, parent)

        # Matrix
        self._matrix = numpy.ndarray((3, 3), float)
        for i in xrange(3):
            for j in xrange(3):
                self._matrix[i][j] = 0.

        return

    def _set_to_table(self):
        """
        Set values in holder '_matrix' to TableWidget
        :return:
        """
        for i_row in xrange(3):
            for j_col in xrange(3):
                self.update_cell_value(i_row, j_col, self._matrix[i_row][j_col])

        return

    def get_matrix(self):
        """
        Get the copy of the matrix
        :return:
        """
        # print '[DB] MatrixTable: _Matrix = ', self._matrix
        return self._matrix.copy()

    def set_from_list(self, element_array):
        """
        Set table value including holder and QTable from a 1D numpy array
        :param element_array:
        :return:
        """
        # Check
        assert isinstance(element_array, list)
        assert len(element_array) == 9

        # Set value
        i_array = 0
        for i in xrange(3):
            for j in xrange(3):
                self._matrix[i][j] = element_array[i_array]
                i_array += 1

        # Set to table
        self._set_to_table()

        return

    def set_from_matrix(self, matrix):
        """
        Set value to both holder and QTable from a numpy 3 x 3 matrix
        :param matrix:
        :return:
        """
        # Check
        assert isinstance(matrix, numpy.ndarray)
        assert matrix.shape == (3, 3)

        for i in xrange(3):
            for j in xrange(3):
                self._matrix[i][j] = matrix[i][j]

        self._set_to_table()

        return

    def setup(self):
        """
        Init setup
        :return:
        """
        # self.init_size(3, 3)

        for i in xrange(3):
            for j in xrange(3):
                self.set_value_cell(i, j)

        self._set_to_table()

        return


# UB peak information table
UB_Peak_Table_Setup = [('Scan', 'int'),
                       ('Pt', 'int'),
                       ('H', 'float'),
                       ('K', 'float'),
                       ('L', 'float'),
                       ('Q_x', 'float'),
                       ('Q_y', 'float'),
                       ('Q_z', 'float'),
                       ('Selected', 'checkbox'),
                       ('m1', 'float'),
                       ('Error', 'float')]


class UBMatrixPeakTable(tableBase.NTableWidget):
    """
    Extended table for peaks used to calculate UB matrix
    """
    def __init__(self, parent):
        """

        :param parent:
        :return:
        """
        tableBase.NTableWidget.__init__(self, parent)

        return

    def get_exp_info(self, row_index):
        """
        Get experiment information from a row
        :param row_index:
        :return: scan number, pt number
        """
        assert isinstance(row_index, int)

        scan_number = self.get_cell_value(row_index, 0)
        assert isinstance(scan_number, int)
        pt_number = self.get_cell_value(row_index, 1)
        assert isinstance(pt_number, int)

        return scan_number, pt_number

    def get_hkl(self, row_index):
        """
        Get reflection's miller index
        :param row_index:
        :return:
        """
        assert isinstance(row_index, int)

        m_h = self.get_cell_value(row_index, 2)
        m_k = self.get_cell_value(row_index, 3)
        m_l = self.get_cell_value(row_index, 4)

        assert isinstance(m_h, float)
        assert isinstance(m_k, float)
        assert isinstance(m_l, float)

        return m_h, m_k, m_l

    def get_scan_pt(self, row_number):
        """
        Get Scan and Pt from a row
        :param row_number:
        :return:
        """
        scan_number = self.get_cell_value(row_number, 0)
        pt_number = self.get_cell_value(row_number, 1)

        return scan_number, pt_number

    def is_selected(self, row_index):
        """

        :return:
        """
        if row_index < 0 or row_index >= self.rowCount():
            raise IndexError('Input row number %d is out of range [0, %d)' % (row_index, self.rowCount()))

        col_index = UB_Peak_Table_Setup.index(('Selected', 'checkbox'))

        return self.get_cell_value(row_index, col_index)

    def setup(self):
        """
        Init setup
        :return:
        """
        self.init_setup(UB_Peak_Table_Setup)

        return

    def set_hkl(self, i_row, hkl, error=None):
        """
        Set HKL to table
        :param irow:
        :param hkl:
        """
        # Check
        assert isinstance(i_row, int)
        assert isinstance(hkl, list)

        i_col_h = UB_Peak_Table_Setup.index(('H', 'float'))
        i_col_k = UB_Peak_Table_Setup.index(('K', 'float'))
        i_col_l = UB_Peak_Table_Setup.index(('L', 'float'))

        self.update_cell_value(i_row, i_col_h, hkl[0])
        self.update_cell_value(i_row, i_col_k, hkl[1])
        self.update_cell_value(i_row, i_col_l, hkl[2])

        if error is not None:
            i_col_error = UB_Peak_Table_Setup.index(('Error', 'float'))
            self.update_cell_value(i_row, i_col_error, error)

        return

    def update_hkl(self, i_row, h, k, l):
        """ Update HKL value
        """
        self.update_cell_value(i_row, 2, h)
        self.update_cell_value(i_row, 3, k)
        self.update_cell_value(i_row, 4, l)

        return


# Processing status table
Process_Table_Setup = [('Scan', 'int'),
                       ('Number Pt', 'int'),
                       ('Status', 'str'),
                       ('Merged Workspace', 'str'),
                       ('Group Name', 'str'),
                       ('Select', 'checkbox')]


class ProcessTableWidget(tableBase.NTableWidget):
    """
    Extended table for peaks used to calculate UB matrix
    """
    def __init__(self, parent):
        """

        :param parent:
        :return:
        """
        tableBase.NTableWidget.__init__(self, parent)

        return

    def append_scans(self, scans):
        """ Append rows
        :param scans:
        :return:
        """
        # Check
        assert isinstance(scans, list)

        # Append rows
        for scan in scans:
            row_value_list = [scan, 0, 'In Queue', '', '', False]
            status, err = self.append_row(row_value_list)
            if status is False:
                raise RuntimeError(err)

        return

    def get_merged_ws_name(self, i_row):
        """
        Get ...
        :param i_row:
        :return:
        """
        j_col_merged = Process_Table_Setup.index(('Merged Workspace', 'str'))

        return self.get_cell_value(i_row, j_col_merged)

    def get_scan_list(self):
        """ Get list of scans to merge from table
        :return: list of 2-tuples (scan number, row number)
        """
        scan_list = list()
        num_rows = self.rowCount()
        j_select = Process_Table_Setup.index(('Select', 'checkbox'))
        j_scan = Process_Table_Setup.index(('Scan', 'int'))

        for i_row in xrange(num_rows):
            if self.get_cell_value(i_row, j_select) is True:
                scan_num = self.get_cell_value(i_row, j_scan)
                scan_list.append((scan_num, i_row))

        return scan_list

    def setup(self):
        """
        Init setup
        :return:
        """
        self.init_setup(Process_Table_Setup)

        return

    def set_scan_pt(self, scan_no, pt_list):
        """
        :param scan_no:
        :param pt_list:
        :return:
        """
        # Check
        assert isinstance(scan_no, int)

        num_rows = self.rowCount()
        set_done = False
        for i_row in xrange(num_rows):
            tmp_scan_no = self.get_cell_value(i_row, 0)
            if scan_no == tmp_scan_no:
                self.update_cell_value(i_row, 1, len(pt_list))
                set_done = True
                break
        # END-FOR

        if set_done is False:
            return 'Unable to find scan %d in table.' % scan_no

        return ''

    def set_pt_by_row(self, row_number, pt_list):
        """
        :param scan_no:
        :param pt_list:
        :return:
        """
        # Check
        assert isinstance(row_number, int)
        assert isinstance(pt_list, list)

        j_pt = Process_Table_Setup.index(('Number Pt', 'int'))
        self.update_cell_value(row_number, j_pt, len(pt_list))

        return ''

    def set_status(self, scan_no, status):
        """
        Set the status for merging scan to QTable
        :param status:
        :return:
        """
        # Check
        assert isinstance(scan_no, int)

        num_rows = self.rowCount()
        set_done = False
        for i_row in xrange(num_rows):
            tmp_scan_no = self.get_cell_value(i_row, 0)
            if scan_no == tmp_scan_no:
                self.update_cell_value(i_row, 2, status)
                set_done = True
                break
        # END-FOR

        if set_done is False:
            return 'Unable to find scan %d in table.' % scan_no

        return ''

    def set_status_by_row(self, row_number, status):
        """
        Set status to a specified row according to row number
        :param row_number:
        :param status:
        :return:
        """
        # Check
        assert isinstance(row_number, int)
        assert isinstance(status, str)

        # Set
        i_status = Process_Table_Setup.index(('Status', 'str'))
        self.update_cell_value(row_number, i_status, status)

        return

    def set_ws_names(self, scan_num, merged_md_name, ws_group_name):
        """
        Set the output workspace and workspace group's names to QTable
        :param merged_md_name:
        :param ws_group_name:
        :return:
        """
        # Check
        assert isinstance(scan_num, int)
        assert isinstance(merged_md_name, str) or merged_md_name is None
        assert isinstance(ws_group_name, str) or ws_group_name is None

        num_rows = self.rowCount()
        set_done = False
        for i_row in xrange(num_rows):
            tmp_scan_no = self.get_cell_value(i_row, 0)
            if scan_num == tmp_scan_no:
                if merged_md_name is not None:
                    self.update_cell_value(i_row, 3, merged_md_name)
                if ws_group_name is not None:
                    self.update_cell_value(i_row, 4, ws_group_name)
                set_done = True
                break
        # END-FOR

        if set_done is False:
            return 'Unable to find scan %d in table.' % scan_num

        return

    def set_ws_names_by_row(self, row_number, merged_md_name, ws_group_name):
        """
        Set the workspaces' names to this table
        :param row_number:
        :param merged_md_name:
        :param ws_group_name:
        :return:
        """
        # Check
        assert isinstance(row_number, int)
        assert isinstance(merged_md_name, str) or merged_md_name is None
        assert isinstance(ws_group_name, str) or ws_group_name is None

        j_ws_name = Process_Table_Setup.index(('Merged Workspace', 'str'))
        j_group_name = Process_Table_Setup.index(('Group Name', 'str'))

        if merged_md_name is not None:
            self.update_cell_value(row_number, j_ws_name, merged_md_name)
        if ws_group_name is not None:
            self.update_cell_value(row_number, j_group_name, ws_group_name)

        return


class ScanSurveyTable(tableBase.NTableWidget):
    """
    Extended table widget for peak integration
    """
    Table_Setup = [('Scan', 'int'),
                   ('Max Counts Pt', 'int'),
                   ('Max Counts', 'double'),
                   ('H', 'float'),
                   ('K', 'float'),
                   ('L', 'float'),
                   ('Selected', 'checkbox')]

    def __init__(self, parent):
        """
        :param parent:
        """
        tableBase.NTableWidget.__init__(self, parent)
        self.set_status_column_name('Selected')

        self._myScanInfoDict = dict()
        self._myScanList = list()

        return

    def get_selected_run_surveyed(self):
        """

        :return:
        """
        # TODO/NOW/1st: documents + check

        row_index_list = self.get_selected_rows(True)
        assert len(row_index_list) > 0

        row_index = row_index_list[0]
        scan_number = self.get_cell_value(row_index, 0)
        pt_number = self.get_cell_value(row_index, 1)

        # clear selection
        # TODO/NOW/1st - Implement!

        return scan_number, pt_number

    def show_reflections(self, num_rows):
        """
        :param num_rows:
        :return:
        """
        assert isinstance(num_rows, int)
        assert num_rows > 0
        assert len(self._myScanList) > 0

        for i_ref in xrange(min(num_rows, len(self._myScanList))):
            # get counts
            counts = self._myScanList[i_ref]
            dict_value = self._myScanInfoDict[counts]
            if isinstance(dict_value, tuple):
                row_items = list(dict_value)
                row_items.insert(2, counts)
                row_items.append(False)
                print 'Appending row:', row_items
                self.append_row(row_items)
            elif isinstance(dict_value, list):
                for reflection in dict_value:
                    row_items = list(reflection)
                    row_items.insert(2, counts)
                    row_items.append(False)
                    print 'Appending row-2:', row_items
                    self.append_row(row_items)
            else:
                err_msg = 'Type %s is not a supported value type of counts dictionary.' % str(type(dict_value))
                raise RuntimeError(err_msg)
        # END-FOR

        return

    def set_survey_result(self, survey_scan_dict):
        """

        :param survey_scan_dict:
        :return:
        """
        # check
        assert isinstance(survey_scan_dict, dict)

        # Sort and add to table
        scan_list = survey_scan_dict.keys()
        scan_list.sort(reverse=True)

        self._myScanInfoDict = survey_scan_dict
        self._myScanList = scan_list

        return

    def setup(self):
        """
        Init setup
        :return:
        """
        self.init_setup(ScanSurveyTable.Table_Setup)

        return
