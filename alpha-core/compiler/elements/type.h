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

#include <string>
#include <unordered_map>
#include "element.h"

namespace basecode::compiler {

    class type : public element {
    public:
        type(
            block* parent_scope,
            element_type_t type,
            const std::string& name);

        bool initialize(
            common::result& r,
            compiler::program* program);

        bool packed() const;

        void packed(bool value);

        std::string name() const;

        size_t alignment() const;

        size_t size_in_bytes() const;

        void alignment(size_t value);

        void name(const std::string& value);

    protected:
        virtual bool on_initialize(
            common::result& r,
            compiler::program* program);

        void size_in_bytes(size_t value);

    private:
        std::string _name;
        bool _packed = false;
        size_t _alignment = 0;
        size_t _size_in_bytes {};
    };

};