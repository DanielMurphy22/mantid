#include "MantidIndexing/IndexInfo.h"
#include "MantidIndexing/RoundRobinPartitioner.h"
#include "MantidIndexing/SpectrumNumberTranslator.h"
#include "MantidKernel/make_cow.h"
#include "MantidKernel/make_unique.h"
#include "MantidTypes/SpectrumDefinition.h"

#include <algorithm>
#include <functional>
#include <numeric>

namespace Mantid {
namespace Indexing {

/// Construct a default IndexInfo, with contiguous spectrum numbers starting at
/// 1 and no detector IDs.
IndexInfo::IndexInfo(const size_t globalSize, const StorageMode storageMode,
                     const Communicator &communicator)
    : m_storageMode(storageMode), m_communicator(communicator) {
  // Default to spectrum numbers 1...globalSize
  std::vector<SpectrumNumber> specNums(globalSize);
  std::iota(specNums.begin(), specNums.end(), 1);
  makeSpectrumNumberTranslator(std::move(specNums));
  m_spectrumDefinitions =
      Kernel::make_cow<std::vector<SpectrumDefinition>>(size());
}

/// Construct with given spectrum number and vector of detector IDs for each
/// index.
IndexInfo::IndexInfo(std::vector<SpectrumNumber> spectrumNumbers,
                     const StorageMode storageMode,
                     const Communicator &communicator)
    : m_storageMode(storageMode), m_communicator(communicator) {
  makeSpectrumNumberTranslator(std::move(spectrumNumbers));
  m_spectrumDefinitions =
      Kernel::make_cow<std::vector<SpectrumDefinition>>(size());
}

/// The *local* size, i.e., the number of spectra in this partition.
size_t IndexInfo::size() const {
  if (!m_spectrumNumberTranslator)
    return 0;
  return m_spectrumNumberTranslator->localSize();
}

/// Returns the spectrum number for given index.
SpectrumNumber IndexInfo::spectrumNumber(const size_t index) const {
  return m_spectrumNumberTranslator->spectrumNumber(index);
}

/// Returns the spectrum definition for given index.
const SpectrumDefinition &
IndexInfo::spectrumDefinition(const size_t index) const {
  return (*m_spectrumDefinitions)[index];
}

/// Sets the SpectrumDefinition for given index.
void IndexInfo::setSpectrumDefinition(const size_t index,
                                      SpectrumDefinition def) {
  m_spectrumDefinitions.access()[index] = std::move(def);
}

/// Set a spectrum number for each index.
void IndexInfo::setSpectrumNumbers(
    std::vector<SpectrumNumber> &&spectrumNumbers) {
  if (m_spectrumNumberTranslator->globalSize() != spectrumNumbers.size())
    throw std::runtime_error(
        "IndexInfo: Size mismatch when setting new spectrum numbers");
  makeSpectrumNumberTranslator(std::move(spectrumNumbers));
}

/// Set a contiguous range of spectrum numbers.
void IndexInfo::setSpectrumNumbers(const SpectrumNumber min,
                                   const SpectrumNumber max) {
  auto newSize = static_cast<int32_t>(max) - static_cast<int32_t>(min) + 1;
  if (static_cast<int64_t>(m_spectrumNumberTranslator->globalSize()) != newSize)
    throw std::runtime_error(
        "IndexInfo: Size mismatch when setting new spectrum numbers");
  std::vector<SpectrumNumber> specNums(newSize);
  std::iota(specNums.begin(), specNums.end(), static_cast<int32_t>(min));
  makeSpectrumNumberTranslator(std::move(specNums));
}

/// Set the spectrum definitions.
void IndexInfo::setSpectrumDefinitions(
    std::vector<SpectrumDefinition> spectrumDefinitions) {
  if (size() != spectrumDefinitions.size())
    throw std::runtime_error(
        "IndexInfo: Size mismatch when setting new spectrum definitions");
  m_spectrumDefinitions.access() = std::move(spectrumDefinitions);
}

/** Set the spectrum definitions.
 *
 * Note that in principle the spectrum definitions contain the same information
 * as the groups of detector IDs. However, Mantid currently supports invalid
 * detector IDs in groups, whereas spectrum definitions contain only valid
 * indices. Validation requires access to the instrument and thus cannot be done
 * internally in IndexInfo, i.e., spectrum definitions must be set by hand. */
void IndexInfo::setSpectrumDefinitions(
    Kernel::cow_ptr<std::vector<SpectrumDefinition>> spectrumDefinitions) {
  if (!spectrumDefinitions || (size() != spectrumDefinitions->size()))
    throw std::runtime_error(
        "IndexInfo: Size mismatch when setting new spectrum definitions");
  m_spectrumDefinitions = spectrumDefinitions;
}

/// Returns the spectrum definitions.
const Kernel::cow_ptr<std::vector<SpectrumDefinition>> &
IndexInfo::spectrumDefinitions() const {
  return m_spectrumDefinitions;
}

/** Creates an index set containing all indices.
 *
 * If there are multiple partitions (MPI ranks), the returned set contains the
 * subset of indices on this partition. */
SpectrumIndexSet IndexInfo::makeIndexSet() const {
  return m_spectrumNumberTranslator->makeIndexSet();
}

/** Creates an index set containing all indices with spectrum number between
 * `min` and `max`.
 *
 * If there are multiple partitions (MPI ranks), the returned set contains the
 * subset of indices on this partition. */
SpectrumIndexSet IndexInfo::makeIndexSet(SpectrumNumber min,
                                         SpectrumNumber max) const {
  return m_spectrumNumberTranslator->makeIndexSet(min, max);
}

/** Creates an index set containing all indices with global index between `min`
 * and `max`.
 *
 * If there are multiple partitions (MPI ranks), the returned set contains the
 * subset of indices on this partition. */
SpectrumIndexSet IndexInfo::makeIndexSet(GlobalSpectrumIndex min,
                                         GlobalSpectrumIndex max) const {
  return m_spectrumNumberTranslator->makeIndexSet(min, max);
}

/** Creates an index set containing all indices corresponding to the spectrum
 * numbers in the provided vector.
 *
 * If there are multiple partitions (MPI ranks), the returned set contains the
 * subset of indices on this partition. */
SpectrumIndexSet IndexInfo::makeIndexSet(
    const std::vector<SpectrumNumber> &spectrumNumbers) const {
  return m_spectrumNumberTranslator->makeIndexSet(spectrumNumbers);
}

/** Creates an index set containing all indices corresponding to the global
 * indices in the provided vector.
 *
 * If there are multiple partitions (MPI ranks), the returned set contains the
 * subset of indices on this partition. */
SpectrumIndexSet IndexInfo::makeIndexSet(
    const std::vector<GlobalSpectrumIndex> &globalIndices) const {
  return m_spectrumNumberTranslator->makeIndexSet(globalIndices);
}

void IndexInfo::makeSpectrumNumberTranslator(
    std::vector<SpectrumNumber> &&spectrumNumbers) const {
  PartitionIndex partition;
  int numberOfPartitions;
  if (m_storageMode == StorageMode::Distributed) {
    partition = PartitionIndex(m_communicator.rank);
    numberOfPartitions = m_communicator.size;
  } else {
    partition = PartitionIndex(0);
    numberOfPartitions = 1;
  }
  auto partitioner = Kernel::make_unique<RoundRobinPartitioner>(
      numberOfPartitions, partition,
      Partitioner::MonitorStrategy::TreatAsNormalSpectrum);
  // TODO How should we handle StorageMode::MasterOnly?
  m_spectrumNumberTranslator = Kernel::make_cow<SpectrumNumberTranslator>(
      std::move(spectrumNumbers), std::move(partitioner), partition);
}

} // namespace Indexing
} // namespace Mantid
