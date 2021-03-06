// Mantid Repository : https://github.com/mantidproject/mantid
//
// Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
//   NScD Oak Ridge National Laboratory, European Spallation Source,
//   Institut Laue - Langevin & CSNS, Institute of High Energy Physics, CAS
// SPDX - License - Identifier: GPL - 3.0 +
#include "IndirectFitOutput.h"

#include "MantidAPI/AlgorithmManager.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/TableRow.h"
#include "MantidAPI/TextAxis.h"

#include <boost/functional/hash.hpp>

#include <unordered_set>
#include <utility>

using namespace Mantid::API;
using IDAWorkspaceIndex = MantidQt::CustomInterfaces::IDA::WorkspaceIndex;

namespace {
using namespace MantidQt::CustomInterfaces::IDA;

struct TableRowExtractor {
  explicit TableRowExtractor(ITableWorkspace_sptr table)
      : m_table(std::move(table)), m_columns(m_table->getColumnNames()) {
    m_chiIndex = std::find(m_columns.begin(), m_columns.end(), "Chi_squared") -
                 m_columns.begin();
  }

  std::unordered_map<std::string, ParameterValue>
  operator()(IDAWorkspaceIndex index) {
    TableRow row = m_table->getRow(index.value);
    std::unordered_map<std::string, ParameterValue> parameters;

    for (auto i = 1u; i < m_chiIndex; i += 2) {
      const auto &columnName = m_columns[i];
      parameters[columnName] = ParameterValue(row.Double(i), row.Double(i + 1));
    }
    return parameters;
  }

private:
  ITableWorkspace_sptr m_table;
  const std::vector<std::string> m_columns;
  std::size_t m_chiIndex;
};

template <typename Map, typename Key>
typename Map::mapped_type &extractOrAddDefault(Map &map, const Key &key) {
  auto values = map.find(key);
  if (values != map.end())
    return values->second;
  return map[key] = typename Map::mapped_type();
}

template <typename F>
void applyEnumeratedData(F &&functor, const FitDataIterator &fitDataBegin,
                         const FitDataIterator &fitDataEnd) {
  IDAWorkspaceIndex start{0};
  for (auto it = fitDataBegin; it < fitDataEnd; ++it)
    start = (*it)->applyEnumeratedSpectra(functor(it->get()), start);
}

template <typename F>
void applyData(F &&functor, const FitDataIterator &fitDataBegin,
               const FitDataIterator &fitDataEnd) {
  for (auto it = fitDataBegin; it < fitDataEnd; ++it)
    (*it)->applySpectra(functor(it->get()));
}

void extractParametersFromTable(
    ITableWorkspace_sptr tableWs, const FitDataIterator &fitDataBegin,
    const FitDataIterator &fitDataEnd,
    std::unordered_map<IndirectFitData const *, ParameterValuesNew>
        &parameters) {
  TableRowExtractor extractRowFromTable(std::move(tableWs));
  IDAWorkspaceIndex index;
  for (auto fitData = fitDataBegin; fitData < fitDataEnd; ++fitData) {
    auto &values = parameters[fitData->get()];
    for (const auto &spectrum : (**fitData).spectra()) {
      values[spectrum] = extractRowFromTable(index);
      ++index;
    }
  }
}

template <typename Map, typename Value, typename Key>
Value getValueOr(const Map &map, const Value &defaultValue, const Key &key) {
  const auto value = map.find(key);
  if (value != map.end())
    return value->second;
  return defaultValue;
}

template <typename MapND, typename Value, typename Key, typename... Keys>
Value getValueOr(const MapND &map, const Value &defaultValue, const Key &key,
                 const Keys &... keys) {
  const auto values = map.find(key);
  if (values != map.end())
    return getValueOr(values->second, defaultValue, keys...);
  return defaultValue;
}

template <typename Map, typename KeyMap>
Map mapKeys(const Map &map, const KeyMap &keyMap) {
  Map newMap;

  for (const auto unmapped : map) {
    const auto mapping = keyMap.find(unmapped.first);
    if (mapping != keyMap.end())
      newMap[mapping->second] = unmapped.second;
    else
      newMap[unmapped.first] = unmapped.second;
  }
  return newMap;
}

MatrixWorkspace_sptr
getMatrixWorkspaceFromGroup(const WorkspaceGroup_sptr &group,
                            std::size_t index) {
  if (group->size() > index)
    return std::dynamic_pointer_cast<MatrixWorkspace>(group->getItem(index));
  return nullptr;
}

std::vector<std::string> getAxisLabels(TextAxis const *axis) {
  std::vector<std::string> labels;
  labels.reserve(axis->length());
  for (auto i = 0u; i < axis->length(); ++i)
    labels.emplace_back(axis->label(i));
  return labels;
}

std::vector<std::string> getAxisLabels(const MatrixWorkspace_sptr &workspace,
                                       std::size_t index) {
  auto axis = dynamic_cast<TextAxis *>(workspace->getAxis(index));
  if (axis)
    return getAxisLabels(axis);
  return std::vector<std::string>();
}

std::string cutLastOf(const std::string &str, const std::string &delimiter) {
  const auto cutIndex = str.rfind(delimiter);
  if (cutIndex != std::string::npos)
    return str.substr(0, cutIndex);
  return str;
}

bool containsMultipleData(const std::string &name) {
  return name.substr(0, 5) == "Multi";
}

std::string constructResultName(const std::string &name,
                                IndirectFitData const *fitData) {
  if (containsMultipleData(name)) {
    const auto formatString = cutLastOf(name, "_Results") + "_%1%_s%2%_Result";
    return fitData->displayName(formatString, "_to_");
  } else
    return cutLastOf(name, "s_1");
}

void renameWorkspace(std::string const &name, std::string const &newName) {
  auto renamer = AlgorithmManager::Instance().create("RenameWorkspace");
  renamer->setProperty("InputWorkspace", name);
  renamer->setProperty("OutputWorkspace", newName);
  renamer->execute();
}

void renameResult(const Workspace_sptr &resultWorkspace,
                  const std::string &workspaceName) {
  renameWorkspace(resultWorkspace->getName(), workspaceName + "_Result");
}

void renameResult(const Workspace_sptr &resultWorkspace,
                  IndirectFitData const *fitData) {
  const auto name = resultWorkspace->getName();
  const auto newName = constructResultName(name, fitData);
  if (newName != name)
    renameWorkspace(name, newName);
}

void renameResult(const WorkspaceGroup_sptr &resultWorkspace,
                  IndirectFitData const *fitData) {
  for (auto const &workspace : *resultWorkspace)
    renameResult(workspace, fitData);
}

void renameResultWithoutSpectra(const WorkspaceGroup_sptr &resultWorkspace,
                                const FitDataIterator &fitDataBegin,
                                const FitDataIterator &fitDataEnd) {
  std::size_t index = 0;
  MatrixWorkspace const *previous = nullptr;

  for (auto it = fitDataBegin; it < fitDataEnd; ++it) {
    auto workspace = (*it)->workspace().get();
    if (workspace != previous) {
      renameResult(resultWorkspace->getItem(index++), workspace->getName());
      previous = workspace;
    }
  }
}

void renameResultWithSpectra(const WorkspaceGroup_sptr &resultWorkspace,
                             const FitDataIterator &fitDataBegin,
                             const FitDataIterator &fitDataEnd) {
  std::size_t index = 0;
  for (auto it = fitDataBegin; it < fitDataEnd; ++it)
    renameResult(resultWorkspace->getItem(index++), it->get());
}

void renameResult(const WorkspaceGroup_sptr &resultWorkspace,
                  const FitDataIterator &fitDataBegin,
                  const FitDataIterator &fitDataEnd) {
  if (static_cast<int>(resultWorkspace->size()) >= fitDataEnd - fitDataBegin)
    renameResultWithSpectra(resultWorkspace, fitDataBegin, fitDataEnd);
  else
    renameResultWithoutSpectra(resultWorkspace, fitDataBegin, fitDataEnd);
}

template <typename Map, typename Key>
typename Map::mapped_type &findOrCreateDefaultInMap(Map &map, const Key &key) {
  auto valueIt = map.find(key);
  if (valueIt != map.end())
    return valueIt->second;
  return map[key] = typename Map::mapped_type();
}

TableRowIndex numberOfSpectraIn(const FitDataIterator &fitDataBegin,
                                const FitDataIterator &fitDataEnd) {
  TableRowIndex spectra{0};
  for (auto it = fitDataBegin; it < fitDataEnd; ++it)
    spectra += (*it)->numberOfSpectra();
  return spectra;
}
} // namespace

