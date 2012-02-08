import unittest
from testhelpers import run_algorithm
from mantid.api import AnalysisDataService, MatrixWorkspace, Workspace
from mantid import mtd

class AnalysisDataServiceTest(unittest.TestCase):
  
    def test_len_returns_correct_value(self):
        self.assertEquals(len(AnalysisDataService.Instance()), 0)
        
    def test_mtd_is_same_object_type_as_analysis_data_service(self):
        self.assertTrue(isinstance(AnalysisDataService.Instance(), AnalysisDataService))
        self.assertTrue(isinstance(mtd, AnalysisDataService))

    def test_retrieval_of_non_existent_data_raises_KeyError(self):
        try:
            AnalysisDataService.Instance()['NotHere']
            self.fail('AnalysisDataService did not throw when object does not exist')
        except KeyError:
            pass

    def _run_createws(self, wsname):
        """
            Run create workspace storing the output in the named workspace
        """
        data = [1.0,2.0,3.0]
        run_algorithm('CreateWorkspace',OutputWorkspace=wsname,DataX=data,DataY=data,NSpec=1,UnitX='Wavelength')
        
        
    def test_len_increases_when_item_added(self):
        wsname = 'ADSTest_test_len_increases_when_item_added'
        current_len = len(AnalysisDataService.Instance())
        self._run_createws(wsname)
        self.assertEquals(len(AnalysisDataService.Instance()), current_len + 1)
        # Remove to clean the test up
        AnalysisDataService.Instance().remove(wsname)
        
    def test_len_decreases_when_item_removed(self):
        wsname = 'ADSTest_test_len_decreases_when_item_removed'
        self._run_createws(wsname)
        current_len = len(AnalysisDataService.Instance())
        # Remove to clean the test up
        del AnalysisDataService.Instance()[wsname]
        self.assertEquals(len(AnalysisDataService.Instance()), current_len - 1)
    
    def test_key_operator_does_same_as_retrieve(self):
        wsname = 'ADSTest_test_key_operator_does_same_as_retrieve'
        self._run_createws(wsname)
        ws_from_op = AnalysisDataService.Instance()[wsname]
        ws_from_method = AnalysisDataService.Instance().retrieve(wsname)
        
        # Type check
        self.assertTrue(isinstance(ws_from_op, MatrixWorkspace))
        self.assertTrue(isinstance(ws_from_method, MatrixWorkspace))

        self.assertNotEquals(ws_from_op.name(), '')
        self.assertEquals(ws_from_op.name(), ws_from_method.name())
        self.assertEquals(ws_from_op.getMemorySize(), ws_from_method.getMemorySize())

        # Remove to clean the test up
        AnalysisDataService.Instance().remove(wsname)

    def test_removing_item_invalidates_extracted_handles(self):
        # If a reference to a DataItem has been extracted from the ADS
        # and it is then removed. The extracted handle should no longer
        # be able to access the DataItem
        wsname = 'ADSTest_test_removing_item_invalidates_extracted_handles'
        self._run_createws(wsname)
        ws_handle = AnalysisDataService.Instance()[wsname]
        succeeded = False
        try:
            ws_handle.id() # Should be okay
            succeeded = True
        except RuntimeError:
            pass
        self.assertTrue(succeeded, "DataItem handle should be valid and allow function calls")
        AnalysisDataService.Instance().remove(wsname)
        self.assertRaises(RuntimeError, ws_handle.id)
        
    def test_importAll_exists_as_member(self):
        self.assertTrue(hasattr(AnalysisDataService.Instance(), "importAll"))
        
    def test_importAll_creates_variable_in_current_global_dict_pointing_to_each_workspace(self):
        obj_names = mtd.getObjectNames()
        extra_names = ["ADSTest_test_1", "ADSTest_test_2", "ADSTest_test_3"]
        for name in extra_names:
            self._run_createws(name)
        obj_names += extra_names
        
        # Check no names are in globals
        for name in obj_names:
            self.assertFalse(name in locals())

        # Pull in variables
        mtd.importAll()
        # Are they in the local namespace
        for name in obj_names:
            self.assertTrue(name in locals())
        
        # Clean up
        for name in obj_names:
            try:
                del locals()[name]
            except KeyError:
                pass
        for name in extra_names:
            mtd.remove(name)

if __name__ == '__main__':
    unittest.main()
