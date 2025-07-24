/**
 * @file python_datastream_bindings.cpp
 * @brief Python bindings for SAGE DataStream API
 * 
 * This file provides Python bindings for the DataStream API, enabling
 * seamless integration with sage_examples and sage_core Python code.
 */

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <memory>
#include <string>
#include <functional>
#include <algorithm>

// Include the base operator for bindings
#include "operator/base_operator.h"
#include "operator/operator_types.h"

// Forward declarations for binding functions
void BindLambdaMapOperator(pybind11::module& m);
void BindLambdaFilterOperator(pybind11::module& m);
void BindLambdaSourceOperator(pybind11::module& m);
void BindTerminalSinkOperator(pybind11::module& m);
void BindFileSinkOperator(pybind11::module& m);
void BindVectorStoreSinkOperator(pybind11::module& m);

// For now, we'll bind our mock classes
// Later this will include actual SAGE framework components

namespace sage_flow {

/**
 * @brief Python-friendly MockMultiModalMessage
 */
class PyMultiModalMessage {
public:
    explicit PyMultiModalMessage(std::string content) : content_(std::move(content)), uid_(next_uid++) {}
    
    auto getContent() const -> const std::string& { return content_; }
    auto getUid() const -> uint64_t { return uid_; }
    auto setContent(std::string content) -> void { content_ = std::move(content); }
    
    // Python-friendly representation
    auto __repr__() const -> std::string {
        return "PyMultiModalMessage(uid=" + std::to_string(uid_) + ", content='" + content_ + "')";
    }
    
private:
    std::string content_;
    uint64_t uid_;
    static uint64_t next_uid;
};

uint64_t PyMultiModalMessage::next_uid = 1;

/**
 * @brief Python-friendly DataStream API
 */
class PyDataStream {
public:
    PyDataStream() = default;
    
    auto from_source(std::function<std::shared_ptr<PyMultiModalMessage>()> generator) -> PyDataStream& {
        source_func_ = std::move(generator);
        return *this;
    }
    
    auto map(std::function<std::shared_ptr<PyMultiModalMessage>(const PyMultiModalMessage&)> transform) -> PyDataStream& {
        map_funcs_.push_back(std::move(transform));
        return *this;
    }
    
    auto filter(std::function<bool(const PyMultiModalMessage&)> predicate) -> PyDataStream& {
        filter_funcs_.push_back(std::move(predicate));
        return *this;
    }
    
    auto sink(std::function<void(const PyMultiModalMessage&)> output) -> void {
        sink_func_ = std::move(output);
        execute();
    }
    
private:
    auto execute() -> void {
        // Similar execution logic as the C++ version
        std::vector<std::shared_ptr<PyMultiModalMessage>> messages;
        
        // Generate source messages
        for (int i = 0; i < 5; ++i) {
            if (source_func_) {
                auto msg = source_func_();
                if (msg) {
                    messages.push_back(std::move(msg));
                }
            }
        }
        
        // Apply transformations
        for (const auto& transform : map_funcs_) {
            std::vector<std::shared_ptr<PyMultiModalMessage>> transformed;
            for (const auto& msg : messages) {
                if (msg) {
                    auto new_msg = transform(*msg);
                    if (new_msg) {
                        transformed.push_back(std::move(new_msg));
                    }
                }
            }
            messages = std::move(transformed);
        }
        
        // Apply filters
        auto it = std::remove_if(messages.begin(), messages.end(),
            [this](const std::shared_ptr<PyMultiModalMessage>& msg) {
                if (!msg) {
                    return true;
                }
                // Check if any filter rejects the message
                return std::any_of(filter_funcs_.begin(), filter_funcs_.end(),
                    [&msg](const auto& filter) {
                        return !filter(*msg);
                    });
            });
        messages.erase(it, messages.end());
        
        // Send to sink
        if (sink_func_) {
            for (const auto& msg : messages) {
                if (msg) {
                    sink_func_(*msg);
                }
            }
        }
    }
    
