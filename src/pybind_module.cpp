#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // For automatic conversion of std::vector
#include "telnetlibcpp.hpp" // Include your class definition

namespace py = pybind11;

PYBIND11_MODULE(telnetlibcpp, m) {
    py::class_<TelnetlibCpp>(m, "TelnetlibCpp")
        .def(py::init<const std::string&, serverpp::port_identifier>())
        .def("Readout", &TelnetlibCpp::Readout) // Bind the Readout function
        // Bind the Exec function, using a lambda to handle optional vector<string> args
        .def("Exec", [](TelnetlibCpp& self, const std::string& cmd, const py::list& args) {
            std::vector<std::string> vecArgs;
            for (const auto& arg : args) {
                if (!arg.is_none()) {
                    vecArgs.push_back(arg.cast<std::string>());
                } else {
                    // Optionally, handle None values differently, e.g., by inserting an empty string
                    // vecArgs.push_back("");
                }
            }
            return self.Exec(cmd, vecArgs);
        })
        .def("read_some", &TelnetlibCpp::read_some)
        .def("write", &TelnetlibCpp::write)
        .def("write_raw_sequence", &TelnetlibCpp::write_raw_sequence)
        .def("run", &TelnetlibCpp::run);
}
