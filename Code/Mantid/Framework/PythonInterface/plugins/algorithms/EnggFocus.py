#pylint: disable=no-init
from mantid.kernel import *
from mantid.api import *
import mantid.simpleapi as sapi

import EnggUtils

class EnggFocus(PythonAlgorithm):
    INDICES_PROP_NAME = 'SpectrumNumbers'

    def category(self):
        return "Diffraction\\Engineering;PythonAlgorithms"

    def name(self):
        return "EnggFocus"

    def summary(self):
        return "Focuses a run by summing up all the spectra into a single one."

    def PyInit(self):
        self.declareProperty(MatrixWorkspaceProperty("InputWorkspace", "", Direction.Input),
                             "Workspace with the run to focus.")

        self.declareProperty(MatrixWorkspaceProperty("OutputWorkspace", "", Direction.Output),
                             "A workspace with focussed data")

        self.declareProperty(MatrixWorkspaceProperty("VanadiumWorkspace", "", Direction.Input,
                                                     PropertyMode.Optional),
                             doc = 'Workspace with the Vanadium (correction and calibration) run. '
                             'Alternatively, when the Vanadium run has been already processed, '
                             'the properties can be used')

        self.declareProperty(ITableWorkspaceProperty('VanIntegrationWorkspace', '',
                                                     Direction.Input, PropertyMode.Optional),
                             doc = 'Results of integrating the spectra of a Vanadium run, with one column '
                             '(integration result) and one row per spectrum. This can be used in '
                             'combination with OutVanadiumCurveFits from a previous execution and '
                             'VanadiumWorkspace to provide pre-calculated values for Vanadium correction.')

        self.declareProperty(MatrixWorkspaceProperty('VanCurvesWorkspace', '', Direction.Input,
                                                     PropertyMode.Optional),
                             doc = 'A workspace2D with the fitting workspaces corresponding to '
                             'the instrument banks. This workspace has three spectra per bank, as produced '
                             'by the algorithm Fit. This is meant to be used as an alternative input '
                             'VanadiumWorkspace for testing and performance reasons. If not given, no '
                             'workspace is generated.')

        self.declareProperty("Bank", '', StringListValidator(EnggUtils.ENGINX_BANKS),
                             direction=Direction.Input,
                             doc = "Which bank to focus: It can be specified as 1 or 2, or "
                             "equivalently, North or South. See also " + self.INDICES_PROP_NAME + " "
                             "for a more flexible alternative to select specific detectors")

        self.declareProperty(self.INDICES_PROP_NAME, '', direction=Direction.Input,
                             doc = 'Sets the spectrum numbers for the detectors '
                             'that should be considered in the focussing operation (all others will be '
                             'ignored). This option cannot be used together with Bank, as they overlap. '
                             'You can give multiple ranges, for example: "0-99", or "0-9, 50-59, 100-109".')

        self.declareProperty(ITableWorkspaceProperty('DetectorPositions', '', Direction.Input,
                                                     PropertyMode.Optional),
                             "Calibrated detector positions. If not specified, default ones are used.")


    def PyExec(self):
        # Get the run workspace
        ws = self.getProperty('InputWorkspace').value

        # Get spectra indices either from bank or direct list of indices, checking for errors
        bank = self.getProperty('Bank').value
        spectra = self.getProperty(self.INDICES_PROP_NAME).value
        indices = EnggUtils.getWsIndicesFromInProperties(ws, bank, spectra)

    	# Leave the data for the bank we are interested in only
        ws = EnggUtils.cropData(self, ws, indices)

        # Leave data for the same bank in the vanadium workspace too
        vanWS = self.getProperty('VanadiumWorkspace').value
        vanIntegWS = self.getProperty('VanIntegrationWorkspace').value
        vanCurvesWS = self.getProperty('VanCurvesWorkspace').value
        EnggUtils.applyVanadiumCorrections(self, ws, indices, vanWS, vanIntegWS, vanCurvesWS)

    	# Apply calibration
        detPos = self.getProperty("DetectorPositions").value
        if detPos:
            self._applyCalibration(ws, detPos)

    	# Convert to dSpacing
        ws = EnggUtils.convertToDSpacing(self, ws)

    	# Sum the values
        ws = EnggUtils.sumSpectra(self, ws)

    	# Convert back to time of flight
        ws = EnggUtils.convertToToF(self, ws)

    	# OpenGenie displays distributions instead of pure counts (this is done implicitly when
    	# converting units), so I guess that's what users will expect
        self._convertToDistr(ws)

        self.setProperty("OutputWorkspace", ws)

    def _applyCalibration(self, ws, detPos):
        """
        Refines the detector positions using the result of calibration (if one is specified).

        @param ws :: workspace to apply the calibration (on its instrument)
        @param detPos :: detector positions (as a table of positions, one row per detector)
    	"""
        alg = self.createChildAlgorithm('ApplyCalibration')
        alg.setProperty('Workspace', ws)
        alg.setProperty('PositionTable', detPos)
        alg.execute()

    def _convertToDistr(self, ws):
        """
        Convert workspace to distribution

        @param ws :: workspace, which is modified/converted in place
    	"""
        alg = self.createChildAlgorithm('ConvertToDistribution')
        alg.setProperty('Workspace', ws)
        alg.execute()

AlgorithmFactory.subscribe(EnggFocus)
