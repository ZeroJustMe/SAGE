#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "operator/lambda_map_operator.h"

namespace py = pybind11;

void BindLambdaMapOperator(py::module& m) {
    // Bind MapFunction type
    m.def("MapFunction", [](const py::function& func) {
        return sage_flow::MapFunction([func](const sage_flow::MultiModalMessage& msg) {
            auto result = func(msg);
            // Convert Python result back to C++ type
            return result.cast<std::unique_ptr<sage_flow::MultiModalMessage>>();
        });
    });

    // Bind LambdaMapOperator class
    py::class_<sage_flow::LambdaMapOperator, sage_flow::Operator>(m, "LambdaMapOperator")
        .def(py::init<sage_flow::MapFunction>(), 
             "Create a LambdaMapOperator with a map function")
        .def("process", &sage_flow::LambdaMapOperator::process,
             "Process input record through the map function")
        .def("open", &sage_flow::LambdaMapOperator::open,
             "Open the operator for processing")
        .def("close", &sage_flow::LambdaMapOperator::close,
             "Close the operator");

    // Bind factory function
    m.def("CreateLambdaMap", &sage_flow::CreateLambdaMap,
          "Create a LambdaMapOperator instance",
          py::return_value_policy::take_ownership);
}
