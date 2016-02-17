#ifndef FINDPEAKSTEST_H_
#define FINDPEAKSTEST_H_

#include <cxxtest/TestSuite.h>
#include "MantidTestHelpers/WorkspaceCreationHelper.h"

#include "MantidAlgorithms/FindPeaks.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidDataObjects/TableWorkspace.h"
#include "MantidAPI/MatrixWorkspace.h"
#include "MantidAPI/TableRow.h"
#include "MantidAPI/WorkspaceFactory.h"
#include "MantidDataHandling/LoadNexusProcessed.h"
#include "MantidDataHandling/LoadInstrument.h"
#include "MantidDataObjects/TableWorkspace.h"
#include "MantidDataObjects/Workspace2D.h"

#include <fstream>

using Mantid::Algorithms::FindPeaks;

using namespace Mantid::API;
using namespace Mantid::DataObjects;

using namespace std;

class FindPeaksTest : public CxxTest::TestSuite {
public:
  static FindPeaksTest *createSuite() { return new FindPeaksTest(); }
  static void destroySuite(FindPeaksTest *suite) { delete suite; }

  FindPeaksTest() { FrameworkManager::Instance(); }

  /// Test basic functions
  void test_TheBasics() {
    FindPeaks finder;
    TS_ASSERT_EQUALS(finder.name(), "FindPeaks");
    TS_ASSERT_EQUALS(finder.version(), 1);
  }

  /// Test initialization
  void test_Init() {
    FindPeaks finder;
    TS_ASSERT_THROWS_NOTHING(finder.initialize());
    TS_ASSERT(finder.isInitialized());
  }

