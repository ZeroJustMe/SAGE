#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "operator/lambda_filter_operator.h"

namespace py = pybind11;

void BindLambdaFilterOperator(py::module& m) {
    // Bind FilterFunction type
    m.def("FilterFunction", [](const py::function& func) {
        return sage_flow::FilterFunction([func](const sage_flow::MultiModalMessage& msg) {
            auto result = func(msg);
            return result.cast<bool>();
        });
    });

    // Bind LambdaFilterOperator class
    py::class_<sage_flow::LambdaFilterOperator, sage_flow::Operator>(m, "LambdaFilterOperator")
        .def(py::init<sage_flow::FilterFunction>(), 
             "Create a LambdaFilterOperator with a filter function")
        .def("process", &sage_flow::LambdaFilterOperator::process,
             "Process input record through the filter function")
        .def("open", &sage_flow::LambdaFilterOperator::open,
             "Open the operator for processing")
        .def("close", &sage_flow::LambdaFilterOperator::close,
             "Close the operator");

    // Bind factory function
    m.def("CreateLambdaFilter", &sage_flow::CreateLambdaFilter,
          "Create a LambdaFilterOperator instance",
          py::return_value_policy::take_ownership);
}