    std::function<std::shared_ptr<PyMultiModalMessage>()> source_func_;
    std::vector<std::function<std::shared_ptr<PyMultiModalMessage>(const PyMultiModalMessage&)>> map_funcs_;
    std::vector<std::function<bool(const PyMultiModalMessage&)>> filter_funcs_;
    std::function<void(const PyMultiModalMessage&)> sink_func_;
};

/**
 * @brief Python-friendly Environment
 */
class PyEnvironment {
public:
    auto create_datastream() -> PyDataStream {
        return PyDataStream{};
    }
};

}  // namespace sage_flow

namespace py = pybind11;

PYBIND11_MODULE(sage_flow_datastream, m) {
    m.doc() = "SAGE Flow DataStream API - Python bindings";
    
    // PyMultiModalMessage bindings
    py::class_<sage_flow::PyMultiModalMessage>(m, "MultiModalMessage")
        .def(py::init<std::string>(), "Create a new MultiModalMessage")
        .def("get_content", &sage_flow::PyMultiModalMessage::getContent, 
             "Get the content of the message")
        .def("set_content", &sage_flow::PyMultiModalMessage::setContent,
             "Set the content of the message")
        .def("get_uid", &sage_flow::PyMultiModalMessage::getUid,
             "Get the unique ID of the message")
        .def("__repr__", &sage_flow::PyMultiModalMessage::__repr__);
    
    // PyDataStream bindings  
    py::class_<sage_flow::PyDataStream>(m, "DataStream")
        .def(py::init<>(), "Create a new DataStream")
        .def("from_source", &sage_flow::PyDataStream::from_source,
             "Set the source function for the stream", py::return_value_policy::reference_internal)
        .def("map", &sage_flow::PyDataStream::map,
             "Add a transformation function to the stream", py::return_value_policy::reference_internal)
        .def("filter", &sage_flow::PyDataStream::filter,
             "Add a filter function to the stream", py::return_value_policy::reference_internal)
        .def("sink", &sage_flow::PyDataStream::sink,
             "Set the sink function and execute the stream");
    
    // PyEnvironment bindings
    py::class_<sage_flow::PyEnvironment>(m, "Environment")
        .def(py::init<>(), "Create a new Environment")
        .def("create_datastream", &sage_flow::PyEnvironment::create_datastream,
             "Create a new DataStream");
    
    // Convenience functions
    m.def("create_message", [](const std::string& content) {
        return std::make_shared<sage_flow::PyMultiModalMessage>(content);
    }, "Create a new MultiModalMessage");
    
    // Bind base Operator class (required for inheritance)
    py::enum_<sage_flow::OperatorType>(m, "OperatorType")
        .value("kSource", sage_flow::OperatorType::kSource)
        .value("kMap", sage_flow::OperatorType::kMap)
        .value("kFilter", sage_flow::OperatorType::kFilter)
        .value("kSink", sage_flow::OperatorType::kSink);
    
    py::class_<sage_flow::Operator>(m, "Operator")
        .def("get_type", &sage_flow::Operator::getType,
             "Get the operator type")
        .def("get_name", &sage_flow::Operator::getName,
             "Get the operator name")
        .def("set_name", &sage_flow::Operator::setName,
             "Set the operator name")
        .def("get_processed_count", &sage_flow::Operator::getProcessedCount,
             "Get number of processed records")
        .def("get_output_count", &sage_flow::Operator::getOutputCount,
             "Get number of output records")
        .def("reset_counters", &sage_flow::Operator::resetCounters,
             "Reset performance counters");
    
    // Bind Lambda operators
    BindLambdaMapOperator(m);
    BindLambdaFilterOperator(m);
    BindLambdaSourceOperator(m);
    
    // Bind Sink operators
    BindTerminalSinkOperator(m);
    BindFileSinkOperator(m);
    BindVectorStoreSinkOperator(m);
}
