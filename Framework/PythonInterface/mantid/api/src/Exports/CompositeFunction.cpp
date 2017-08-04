#include "MantidPythonInterface/kernel/GetPointer.h"
#include "MantidAPI/CompositeFunction.h"
#include <boost/python/class.hpp>
#include <boost/python/overloads.hpp>
#include <boost/python/register_ptr_to_python.hpp>

using Mantid::API::CompositeFunction;
using Mantid::API::IFunction;
using namespace boost::python;

GET_POINTER_SPECIALIZATION(CompositeFunction)

namespace {

/// An object for constructing a shared_ptr that won't ever delete its pointee
class NoDeleting {
public:
  /// Does nothing
  void operator()(void *) {}
  /// Does nothing
  void operator()(const void *) {}
};

//------------------------------------------------------------------------------------------------------
/**
* Something that makes composite function return to python a composite function
* for Product function, Convolution or
* any similar superclass of composite function.
* @param self :: Enables it to be called as a member function on the
* Composite Function class
*/
Mantid::API::CompositeFunction_sptr
castToCompositeFunction(CompositeFunction &self) {
  return boost::shared_ptr<Mantid::API::CompositeFunction>(&self, NoDeleting());
}

typedef double (CompositeFunction::*getParameterType1)(size_t) const;
typedef double (CompositeFunction::*getParameterType2)(
    const std::string &) const;

typedef void (CompositeFunction::*setParameterType2)(const std::string &,
                                                     const double &, bool);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(setParameterType2_Overloads,
                                       setParameter, 2, 3)
}

void export_CompositeFunction() {

  register_ptr_to_python<boost::shared_ptr<CompositeFunction>>();

  class_<CompositeFunction, bases<IFunction>, boost::noncopyable>(
      "CompositeFunction", "Composite Fit functions")
      .def("nFunctions", &CompositeFunction::nFunctions, arg("self"),
           "Get the number of member functions.")
      .def("castToComposite", &castToCompositeFunction, arg("self"),
           "REturn pointer to Composite function")
      .def("__len__", &CompositeFunction::nFunctions, arg("self"),
           "Get the number of member functions.")
      .def("getFunction", &CompositeFunction::getFunction,
           (arg("self"), arg("i")), "Get the i-th function.")
      .def("__getitem__", &CompositeFunction::getFunction,
           (arg("self"), arg("i")), "Get the i-th function.")
      .def("__setitem__", &CompositeFunction::replaceFunction,
           (arg("self"), arg("i"), arg("f")),
           "Put function in place of the i-th function.")
      .def("add", &CompositeFunction::addFunction,
           (arg("self"), arg("function")), "Add a member function.")
      .def("getParameterValue",
           (getParameterType1)&CompositeFunction::getParameter,
           (arg("self"), arg("i")), "Get value of parameter of given index.")
      .def("getParameterValue",
           (getParameterType2)&CompositeFunction::getParameter,
           (arg("self"), arg("name")), "Get value of parameter of given name.")
      .def("__getitem__", (getParameterType2)&CompositeFunction::getParameter,
           (arg("self"), arg("name")), "Get value of parameter of given name.")
      .def("__setitem__", (setParameterType2)&CompositeFunction::setParameter,
           setParameterType2_Overloads(
               (arg("self"), arg("name"), arg("value"), arg("explicitlySet")),
               "Get value of parameter of given name."))
      .def("__delitem__", &CompositeFunction::removeFunction,
           (arg("self"), arg("index")));
}
