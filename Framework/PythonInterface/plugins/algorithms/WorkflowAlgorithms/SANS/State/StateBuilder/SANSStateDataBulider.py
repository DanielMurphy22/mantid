import copy
from SANSStateBase import (TypedParameter, is_not_none)
from Common.SANSConstants import SANSFacility
from State.SANSStateData import SANSStateDataISIS


# ---------------------------------------
# State builders
# ---------------------------------------
class SANStateDataISISBuilder(object):
    state = TypedParameter(SANSStateDataISIS, is_not_none)

    def __init__(self):
        super(SANStateDataISISBuilder, self).__init__()
        self.state = SANSStateDataISIS()

    def build(self):
        # Make sure that the product is in a valid state, ie not incomplete
        self.state.validate()
        return copy.copy(self.state)


# ------------------------------------------
# Factory method for SANStateDataBuilder
# ------------------------------------------
def get_state_data_builder(facility):
    if facility is SANSFacility.ISIS:
        return SANSStateDataISIS()
    else:
        raise NotImplementedError("SANSStateDataBuilder: The selected facility {} does not seem"
                                  " to exist".format(str(facility)))
