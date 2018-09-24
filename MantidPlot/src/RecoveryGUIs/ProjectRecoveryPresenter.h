/** Adapter class which handles saving or restoring project windows

@author Samuel Jones, ISIS, RAL
@date 21/09/2018

Copyright &copy; 2007-2018 ISIS Rutherford Appleton Laboratory, NScD Oak Ridge
National Laboratory & European Spallation Source

This file is part of Mantid.

Mantid is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Mantid is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

File change history is stored at: <https://github.com/mantidproject/mantid>
*/

#ifndef PROJECTRECOVERYPRESENTER_H
#define PROJECTRECOVERYPRESENTER_H

#include "../ProjectRecovery.h"
#include "ProjectRecoveryView.h"
#include "ProjectRecoveryModel.h"
#include "RecoveryFailureView.h"

class ProjectRecoveryPresenter {
public:
  ProjectRecoveryPresenter(MantidQt::ProjectRecovery *projectRecovery);

private:
  ProjectRecoveryModel m_guiModel;
  ProjectRecoveryView m_recoveryView;
  RecoveryFailureView m_failureView;
  MantidQt::ProjectRecovery *m_projectRecovery;
};

#endif /* PROJECTRECOVERYPRESENTER_H */