namespace MantidQt {
namespace CustomInterfaces {
namespace IDA {

IndirectFitOutput::IndirectFitOutput(const WorkspaceGroup_sptr &resultGroup,
                                     ITableWorkspace_sptr parameterTable,
                                     const WorkspaceGroup_sptr &resultWorkspace,
                                     const FitDataIterator &fitDataBegin,
                                     const FitDataIterator &fitDataEnd)
    : m_resultGroup(resultGroup), m_resultWorkspace(resultWorkspace),
      m_parameters(), m_outputResultLocations() {
  addOutput(resultGroup, std::move(parameterTable), resultWorkspace,
            fitDataBegin, fitDataEnd);
}

IndirectFitOutput::IndirectFitOutput(const WorkspaceGroup_sptr &resultGroup,
                                     ITableWorkspace_sptr parameterTable,
                                     const WorkspaceGroup_sptr &resultWorkspace,
                                     IndirectFitData const *fitData,
                                     WorkspaceIndex spectrum) {
  m_parameters[fitData] = ParameterValuesNew();
  m_outputResultLocations[fitData] = ResultLocationsNew();
  addOutput(std::move(resultGroup), std::move(parameterTable),
            std::move(resultWorkspace), fitData, spectrum);
}

bool IndirectFitOutput::isSpectrumFit(IndirectFitData const *fitData,
                                      WorkspaceIndex spectrum) const {
  auto values = m_parameters.find(fitData);
  return values != m_parameters.end() &&
         values->second.find(spectrum) != values->second.end();
}

std::unordered_map<std::string, ParameterValue>
IndirectFitOutput::getParameters(IndirectFitData const *fitData,
                                 WorkspaceIndex spectrum) const {
  return getValueOr(m_parameters,
                    std::unordered_map<std::string, ParameterValue>(), fitData,
                    spectrum);
}

boost::optional<ResultLocationNew>
IndirectFitOutput::getResultLocation(IndirectFitData const *fitData,
                                     WorkspaceIndex spectrum) const {
  return getValueOr(m_outputResultLocations,
                    boost::optional<ResultLocationNew>(boost::none), fitData,
                    spectrum);
}

std::vector<std::string> IndirectFitOutput::getResultParameterNames() const {
  if (auto resultWorkspace = getLastResultWorkspace())
    if (auto workspace = getMatrixWorkspaceFromGroup(resultWorkspace, 0))
      return getAxisLabels(workspace, 1);
  return std::vector<std::string>();
}

WorkspaceGroup_sptr IndirectFitOutput::getLastResultWorkspace() const {
  return m_resultWorkspace.lock();
}

WorkspaceGroup_sptr IndirectFitOutput::getLastResultGroup() const {
  return m_resultGroup.lock();
}

void IndirectFitOutput::mapParameterNames(
    const std::unordered_map<std::string, std::string> &parameterNameChanges,
    const FitDataIterator &fitDataBegin, const FitDataIterator &fitDataEnd) {
  for (auto it = fitDataBegin; it < fitDataEnd; ++it)
    mapParameterNames(parameterNameChanges, it->get());
}

void IndirectFitOutput::mapParameterNames(
    const std::unordered_map<std::string, std::string> &parameterNameChanges,
    IndirectFitData const *fitData) {
  const auto parameterIt = m_parameters.find(fitData);
  if (parameterIt != m_parameters.end()) {
    for (const auto &values : parameterIt->second)
      parameterIt->second[values.first] =
          mapKeys(values.second, parameterNameChanges);
  }
}

void IndirectFitOutput::mapParameterNames(
    const std::unordered_map<std::string, std::string> &parameterNameChanges,
    IndirectFitData const *fitData, WorkspaceIndex spectrum) {
  auto &parameters = m_parameters[fitData][spectrum];
  parameters = mapKeys(parameters, parameterNameChanges);
}

void IndirectFitOutput::addOutput(const WorkspaceGroup_sptr &resultGroup,
                                  ITableWorkspace_sptr parameterTable,
                                  const WorkspaceGroup_sptr &resultWorkspace,
                                  const FitDataIterator &fitDataBegin,
                                  const FitDataIterator &fitDataEnd) {
  updateParameters(std::move(parameterTable), fitDataBegin, fitDataEnd);
  updateFitResults(resultGroup, fitDataBegin, fitDataEnd);
  renameResult(resultWorkspace, fitDataBegin, fitDataEnd);
  m_resultWorkspace = resultWorkspace;
  m_resultGroup = resultGroup;
}

void IndirectFitOutput::addOutput(const WorkspaceGroup_sptr &resultGroup,
                                  ITableWorkspace_sptr parameterTable,
                                  const WorkspaceGroup_sptr &resultWorkspace,
                                  IndirectFitData const *fitData,
                                  WorkspaceIndex spectrum) {
  TableRowExtractor extractRowFromTable(std::move(parameterTable));
  m_parameters[fitData][spectrum] = extractRowFromTable(WorkspaceIndex{0});
  m_outputResultLocations[fitData][spectrum] =
      ResultLocationNew(resultGroup, WorkspaceGroupIndex{0});
  renameResult(resultWorkspace, fitData);
  m_resultWorkspace = resultWorkspace;
  m_resultGroup = resultGroup;
}

void IndirectFitOutput::removeOutput(IndirectFitData const *fitData) {
  m_parameters.erase(fitData);
  m_outputResultLocations.erase(fitData);
}

void IndirectFitOutput::updateFitResults(const WorkspaceGroup_sptr &resultGroup,
                                         const FitDataIterator &fitDataBegin,
                                         const FitDataIterator &fitDataEnd) {
  if (numberOfSpectraIn(fitDataBegin, fitDataEnd).value <=
      static_cast<int>(resultGroup->size()))
    updateFitResultsFromStructured(resultGroup, fitDataBegin, fitDataEnd);
  else
    updateFitResultsFromUnstructured(resultGroup, fitDataBegin, fitDataEnd);
}

void IndirectFitOutput::updateParameters(ITableWorkspace_sptr parameterTable,
                                         const FitDataIterator &fitDataBegin,
                                         const FitDataIterator &fitDataEnd) {
  extractParametersFromTable(std::move(parameterTable), fitDataBegin,
                             fitDataEnd, m_parameters);
}

void IndirectFitOutput::updateFitResultsFromUnstructured(
    const WorkspaceGroup_sptr &resultGroup, const FitDataIterator &fitDataBegin,
    const FitDataIterator &fitDataEnd) {
  std::unordered_map<MatrixWorkspace *,
                     std::map<WorkspaceIndex, WorkspaceGroupIndex>>
      resultIndices;
  WorkspaceGroupIndex index{0};
  for (auto fitData = fitDataBegin; fitData < fitDataEnd; ++fitData) {
    auto &fitResults = m_outputResultLocations[fitData->get()];
    auto ws = (*fitData)->workspace().get();
    auto &indices = resultIndices[ws];
    for (const auto &spectrum : (**fitData).spectra()) {
      auto defaultIt = indices.find(spectrum);
      if (defaultIt != indices.end()) {
        fitResults[spectrum] =
            ResultLocationNew(resultGroup, defaultIt->second);
      } else if (static_cast<int>(resultGroup->size()) > index.value) {
        fitResults[spectrum] = ResultLocationNew(resultGroup, index);
        indices[spectrum] = index;
        ++index;
      }
    }
  }
}

void IndirectFitOutput::updateFitResultsFromStructured(
    const WorkspaceGroup_sptr &resultGroup, const FitDataIterator &fitDataBegin,
    const FitDataIterator &fitDataEnd) {
  WorkspaceGroupIndex index;
  for (auto fitData = fitDataBegin; fitData < fitDataEnd; ++fitData) {
    auto &fitResults = m_outputResultLocations[fitData->get()];
    for (const auto &spectrum : (**fitData).spectra()) {
      fitResults[spectrum] = ResultLocationNew(resultGroup, index);
      ++index;
    }
  }
}

} // namespace IDA
} // namespace CustomInterfaces
} // namespace MantidQt