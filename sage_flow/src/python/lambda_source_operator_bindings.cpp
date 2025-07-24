#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "operator/lambda_source_operator.h"

namespace py = pybind11;

void BindLambdaSourceOperator(py::module& m) {
    // Bind GeneratorFunction type
    m.def("GeneratorFunction", [](const py::function& func) {
        return sage_flow::GeneratorFunction([func]() {
            auto result = func();
            if (result.is_none()) {
                return std::unique_ptr<sage_flow::MultiModalMessage>{};
            }
            return result.cast<std::unique_ptr<sage_flow::MultiModalMessage>>();
        });
    });

    // Bind LambdaSourceOperator class
    py::class_<sage_flow::LambdaSourceOperator, sage_flow::Operator>(m, "LambdaSourceOperator")
        .def(py::init<sage_flow::GeneratorFunction, size_t>(), 
             "Create a LambdaSourceOperator with a generator function",
             py::arg("generator"), py::arg("max_messages") = 0)
        .def("process", &sage_flow::LambdaSourceOperator::process,
             "Process and generate messages")
        .def("open", &sage_flow::LambdaSourceOperator::open,
             "Open the operator for processing")
        .def("close", &sage_flow::LambdaSourceOperator::close,
             "Close the operator")
        .def("has_next", &sage_flow::LambdaSourceOperator::hasNext,
             "Check if more messages can be generated")
        .def("get_generated_count", &sage_flow::LambdaSourceOperator::getGeneratedCount,
             "Get the number of messages generated so far");

    // Bind factory function
    m.def("CreateLambdaSource", &sage_flow::CreateLambdaSource,
          "Create a LambdaSourceOperator instance",
          py::arg("generator"), py::arg("max_messages") = 0,
          py::return_value_policy::take_ownership);
}
