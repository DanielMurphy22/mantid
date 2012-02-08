""" Test script for running python commands within MantidPlot.
This will test the interface to SliceViewer widgets.

Note: the SliceViewerPythonInterfaceTest.py offers
more tests of specific functions. This module
tests (primarily) the plotSlice() helper methods that is available
only within mantidplot.py
"""
import sys
import os
import unittest
import mantidplottests
from mantidplottests import *
import time

CreateMDWorkspace(Dimensions='3',Extents='0,10,0,10,0,10',Names='x,y,z',Units='m,m,m',SplitInto='5',MaxRecursionDepth='20',OutputWorkspace='mdw')
FakeMDEventData(InputWorkspace="mdw",  UniformParams="1e5")
FakeMDEventData(InputWorkspace="mdw",  PeakParams="1e4, 2,4,6, 1.5")
BinMD(InputWorkspace="mdw", OutputWorkspace="uniform",  AxisAligned=True, AlignedDimX="x,0,10,30", AlignedDimY="y,0,10,30", AlignedDimZ="z,0,10,30", IterateEvents="1", Parallel="0")
CreateMDWorkspace(Dimensions='3',Extents='0,10,0,10,0,10',Names='x,y,z',Units='m,m,m',SplitInto='5',MaxRecursionDepth='20',OutputWorkspace='empty')
CreateMDWorkspace(Dimensions='4',Extents='0,10,0,10,0,10,0,10',Names='x,y,z,e',Units='m,m,m,meV',SplitInto='5',MaxRecursionDepth='20',OutputWorkspace='md4')

class MantidPlotSliceViewerTest(unittest.TestCase):
    
    def setUp(self):
        pass

    def tearDown(self):
        closeAllSliceViewers()
        pass
    
    def test_plotSlice(self):
        """ Basic plotSlice() usage """
        svw = plotSlice('uniform')
        self.assertEqual(svw.getSlicer().getWorkspaceName(), "uniform")
        
    def test_mouseMove(self):
        """ Move the mouse over the slice viewer """
        svw = plotSlice('uniform')
        svw.setSlicePoint(2, 2.5)
        if qtest:
            sv = svw._getHeldObject()
            QtCore.QCoreApplication.processEvents()
            QTest.mouseMove(sv)
            QtCore.QCoreApplication.processEvents()
        screenshot(svw, "SliceViewer", "SliceViewer with mouse at center, showing coordinates.")
        
    def test_plotSlice_empty(self):
        """ Plot slice on an empty workspace """
        svw = plotSlice('empty')
        self.assertEqual(svw.getSlicer().getWorkspaceName(), "empty")
        
        
    def test_closingWindowIsSafe(self):
        svw = plotSlice('uniform', label='closing!')
        svw.close()
        
    def test_methods_pass_through(self):
        """ Methods called on SliceViewerWindow pass-through to the SliceViewer widget"""
        svw = plotSlice('uniform')
        svw.setSlicePoint(0, 2.5)
        self.assertAlmostEqual(svw.getSlicePoint(0), 2.5, 3) 
        svw.setXYDim("z", "x")
        self.assertEqual(svw.getDimX(), 2) 
        self.assertEqual(svw.getDimY(), 0) 
        
    def test_plot4D_workspace(self):
        svw = plotSlice('md4')
        svw.setSlicePoint(2, 2.5)
        svw.setSlicePoint(3, 7.5)
        self.assertAlmostEqual(svw.getSlicePoint(2), 2.5, 3) 
        self.assertAlmostEqual(svw.getSlicePoint(3), 7.5, 3) 
        screenshot(svw, "SliceViewer_4D", "SliceViewer open to a 4D workspace; z=2.5, e=7.5.")
        svw.setXYDim("z", "e")
        self.assertEqual(svw.getDimX(), 2) 
        self.assertEqual(svw.getDimY(), 3) 

    def test_plotSlice_arguments(self):
        """ Pass arguments to plotSlice """
        svw = plotSlice('uniform', label='test_label', xydim=[1,2], 
            slicepoint=[2.5, 0, 0], colormin=20, colormax=5000, colorscalelog=True,
            limits=[2, 8, 3, 9])
        self.assertEqual(svw.getLabel(), "test_label") 
        self.assertEqual(svw.getDimX(), 1) 
        self.assertEqual(svw.getDimY(), 2) 
        self.assertAlmostEqual(svw.getSlicePoint(0), 2.5, 3) 
        self.assertAlmostEqual(svw.getColorScaleMin(), 20, 2) 
        self.assertAlmostEqual(svw.getColorScaleMax(), 5000, 2) 
        assert svw.getColorScaleLog() 
        self.assertEqual(svw.getXLimits(), [2,8]) 
        self.assertEqual(svw.getYLimits(), [3,9]) 

    def test_plotSlice_arguments2(self):
        """ Another way to pass xydim """
        svw = plotSlice('uniform', xydim=["y", "z"])
        self.assertEqual(svw.getDimX(), 1) 
        self.assertEqual(svw.getDimY(), 2) 

    def test_getSliceViewer(self):
        """ Retrieving an open SliceViewer """
        svw1 = plotSlice('uniform')
        svw2 = getSliceViewer('uniform')
        assert svw2 is not None
        self.assertEqual(svw2.getSlicer().getWorkspaceName(), "uniform") 

    def test_getSliceViewer_failure(self):
        """ Retrieving an open SliceViewer, cases where it fails """
        self.assertRaises(Exception, getSliceViewer, 'nonexistent')
        svw = plotSlice('uniform', label='alabel')
        self.assertRaises(Exception, getSliceViewer, 'uniform', 'different_label')
        
    def test_saveImage(self):
        """ Save the rendered image """
        svw = plotSlice('uniform')
        svw.setSlicePoint(2,6.0)
        dest = get_screenshot_dir()
        if not dest is None:
            filename = "SliceViewerSaveImage"
            svw.saveImage(os.path.join(dest, filename+".png") )
            # Add to the HTML report
            screenshot(None, filename, "SliceViewer: result of saveImage(). Should be only the 2D plot with a color bar (no GUI elements)",
                       png_exists=True)
        
    def test_showLine(self):
        svw = plotSlice('uniform')
        svw.setSlicePoint(2,6.0)
        liner = svw.showLine([1,1], [7,9], width=0.88, num_bins=200)
        self.assertTrue( not (liner is None), "Returns a LineViewer proxy object")
        # Plot the X units
        liner.setPlotAxis(2);
        liner.apply()
        # Check that the values are there
        self.assertEqual(liner.getNumBins(), 200)
        # Length of 10 with 200 bins = 0.05 width
        self.assertAlmostEqual(liner.getBinWidth(), 0.05, 3)
        # Width was set
        self.assertAlmostEqual(liner.getPlanarWidth(), 0.88, 3)
        screenshot(svw, "SliceViewer_and_LineViewer", "SliceViewer with LineViewer open, showing line overlay and integrated line.")
        # Now turn it off
        svw.toggleLineMode(False)
        self.assertFalse( liner.isVisible(), "LineViewer was hidden")
        
        
        
# Run the unit tests
mantidplottests.runTests(MantidPlotSliceViewerTest) 

