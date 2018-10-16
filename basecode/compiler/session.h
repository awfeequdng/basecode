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

#pragma once

#include <map>
#include <cstdio>
#include <string>
#include <vector>
#include <vm/ffi.h>
#include <vm/terp.h>
#include <fmt/format.h>
#include <vm/assembler.h>
#include <common/defer.h>
#include <parser/parser.h>
#include <boost/filesystem.hpp>
#include <common/source_file.h>
#include <vm/default_allocator.h>
#include "variable.h"
#include "element_map.h"
#include "scope_manager.h"
#include "ast_evaluator.h"
#include "compiler_types.h"
#include "element_builder.h"
#include "elements/program.h"
#include "string_intern_map.h"

namespace basecode::compiler {

    class session {
    public:
        session(
            const session_options_t& options,
            const path_list_t& source_files);

        virtual ~session();

        bool run();

        void error(
            const std::string& code,
            const std::string& message,
            const common::source_location& location);

        void error(
            compiler::element* element,
            const std::string& code,
            const std::string& message,
            const common::source_location& location);

        bool variable(
            compiler::element* element,
            variable_handle_t& handle,
            bool activate = true);

        vm::ffi& ffi();

        bool compile();

        void finalize();

        vm::terp& terp();

        bool initialize();

        bool allocate_reg(
            vm::register_t& reg,
            compiler::element* element);

        bool emit_to_temp(
            compiler::element* element,
            vm::op_sizes reg_size,
            vm::register_type_t reg_type);

        element_map& elements();

        common::result& result();

        element_builder& builder();

        vm::assembler& assembler();

        ast_evaluator& evaluator();

        vm::allocator* allocator();

        compiler::program& program();

        void disassemble(FILE* file);

        bool emit_interned_strings();

        vm::stack_frame_t* stack_frame();

        const element_map& elements() const;

        common::source_file* pop_source_file();

        compiler::scope_manager& scope_manager();

        const session_options_t& options() const;

        const compiler::program& program() const;

        common::source_file* current_source_file();

        std::vector<common::source_file*> source_files();

        const compiler::scope_manager& scope_manager() const;

        vm::label_ref_t* type_info_label(compiler::type* type);

        void push_source_file(common::source_file* source_file);

        common::id_t intern_string(compiler::string_literal* literal);

        void type_info_label(compiler::type* type, vm::label_ref_t* label);

        compiler::module* compile_module(common::source_file* source_file);

        syntax::ast_node_shared_ptr parse(common::source_file* source_file);

        syntax::ast_node_shared_ptr parse(const boost::filesystem::path& path);

        std::string intern_data_label(compiler::string_literal* literal) const;

        common::source_file* add_source_file(const boost::filesystem::path& path);

        common::source_file* find_source_file(const boost::filesystem::path& path);

    private:
        void raise_phase(
            session_compile_phase_t phase,
            const boost::filesystem::path& source_file);

        bool type_check();

        bool resolve_unknown_types();

        void initialize_core_types();

        bool fold_constant_intrinsics();

        bool resolve_unknown_identifiers();

        void initialize_built_in_procedures();

        bool should_read_variable(compiler::element* element);

        void write_code_dom_graph(const boost::filesystem::path& path);

    private:
        vm::ffi _ffi;
        vm::terp _terp;
        common::result _result;
        element_builder _builder;
        vm::assembler _assembler;
        element_map _elements {};
        compiler::program _program;
        ast_evaluator _ast_evaluator;
        session_options_t _options {};
        vm::stack_frame_t _stack_frame;
        string_intern_map _interned_strings {};
        compiler::scope_manager _scope_manager;
        std::stack<common::source_file*> _source_file_stack {};
        std::map<std::string, common::source_file> _source_files {};
        std::unordered_map<common::id_t, compiler::variable> _variables {};
        std::unordered_map<common::id_t, vm::label_ref_t*> _type_info_labels {};
    };

};
