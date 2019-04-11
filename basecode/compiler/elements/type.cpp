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
#include "type.h"
#include "field.h"
#include "identifier.h"
#include "initializer.h"
#include "numeric_type.h"
#include "symbol_element.h"

namespace basecode::compiler {

    std::string type::make_info_label_name(compiler::type* type) {
        type_name_builder builder {};
        builder
            .add_part("ti")
            .add_part(type->symbol()->name());
        return builder.format();
    }

    std::string type::make_literal_label_name(compiler::type* type) {
        type_name_builder builder {};
        builder
            .add_part("ti_lit")
            .add_part(type->symbol()->name());
        return builder.format();
    }

    std::string type::make_literal_data_label_name(compiler::type* type) {
        type_name_builder builder {};
        builder
            .add_part(make_literal_label_name(type))
            .add_part("data");
        return builder.format();
    }

    type::type(
            compiler::module* module,
            block* parent_scope,
            element_type_t type,
            compiler::symbol_element* symbol) : element(module, parent_scope, type),
                                                _symbol(symbol) {
    }

    bool type::type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        return on_type_check(other, options);
    }

    bool type::on_type_check(
            compiler::type* other,
            const type_check_options_t& options) {
        return false;
    }

    bool type::packed() const {
        return _packed;
    }

    bool type::is_signed() const {
        return false;
    }

    void type::packed(bool value) {
        _packed = value;
    }

    size_t type::alignment() const {
        return _alignment;
    }

    bool type::is_proc_type() const {
        return false;
    }

    bool type::is_array_type() const {
        return false;
    }

    bool type::is_family_type() const {
        return false;
    }

    bool type::is_pointer_type() const {
        return false;
    }

    void type::alignment(size_t value) {
        _alignment = value;
    }

    bool type::is_unknown_type() const {
        return false;
    }

    size_t type::size_in_bytes() const {
        return _size_in_bytes;
    }

    bool type::is_composite_type() const {
        return false;
    }

    void type::size_in_bytes(size_t value) {
        _size_in_bytes = value;
    }

    bool type::is_open_generic_type() const {
        return false;
    }

    vm::ffi_types_t type::to_ffi_type() const {
        switch (element_type()) {
            case element_type_t::rune_type: {
                return vm::ffi_types_t::char_type;
            }
            case element_type_t::numeric_type: {
                auto numeric_type = dynamic_cast<const compiler::numeric_type*>(this);
                if (numeric_type != nullptr) {
                    if (numeric_type->number_class() == number_class_t::integer) {
                        switch (size_in_bytes()) {
                            case 1:
                                return vm::ffi_types_t::char_type;
                            case 2:
                                return vm::ffi_types_t::short_type;
                            case 4:
                                return vm::ffi_types_t::int_type;
                            case 8:
                                return vm::ffi_types_t::long_long_type;
                            default:
                                return vm::ffi_types_t::void_type;
                        }
                    } else if (numeric_type->number_class() == number_class_t::floating_point) {
                        switch (size_in_bytes()) {
                            case 4:
                                return vm::ffi_types_t::float_type;
                            case 8:
                                return vm::ffi_types_t::double_type;
                            default:
                                return vm::ffi_types_t::void_type;
                        }
                    } else {
                        // XXX: error
                    }
                }
                break;
            }
            case element_type_t::bool_type: {
                return vm::ffi_types_t::bool_type;
            }
            case element_type_t::pointer_type: {
                return vm::ffi_types_t::pointer_type;
            }
            case element_type_t::composite_type: {
                if (size_in_bytes() <= 16) {
                    // XXX: need to revisit this!
                    return vm::ffi_types_t::struct_type;
                } else {
                    return vm::ffi_types_t::pointer_type;
                }
            }
            default: {
                break;
            }
        }

        return vm::ffi_types_t::void_type;
    }

    number_class_t type::number_class() const {
        return on_number_class();
    }

    number_class_t type::on_number_class() const {
        return number_class_t::none;
    }

    compiler::symbol_element* type::symbol() const {
        return _symbol;
    }

    bool type::initialize(compiler::session& session) {
        return on_initialize(session);
    }

    void type::symbol(compiler::symbol_element* value) {
        _symbol = value;
    }

    void type::on_owned_elements(element_list_t& list) {
        if (_symbol != nullptr)
            list.emplace_back(_symbol);
    }

    bool type::on_initialize(compiler::session& session) {
        return true;
    }

}