  //----------------------------------------------------------------------------------------------
  /** Test find a single peak with given position
    */
  void test_findSinglePeakGivenPeakPosition() {
    FrameworkManager::Instance();

    MatrixWorkspace_sptr dataws = getSinglePeakData();
    std::string wsname("SinglePeakTestData");
    AnalysisDataService::Instance().addOrReplace(wsname, dataws);

    FindPeaks finder1;
    finder1.initialize();
    TS_ASSERT(finder1.isInitialized());

    TS_ASSERT_THROWS_NOTHING(
        finder1.setPropertyValue("InputWorkspace", wsname));
    TS_ASSERT_THROWS_NOTHING(finder1.setPropertyValue("WorkspaceIndex", "0"));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("Tolerance", 4));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("FWHM", 8));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setPropertyValue("PeakPositions", "1.2356"));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setPropertyValue("FitWindows", "1.21, 1.50"));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("PeakFunction", "Gaussian"));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setProperty("BackgroundType", "Quadratic"));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("HighBackground", true));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("MinGuessedPeakWidth", 2));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("MaxGuessedPeakWidth", 10));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setProperty("PeakPositionTolerance", 0.05));
    TS_ASSERT_THROWS_NOTHING(finder1.setProperty("RawPeakParameters", true));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setPropertyValue("CostFunction", "Chi-Square"));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setPropertyValue("Minimizer", "Levenberg-MarquardtMD"));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setPropertyValue("PeaksList", "FoundedSinglePeakTable"));
    TS_ASSERT_THROWS_NOTHING(
        finder1.setProperty("StartFromObservedPeakCentre", false));

    TS_ASSERT_THROWS_NOTHING(finder1.execute());

    TS_ASSERT(finder1.isExecuted());

    // Get output workspace
    TableWorkspace_sptr outtablews =
        boost::dynamic_pointer_cast<TableWorkspace>(
            AnalysisDataService::Instance().retrieve("FoundedSinglePeakTable"));
    TS_ASSERT(outtablews);

    // Size of the output workspace
    TS_ASSERT_EQUALS(outtablews->rowCount(), 1);
    if (outtablews->rowCount() == 0)
      return;

    map<string, double> parammap;
    getParameterMap(outtablews, 0, parammap);

    TS_ASSERT_DELTA(parammap["PeakCentre"], 1.2356, 0.03);
    TS_ASSERT_DELTA(parammap["Height"], 595., 5.00);

    // Clean
    AnalysisDataService::Instance().remove(wsname);
    AnalysisDataService::Instance().remove("FoundedSinglePeakTable");
  }

  //----------------------------------------------------------------------------------------------
  /** Test find peaks automaticallyclear
    */
  void test_findMultiPeaksAuto() {
    // Load data file
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setProperty("Filename", "focussed.nxs");
    loader.setProperty("OutputWorkspace", "FindPeaksTest_peaksWS");
    loader.execute();

    // Find peaks (Test)
    FindPeaks finder;
    if (!finder.isInitialized())
      finder.initialize();

    TS_ASSERT_THROWS_NOTHING(
        finder.setPropertyValue("InputWorkspace", "FindPeaksTest_peaksWS"));
    TS_ASSERT_THROWS_NOTHING(finder.setPropertyValue("WorkspaceIndex", "4"));
    // TS_ASSERT_THROWS_NOTHING(
    // finder.setPropertyValue("SmoothedData","smoothed") );
    TS_ASSERT_THROWS_NOTHING(
        finder.setPropertyValue("PeaksList", "FindPeaksTest_foundpeaks"));

    TS_ASSERT_THROWS_NOTHING(finder.execute());
    TS_ASSERT(finder.isExecuted());

    Mantid::API::ITableWorkspace_sptr peaklist =
        boost::dynamic_pointer_cast<Mantid::API::ITableWorkspace>(
            Mantid::API::AnalysisDataService::Instance().retrieve(
                "FindPeaksTest_foundpeaks"));

    TS_ASSERT(peaklist);
    TS_ASSERT_EQUALS(peaklist->rowCount(), 9);
    TS_ASSERT_DELTA(peaklist->Double(1, 1), 0.59, 0.01);
    TS_ASSERT_DELTA(peaklist->Double(2, 1), 0.71, 0.01);
    TS_ASSERT_DELTA(peaklist->Double(3, 1), 0.81, 0.01);
    // This is a dodgy value, that comes out different on different platforms
    // TS_ASSERT_DELTA( peaklist->Double(3,1), 1.03, 0.01 );
    TS_ASSERT_DELTA(peaklist->Double(5, 1), 0.96, 0.01);
    TS_ASSERT_DELTA(peaklist->Double(6, 1), 1.24, 0.01);
    TS_ASSERT_DELTA(peaklist->Double(7, 1), 1.52, 0.01);
    TS_ASSERT_DELTA(peaklist->Double(8, 1), 2.14, 0.01);
  }

  void NtestFindMultiPeaksGivenPeaksList() {
    this->LoadPG3_733();

    FindPeaks finder;
    if (!finder.isInitialized())
      finder.initialize();
    TS_ASSERT_THROWS_NOTHING(
        finder.setPropertyValue("InputWorkspace", "FindPeaksTest_vanadium"));
    TS_ASSERT_THROWS_NOTHING(finder.setPropertyValue("WorkspaceIndex", "0"));
    TS_ASSERT_THROWS_NOTHING(finder.setPropertyValue(
        "PeakPositions", "0.5044,0.5191,0.5350,0.5526,0.5936,0.6178,0.6453,0."
                         "6768,0.7134,0.7566,0.8089,0.8737,0.9571,1.0701,1."
                         "2356,1.5133,2.1401"));
    TS_ASSERT_THROWS_NOTHING(
        finder.setPropertyValue("PeaksList", "FindPeaksTest_foundpeaks2"));

    TS_ASSERT_THROWS_NOTHING(finder.execute());
    TS_ASSERT(finder.isExecuted());
  }

  //----------------------------------------------------------------------------------------------
  /** Parse a row in output parameter tableworkspace to a string/double
   * parameter name/value map
    */
  void getParameterMap(TableWorkspace_sptr tablews, size_t rowindex,
                       map<string, double> &parammap) {
    parammap.clear();

    vector<string> vecnames = tablews->getColumnNames();
    /*
    for (size_t i = 0; i < vecnames.size(); ++i)
      cout << "Column " << i << " : " << vecnames[i] << "\n";

    size_t numrows = tablews->rowCount();
    cout << "Number of rows = " << numrows << "\n";
    */

    for (size_t i = 0; i < vecnames.size(); ++i) {
      string parname = vecnames[i];
      if (parname != "spectrum") {
        double parvalue = tablews->cell<double>(rowindex, i);
        parammap.emplace(parname, parvalue);
        cout << "Add parameter " << parname << " = " << parvalue << "\n";
      }
    }

    return;
  }

  /// Load PG3_733 focussed data from AutoTest
  void LoadPG3_733() {
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setProperty("Filename", "PG3_733_focussed.nxs");
    loader.setProperty("OutputWorkspace", "FindPeaksTest_vanadium");
    loader.execute();
  }

  //----------------------------------------------------------------------------------------------
  /** Create a workspace as a partial data from PG3_4866 around Vanadium peak at
   * d = 1.235
    */
  MatrixWorkspace_sptr getSinglePeakData() {

		const size_t size = 83;

    std::array<double, size> vecX = {
        1.210120, 1.210600, 1.211080, 1.211570, 1.212050, 1.212540, 1.213020,
        1.213510, 1.213990, 1.214480, 1.214970, 1.215450, 1.215940, 1.216420,
        1.216910, 1.217400, 1.217880, 1.218370, 1.218860, 1.219350, 1.219830,
        1.220320, 1.220810, 1.221300, 1.221790, 1.222280, 1.222760, 1.223250,
        1.223740, 1.224230, 1.224720, 1.225210, 1.225700, 1.226190, 1.226680,
        1.227170, 1.227660, 1.228160, 1.228650, 1.229140, 1.229630, 1.230120,
        1.230610, 1.231110, 1.231600, 1.232090, 1.232580, 1.233080, 1.233570,
        1.234060, 1.234560, 1.235050, 1.235540, 1.236040, 1.236530, 1.237030,
        1.237520, 1.238020, 1.238510, 1.239010, 1.239500, 1.240000, 1.240500,
        1.240990, 1.241490, 1.241990, 1.242480, 1.242980, 1.243480, 1.243970,
        1.244470, 1.244970, 1.245470, 1.245970, 1.246460, 1.246960, 1.247460,
        1.247960, 1.248460, 1.248960, 1.249460, 1.249960, 1.250460};
    std::array<double, size> vecY = {
        1619.0, 1644.0, 1616.0, 1589.0, 1608.0, 1612.0, 1630.0, 1671.0, 1588.0,
        1577.0, 1616.0, 1556.0, 1625.0, 1655.0, 1552.0, 1539.0, 1538.0, 1542.0,
        1558.0, 1628.0, 1557.0, 1606.0, 1563.0, 1611.0, 1584.0, 1447.0, 1532.0,
        1580.0, 1539.0, 1513.0, 1601.0, 1558.0, 1567.0, 1573.0, 1551.0, 1465.0,
        1602.0, 1543.0, 1538.0, 1515.0, 1556.0, 1574.0, 1519.0, 1452.0, 1568.0,
        1522.0, 1518.0, 1603.0, 1538.0, 1659.0, 1685.0, 1763.0, 1846.0, 1872.0,
        2018.0, 2035.0, 2113.0, 2131.0, 1921.0, 1947.0, 1756.0, 1603.0, 1602.0,
        1552.0, 1558.0, 1518.0, 1512.0, 1511.0, 1466.0, 1474.0, 1368.0, 1463.0,
        1447.0, 1409.0, 1381.0, 1478.0, 1445.0, 1429.0, 1447.0, 1354.0, 1430.0,
        1440.0, 1423.0};
    std::array<double, size> vecE = {
        40.236800, 40.546300, 40.199500, 39.862300, 40.099900, 40.149700,
        40.373300, 40.877900, 39.849700, 39.711500, 40.199500, 39.446200,
        40.311300, 40.681700, 39.395400, 39.230100, 39.217300, 39.268300,
        39.471500, 40.348500, 39.458800, 40.074900, 39.534800, 40.137300,
        39.799500, 38.039500, 39.140800, 39.749200, 39.230100, 38.897300,
        40.012500, 39.471500, 39.585400, 39.661100, 39.382700, 38.275300,
        40.025000, 39.281000, 39.217300, 38.923000, 39.446200, 39.673700,
        38.974400, 38.105100, 39.598000, 39.012800, 38.961500, 40.037500,
        39.217300, 40.730800, 41.048800, 41.988100, 42.965100, 43.266600,
        44.922200, 45.111000, 45.967400, 46.162800, 43.829200, 44.124800,
        41.904700, 40.037500, 40.025000, 39.395400, 39.471500, 38.961500,
        38.884400, 38.871600, 38.288400, 38.392700, 36.986500, 38.249200,
        38.039500, 37.536600, 37.161800, 38.444800, 38.013200, 37.802100,
        38.039500, 36.796700, 37.815300, 37.947300, 37.722700,
    };

    MatrixWorkspace_sptr dataws = boost::dynamic_pointer_cast<MatrixWorkspace>(
        WorkspaceFactory::Instance().create("Workspace2D", 1, size, size));

    for (size_t i = 0; i < size; ++i) {
      dataws->dataX(0)[i] = vecX[i];
      dataws->dataY(0)[i] = vecY[i];
      dataws->dataE(0)[i] = vecE[i];
    }

    return dataws;
  }

