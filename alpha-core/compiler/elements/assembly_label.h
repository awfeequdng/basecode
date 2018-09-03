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

    class assembly_label : public element {
    public:
        assembly_label(
            compiler::module* module,
            block* parent_scope,
            const std::string& name);

        std::string name() const;

    protected:
        bool on_infer_type(
            const compiler::session& session,
            infer_type_result_t& result) override;

        bool on_is_constant() const override;

        bool on_emit(compiler::session& session) override;

    private:
        std::string _name;
    };

};

