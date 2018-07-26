#include "MantidDataHandling/LoadPSIMuonBin.h"

#include <fstream>

namespace Mantid {
namespace DataHandling {

DECLARE_FILELOADER_ALGORITHM(LoadPSIMuonBin)

// Algorithm's name for identification. @see Algorithm::name
const std::string LoadPSIMuonBin::name() const { return "LoadPSIMuonBin"; }

// Algorithm's version for identification. @see Algorithm::version
int LoadPSIMuonBin::version() const { return 1; }

const std::string LoadPSIMuonBin::summary() const {
  return "Loads a data file that is in PSI Muon Binary format into a "
         "workspace (Workspace2D class).";
}

// Algorithm's category for identification. @see Algorithm::category
const std::string LoadPSIMuonBin::category() const {
  return "DataHandling\\PSIMuonBin";
}

// Default constructor TODO: Delete if not needed
LoadPSIMuonBin::LoadPSIMuonBin() {}

/**
 * @brief
 *
 * @param descriptor
 * @return int
 */
int LoadPSIMuonBin::confidence(Kernel::FileDescriptor &descriptor) const override {
  auto &stream = descriptor.data();
  // 85th character is a space & 89th character is a ~
  stream.seekg(0, std::ios::beg);
  int c = stream.get();
  int confidence(0);
  // Checks if postion 0 is a 1
  if (c == 49) {
    stream.seekg(1, std::ios::cur);
    int c = stream.get();
    // Checks if position 1 is N
    if (c == 78)
      confidence = 90;
  }
  return confidence;
}
}

// version 1 does not however there is an issue open to create a version which
// handles temperatures aswell
bool LoadPSIMuonBin::loadMutipleAsOne() override { return false; }

void LoadPSIMuonBin::init() {
  const std::vector<std::string> exts{".bin"};
  declareProperty(Kernel::make_unique<FileProperty>("Filename", "",
                                                    FileProperty::Load, exts),
                  "The name of the Bin file to load");

  declareProperty(Kernel::make_unique<WorkspaceProperty<Workspace>>(
                      "OutputWorkspace", "", Mantid::Direction::Output),
                  "An output workspace.");
}

void LoadPSIMuonBin::exec() {

  if (sizeOf(float) != 4) {
    // assumpetions made about the binary input won't work but since there is no
    // way to guarantee floating points are 32 bits on all Operating systems here
    // we are.
  }

  std::string binFilename = getPropertyValue("Filename");
  // std::vector<Histogram> readHistogramData;

  ifstream binFile(binFileName, ios::in|ios::binary:ios::ate);

  auto size = binFile.tellg();

  const int sizeOfHeader = 1024;
  const char binFileHeaderBuffer[sizeofHeader];
  binFile.read(binFileHeaderBuffer, sizeOfHeader);

  std::string fileFormat(binFileHeaderBuffer, 2);

  if (fileFormat != "1N") {
    // Throw Error for file type
  }

  std::memcpy(&tdcResolution, binFileHeaderBuffer + 2, 2); // get resolution
}
} // namespace DataHandling
} // namespace Mantid