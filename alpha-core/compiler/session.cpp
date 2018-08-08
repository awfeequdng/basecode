// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#include <fstream>
#include "session.h"
#include "elements/program.h"
#include "code_dom_formatter.h"

namespace basecode::compiler {

    session::session(
            const session_options_t& options,
            const path_list_t& source_files) : _terp(options.heap_size, options.stack_size),
                                               _assembler(&_terp),
                                               _program(&_terp, &_assembler),
                                               _options(options) {
        for (const auto& path : source_files) {
            add_source_file(path);
        }
    }

    session::~session() {
    }

    vm::terp& session::terp() {
        return _terp;
    }

    void session::raise_phase(
            session_compile_phase_t phase,
            const boost::filesystem::path& source_file) {
        if (_options.compile_callback == nullptr)
            return;
        _options.compile_callback(phase, source_file);
    }

    void session::finalize() {
        if (_options.verbose) {
            try {
                _program.disassemble(stdout);
                if (!_options.dom_graph_file.empty())
                    write_code_dom_graph(_options.dom_graph_file);
            } catch (const fmt::format_error& e) {
                fmt::print("fmt::format_error caught: {}\n", e.what());
            }
        }
    }

    vm::assembler& session::assembler() {
        return _assembler;
    }

    compiler::program& session::program() {
        return _program;
    }

    bool session::compile(common::result& r) {
        return _program.compile(r, *this);
    }

    bool session::initialize(common::result& r) {
        _terp.initialize(r);
        _assembler.initialize(r);
        return !r.is_failed();
    }

    syntax::ast_node_shared_ptr session::parse(
            common::result& r,
            const boost::filesystem::path& path) {
        auto source_file = find_source_file(path);
        if (source_file == nullptr) {
            source_file = add_source_file(path);
        }
        return parse(r, source_file);
    }

    syntax::ast_node_shared_ptr session::parse(
            common::result& r,
            common::source_file* source_file) {
        if (source_file->empty()) {
            if (!source_file->load(r))
                return nullptr;
        }

        syntax::parser alpha_parser(source_file);
        auto module_node = alpha_parser.parse(r);
        if (module_node != nullptr && !r.is_failed()) {
            if (_options.verbose && !_options.ast_graph_file.empty())
                alpha_parser.write_ast_graph(_options.ast_graph_file, module_node);
        }
        return module_node;
    }

    common::source_file* session::pop_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        auto source_file = _source_file_stack.top();
        _source_file_stack.pop();
        return source_file;
    }

    const session_options_t& session::options() const {
        return _options;
    }

    common::source_file* session::current_source_file() {
        if (_source_file_stack.empty())
            return nullptr;
        return _source_file_stack.top();
    }

    std::vector<common::source_file*> session::source_files() {
        std::vector<common::source_file*> list {};
        for (auto& it : _source_files) {
            list.push_back(&it.second);
        }
        return list;
    }

    void session::push_source_file(common::source_file* source_file) {
        _source_file_stack.push(source_file);
    }

    void session::write_code_dom_graph(const boost::filesystem::path& path) {
        FILE* output_file = nullptr;
        if (!path.empty()) {
            output_file = fopen(path.string().c_str(), "wt");
        }
        defer({
            if (output_file != nullptr)
                fclose(output_file);
        });

        compiler::code_dom_formatter formatter(&_program, output_file);
        formatter.format(fmt::format("Code DOM Graph: {}", path.string()));
    }

    common::source_file* session::add_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.insert(std::make_pair(
            path.string(),
            common::source_file(path)));
        if (!it.second)
            return nullptr;
        return &it.first->second;
    }

    common::source_file* session::find_source_file(const boost::filesystem::path& path) {
        auto it = _source_files.find(path.string());
        if (it == _source_files.end())
            return nullptr;
        return &it->second;
    }

};