private:
};

//=================================================================================================
/** Performance test with large workspaces.
  */

class FindPeaksTestPerformance : public CxxTest::TestSuite {
  Mantid::API::MatrixWorkspace_sptr m_dataWS;

public:
  static FindPeaksTestPerformance *createSuite() {
    return new FindPeaksTestPerformance();
  }
  static void destroySuite(FindPeaksTestPerformance *suite) { delete suite; }

  /** Constructor
    */
  FindPeaksTestPerformance() {}

  /** Set up workspaces
    */
  void setUp() {
    // Load data file
    Mantid::DataHandling::LoadNexusProcessed loader;
    loader.initialize();
    loader.setProperty("Filename", "focussed.nxs");
    loader.setProperty("OutputWorkspace", "FindPeaksTest_peaksWS");
    loader.execute();

    m_dataWS = boost::dynamic_pointer_cast<Mantid::API::MatrixWorkspace>(
        AnalysisDataService::Instance().retrieve("FindPeaksTest_peaksWS"));

    return;
  }

  /** Find peaks by auto-determine peaks' positions
    */
  void test_FindPeaksAutoPeakPositions() {
    // Find peaks (Test)
    FindPeaks finder;
    if (!finder.isInitialized())
      finder.initialize();

    if (!m_dataWS)
      throw std::runtime_error("Unable to get input matrix workspace. ");
    finder.setPropertyValue("InputWorkspace", "FindPeaksTest_peaksWS");
    finder.setPropertyValue("PeakPositions",
                            "0.8089, 0.9571, 1.0701,1.2356,1.5133,2.1401");
    finder.setPropertyValue("PeaksList", "FindPeaksTest_foundpeaks");

    finder.execute();
  }

}; // end of class FindPeaksTestPerformance

#endif /*FINDPEAKSTEST_H_*/
