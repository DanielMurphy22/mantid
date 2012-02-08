#ifndef MANTID_MDALGORITHMS_COMPAREMDWORKSPACESTEST_H_
#define MANTID_MDALGORITHMS_COMPAREMDWORKSPACESTEST_H_

#include "MantidKernel/System.h"
#include "MantidKernel/Timer.h"
#include "MantidMDAlgorithms/CompareMDWorkspaces.h"
#include "MantidTestHelpers/MDEventsTestHelper.h"
#include <cxxtest/TestSuite.h>
#include <iomanip>
#include <iostream>
#include "MantidMDEvents/MDHistoWorkspace.h"

using namespace Mantid;
using namespace Mantid::MDAlgorithms;
using namespace Mantid::API;
using namespace Mantid::MDEvents;
using namespace Mantid::MDEvents::MDEventsTestHelper;
using Mantid::MDEvents::MDHistoWorkspace_sptr;

class CompareMDWorkspacesTest : public CxxTest::TestSuite
{
public:
  void test_Init()
  {
    CompareMDWorkspaces alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
  }
  
  void doTest(std::string ws1, std::string ws2, std::string resultExpected="Success!",
      bool CheckEvents = true)
  {

    CompareMDWorkspaces alg;
    TS_ASSERT_THROWS_NOTHING( alg.initialize() )
    TS_ASSERT( alg.isInitialized() )
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("Workspace1", ws1) );
    TS_ASSERT_THROWS_NOTHING( alg.setPropertyValue("Workspace2", ws2) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("CheckEvents", CheckEvents) );
    TS_ASSERT_THROWS_NOTHING( alg.setProperty("Tolerance", 1e-5) );
    TS_ASSERT_THROWS_NOTHING( alg.execute(); );
    TS_ASSERT( alg.isExecuted() );

    std::string result = alg.getPropertyValue("Result");
    std::cout << result << std::endl;
    TSM_ASSERT( result.c_str(), boost::starts_with(result, resultExpected));
  }

  void test_histo()
  {
    MDHistoWorkspace_sptr A = makeFakeMDHistoWorkspace(1.56, 3, 10, 10.0, 1.57, "A");
    MDHistoWorkspace_sptr B = makeFakeMDHistoWorkspace(1.56, 3, 10, 10.0, 1.57, "B");
    doTest("A", "B");
    B->setSignalAt(123, 2.34);
    doTest("A", "B", "MDHistoWorkspaces have a different signal at index 123");
    B->setSignalAt(123, 1.56);
    B->setErrorSquaredAt(123, 2.34);
    doTest("A", "B", "MDHistoWorkspaces have a different error at index 123");

    MDHistoWorkspace_sptr C = makeFakeMDHistoWorkspace(1.56, 3, 9, 10.0, 1.57, "C");
    doTest("A", "C", "Dimension #0 has a different number of bins");
    MDHistoWorkspace_sptr C2 = makeFakeMDHistoWorkspace(1.56, 3, 10, 20.0, 1.57, "C2");
    doTest("A", "C2", "Dimension #0 has a different maximum");

    MDHistoWorkspace_sptr D = makeFakeMDHistoWorkspace(1.56, 2, 10, 10.0, 1.57, "D");
    doTest("A", "D", "Workspaces have a different number of dimensions");

  }
  

  void test_md()
  {
    makeAnyMDEW<MDEvent<3>, 3>(2, 0., 10., 1, "A");
    makeAnyMDEW<MDLeanEvent<3>, 3>(2, 0., 10., 1, "mdle3");
    doTest("A", "mdle3", "Workspaces are of different types");

    makeAnyMDEW<MDEvent<3>, 3>(2, 0., 10., 2, "B");
    doTest("A", "B", "Box signal does not match");

    makeAnyMDEW<MDEvent<3>, 3>(3, 0., 10., 1, "C");
    doTest("A", "C", "Workspaces do not have the same number of boxes");


  }


};


#endif /* MANTID_MDALGORITHMS_COMPAREMDWORKSPACESTEST_H_ */
