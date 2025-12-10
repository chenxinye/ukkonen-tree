#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "suffixtree_py.h" 

namespace py = pybind11;

PYBIND11_MODULE(pyukkonen, m) {
    m.doc() = "High-performance Suffix Tree Plugin";

    py::class_<SuffixTree>(m, "SuffixTree")
        .def(py::init<std::string>(), "Initialize with text (automatically appends $ if missing)")
        .def("search", &SuffixTree::search, "Check if pattern exists in text")
        .def("print_tree", &SuffixTree::printTree, "Print tree structure to stdout")
        .def("get_text", &SuffixTree::getText, "Get the original text");
}