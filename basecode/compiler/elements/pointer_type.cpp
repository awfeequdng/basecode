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
#include <compiler/element_builder.h>
#include <compiler/type_name_builder.h>
#include "identifier.h"
#include "initializer.h"
#include "numeric_type.h"
#include "pointer_type.h"
#include "symbol_element.h"
#include "type_reference.h"

namespace basecode::compiler {

    std::string pointer_type::name_for_pointer(compiler::type* base_type) {
        type_name_builder builder {};
        builder
            .add_part("ptr")
            .add_part(std::string(base_type->symbol()->name()));
        return builder.format();
    }

    ///////////////////////////////////////////////////////////////////////////

    pointer_type::pointer_type(
            compiler::module* module,
            compiler::block* parent_scope,
            compiler::type_reference* base_type) : compiler::type(
                                                        module,
                                                        parent_scope,
                                                        element_type_t::pointer_type,
                                                        nullptr),
                                                   _base_type_ref(base_type) {
    }

    bool pointer_type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        if (other == nullptr)
            return false;

        switch (other->element_type()) {
            case element_type_t::pointer_type: {
                auto other_pointer_type = dynamic_cast<compiler::pointer_type*>(other);
                if (other_pointer_type->_base_type_ref->is_void())
                    return true;
                return _base_type_ref
                    ->type()
                    ->type_check(other_pointer_type->_base_type_ref->type(), options);
            }
            case element_type_t::numeric_type: {
                auto numeric_type = dynamic_cast<compiler::numeric_type*>(other);
                return numeric_type->size_in_bytes() == 8;
            }
            default: {
                return false;
            }
        }
    }

    bool pointer_type::is_pointer_type() const {
        return true;
    }

    bool pointer_type::is_unknown_type() const {
        return _base_type_ref != nullptr && _base_type_ref->is_unknown_type();
    }

    bool pointer_type::is_composite_type() const {
        return _base_type_ref->type()->is_composite_type();
    }

    number_class_t pointer_type::on_number_class() const {
        return number_class_t::integer;
    }

    bool pointer_type::on_initialize(compiler::session& session) {
        auto it = session.strings().insert(name_for_pointer(_base_type_ref->type()));
        auto type_symbol = session.builder().make_symbol(parent_scope(), *it.first);
        symbol(type_symbol);
        type_symbol->parent_element(this);
        alignment(sizeof(uint64_t));
        size_in_bytes(sizeof(uint64_t));
        return true;
    }

    compiler::type_reference* pointer_type::base_type_ref() const {
        auto current = this;
        while (true) {
            if (!current->_base_type_ref->is_pointer_type())
                break;
            current = dynamic_cast<compiler::pointer_type*>(current->_base_type_ref->type());
        }
        return current->_base_type_ref;
    }

    void pointer_type::base_type_ref(compiler::type_reference* value) {
        auto current = this;
        while (true) {
            if (!current->_base_type_ref->is_pointer_type())
                break;
            current = dynamic_cast<compiler::pointer_type*>(current->_base_type_ref->type());
        }
        current->_base_type_ref = value;
        current->_base_type_ref->parent_element(this);
    }

}
