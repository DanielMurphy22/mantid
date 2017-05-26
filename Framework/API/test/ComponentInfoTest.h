#ifndef MANTID_API_COMPONENTINFOTEST_H_
#define MANTID_API_COMPONENTINFOTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidAPI/ComponentInfo.h"
#include "MantidBeamline/ComponentInfo.h"
#include "MantidBeamline/DetectorInfo.h"
#include "MantidGeometry/IComponent.h"
#include "MantidGeometry/Instrument/ObjComponent.h"
#include "MantidKernel/EigenConversionHelpers.h"

#include <boost/make_shared.hpp>

using Mantid::API::ComponentInfo;
using namespace Mantid;
using namespace Mantid::Kernel;

namespace {

/*
 Helper function to create an ID -> index map from an ordered collection of IDs.
 First ID gets index of 0, subsequent ID entries increment index by 1.
*/
boost::shared_ptr<
    const std::unordered_map<Mantid::Geometry::ComponentID, size_t>>
makeComponentIDMap(const boost::shared_ptr<
    const std::vector<Mantid::Geometry::ComponentID>> &componentIds) {
  auto idMap = boost::make_shared<
      std::unordered_map<Mantid::Geometry::ComponentID, size_t>>();

  for (size_t i = 0; i < componentIds->size(); ++i) {
    (*idMap)[(*componentIds)[i]] = i;
  }
  return idMap;
}
}

class ComponentInfoTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created
  // statically
  // This means the constructor isn't called when running other tests
  static ComponentInfoTest *createSuite() { return new ComponentInfoTest(); }
  static void destroySuite(ComponentInfoTest *suite) { delete suite; }

  void test_indexOf() {
    auto detectorIndices = boost::make_shared<
        std::vector<size_t>>(); // No detectors in this example
    auto detectorRanges = boost::make_shared<std::vector<std::pair<size_t, size_t>>>();
    detectorRanges->push_back(std::make_pair(0, 0)); // One component with no detectors
    detectorRanges->push_back(
        std::make_pair(0, 0)); // Another component with no detectors

    auto componentIndices = boost::make_shared<std::vector<size_t>>(
        std::vector<size_t>{0, 1}); // No detectors in this example
    auto componentRanges =
        boost::make_shared<std::vector<std::pair<size_t, size_t>>>();
    componentRanges->push_back(
        std::make_pair(0, 0)); // One component with no sub-components
    componentRanges->push_back(
        std::make_pair(0, 0)); // Another component with no subcomponents

    auto positions = boost::make_shared<std::vector<Eigen::Vector3d>>(2);
    auto rotations = boost::make_shared<std::vector<Eigen::Quaterniond>>(2);
    auto detectorInfo = boost::make_shared<Beamline::DetectorInfo>();

    Mantid::Beamline::ComponentInfo internalInfo(
        detectorIndices, detectorRanges, componentIndices, componentRanges,
        positions, rotations, detectorInfo);
    Mantid::Geometry::ObjComponent comp1("component1");
    Mantid::Geometry::ObjComponent comp2("component2");

    auto componentIds =
        boost::make_shared<std::vector<Mantid::Geometry::ComponentID>>(
            std::vector<Mantid::Geometry::ComponentID>{&comp1, &comp2});
    ComponentInfo info(internalInfo, componentIds,
                       makeComponentIDMap(componentIds));
    TS_ASSERT_EQUALS(info.indexOf(comp1.getComponentID()), 0);
    TS_ASSERT_EQUALS(info.indexOf(comp2.getComponentID()), 1);
  }
};

#endif /* MANTID_API_COMPONENTINFOTEST_H_ */
