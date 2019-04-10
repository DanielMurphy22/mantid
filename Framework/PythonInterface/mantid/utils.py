# Mantid Repository : https://github.com/mantidproject/mantid
#
# Copyright &copy; 2018 ISIS Rutherford Appleton Laboratory UKRI,
#     NScD Oak Ridge National Laboratory, European Spallation Source
#     & Institut Laue - Langevin
# SPDX - License - Identifier: GPL - 3.0 +
from contextlib import contextmanager
from importlib import import_module


def import_mantid_cext(modulename, package="", caller_globals=None):
    """
    Import a Mantid module built from the PythonInterface.

    :param modulename: Name of the module that will be imported
    :param package: The package where the module should be
    :param caller_globals: The globals() dictionary of the caller - the imported module's dictionary will be inserted
                           there, making all members available in the caller namespace. This simulates a
                           from X import *, which also inserts all imported members in the caller namespace
    :return: The module if successfully imported
    :raises: If the module cannot be imported
    """
    with _shared_cextension():
        if modulename.startswith('.'):
            try:
                # import from PACKAGE.MODULE, this is used for mantid packages, where the .pyd files
                # are placed at e.g. `from mantid.kernel import _kernel`
                # lib = import_module(modulename, package)
                exec "from {}{} import *".format(package, modulename) in caller_globals
            except ImportError as e1:
                # import relative to current working directory, this is essentially doing `import _kernel`
                try:
                    exec "from {} import *".format(modulename.lstrip('.')) in caller_globals
                except ImportError as e2:
                    msg = 'import of "{}" failed with "{}"'
                    msg = 'First ' + msg.format(modulename, e1) \
                        + '. Second ' + msg.format(modulename.lstrip('.'), e2)
                    raise ImportError(msg)
        else:
            pass
            # lib = import_module(modulename)

        # if caller_globals:
        #     caller_globals.update(lib.__dict__)
        return None


@contextmanager
def _shared_cextension():
    """Our extensions need to shared symbols amongst them due to:
      - the static boost python type registry
      - static singleton instances marked as weak symbols by clang
    gcc uses an extension to mark these attributes as globally unique
    but clang marks them as weak and without RTLD_GLOBAL each shared
    library has its own copy of each singleton.

    See https://docs.python.org/3/library/sys.html#sys.setdlopenflags
    """
    import sys
    if not sys.platform.startswith('linux'):
        yield
        return

    import six
    if six.PY2:
        import DLFCN as dl
    else:
        import os as dl
    flags_orig = sys.getdlopenflags()
    sys.setdlopenflags(dl.RTLD_NOW | dl.RTLD_GLOBAL)
    yield
    sys.setdlopenflags(flags_orig)
