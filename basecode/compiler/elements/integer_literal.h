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

#include "element.h"

namespace basecode::compiler {

    class integer_literal : public element {
    public:
        integer_literal(
            compiler::module* module,
            block* parent_scope,
            uint64_t value);

        bool is_signed() const;

        uint64_t value() const;

    protected:
        bool on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_emit(compiler::session& session) override;

        bool on_as_integer(uint64_t& value) const override;

    private:
        uint64_t _value;
    };

};
