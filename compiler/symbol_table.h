// ----------------------------------------------------------------------------
//
// Basecode Bootstrap Compiler
// Copyright (C) 2018 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE.md file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <set>
#include <parser/ast.h>
#include <common/result.h>

namespace basecode::compiler {

    struct symbol_table_entry_t {
        uint64_t address = 0;
        syntax::ast_node_shared_ptr node = nullptr;
    };

    struct symbol_lookup_result_t {
        bool found = false;
        std::vector<symbol_table_entry_t*> entries {};
    };

    using symbol_multimap = std::unordered_multimap<std::string, symbol_table_entry_t>;

    class symbol_table {
    public:
        symbol_table() = default;

        void put(
            const std::string& name,
            const symbol_table_entry_t& value);

        void clear();

        std::set<std::string> names() const;

        void remove(const std::string& name);

        bool is_defined(const std::string& name);

        symbol_lookup_result_t get(const std::string& name);

    private:
        symbol_multimap _symbols;
    };

};