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

#include <compiler/session.h>
#include <compiler/type_name_builder.h>
#include "tuple_type.h"

namespace basecode::compiler {

    std::string tuple_type::name_for_tuple() {
        type_name_builder builder {};
        builder
            .add_part("tuple")
            .add_part(common::id_pool::instance()->allocate());
        return builder.format();
    }

    ///////////////////////////////////////////////////////////////////////////

    tuple_type::tuple_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::block* scope,
            compiler::symbol_element* symbol) : compiler::composite_type(
                                                    module,
                                                    parent_scope,
                                                    composite_types_t::struct_type,
                                                    scope,
                                                    symbol,
                                                    element_type_t::tuple_type) {
    }

    bool tuple_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        return other != nullptr && other->id() == id();
    }

    bool tuple_type::on_initialize(compiler::session& session) {
        return composite_type::on_initialize(session);
    }

}