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

#include <common/defer.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "type.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "array_type.h"
#include "pointer_type.h"
#include "composite_type.h"
#include "symbol_element.h"
#include "type_reference.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    binary_operator::binary_operator(
            compiler::module* module,
            block* parent_scope,
            operator_type_t type,
            element* lhs,
            element* rhs) : operator_base(module, parent_scope, element_type_t::binary_operator, type),
                            _lhs(lhs),
                            _rhs(rhs) {
    }

    bool binary_operator::on_infer_type(
            compiler::session& session,
            infer_type_result_t& result) {
        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::subscript:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_left:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                return _lhs->infer_type(session, result);
            }
            case operator_type_t::member_access: {
                return _rhs->infer_type(session, result);
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                result.inferred_type = session.scope_manager().find_type({.name = "bool"});
                return true;
            }
            default:
                return false;
        }
    }

    bool binary_operator::on_emit(compiler::session& session) {
        auto& assembler = session.assembler();

        auto block = assembler.current_block();
        block->label(assembler.make_label(fmt::format("{}_begin", label_name())));

        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::shift_left:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                emit_arithmetic_operator(session);
                break;
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                emit_relational_operator(session);
                break;
            }
            case operator_type_t::subscript: {
                block->comment("XXX: implement subscript operator", 4);
                block->nop();
                break;
            }
            case operator_type_t::member_access: {
                variable_handle_t field_var {};
                if (!session.variable(this, field_var))
                    return false;
                auto target_reg = assembler.current_target_register();
                block->move_reg_to_reg(*target_reg, field_var->value_reg());
                break;
            }
            case operator_type_t::assignment: {
                variable_handle_t lhs_var;
                if (!session.variable(_lhs, lhs_var))
                    return false;

                variable_handle_t rhs_var;
                if (!session.variable(_rhs, rhs_var))
                    return false;

                lhs_var->write(rhs_var.get());
                break;
            }
            default:
                break;
        }

        return true;
    }

    bool binary_operator::on_fold(
            compiler::session& session,
            fold_result_t& result) {
        switch (operator_type()) {
            case operator_type_t::add: {
                break;
            }
            case operator_type_t::divide: {
                break;
            }
            case operator_type_t::modulo: {
                break;
            }
            case operator_type_t::equals: {
                break;
            }
            case operator_type_t::subtract: {
                break;
            }
            case operator_type_t::exponent: {
                break;
            }
            case operator_type_t::multiply: {
                break;
            }
            case operator_type_t::binary_or: {
                break;
            }
            case operator_type_t::less_than: {
                break;
            }
            case operator_type_t::not_equals: {
                break;
            }
            case operator_type_t::logical_or: {
                break;
            }
            case operator_type_t::binary_and: {
                break;
            }
            case operator_type_t::binary_xor: {
                break;
            }
            case operator_type_t::shift_left: {
                break;
            }
            case operator_type_t::logical_and: {
                break;
            }
            case operator_type_t::shift_right: {
                break;
            }
            case operator_type_t::rotate_left: {
                break;
            }
            case operator_type_t::member_access: {
                break;
            }
            case operator_type_t::rotate_right: {
                break;
            }
            case operator_type_t::greater_than: {
                break;
            }
            case operator_type_t::less_than_or_equal: {
                break;
            }
            case operator_type_t::greater_than_or_equal: {
                break;
            }
            default:
                break;
        }

        return true;
    }

    element* binary_operator::lhs() {
        return _lhs;
    }

    element* binary_operator::rhs() {
        return _rhs;
    }

    bool binary_operator::on_is_constant() const {
        return (_lhs != nullptr && _lhs->is_constant())
            && (_rhs != nullptr && _rhs->is_constant());
    }

    void binary_operator::lhs(compiler::element* element) {
        _lhs = element;
    }

    void binary_operator::rhs(compiler::element* element) {
        _rhs = element;
    }

    void binary_operator::on_owned_elements(element_list_t& list) {
        if (_lhs != nullptr)
            list.emplace_back(_lhs);
        if( _rhs != nullptr)
            list.emplace_back(_rhs);
    }

    void binary_operator::emit_relational_operator(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();

        auto free_target_reg = false;
        auto clear_target_tag = false;
        defer({
            if (!clear_target_tag)
                return;
            vm::register_t temp_reg;
            if (assembler.remove_tagged_register(
                    register_tags_t::tag_rel_expr_target,
                    temp_reg)) {
                if (free_target_reg)
                    assembler.free_reg(temp_reg);
            }
        });

        auto target_reg = assembler.tagged_register(register_tags_t::tag_rel_expr_target);
        if (target_reg == nullptr) {
            clear_target_tag = true;

            target_reg = assembler.current_target_register();
            if (target_reg == nullptr) {
                if (!assembler.allocate_reg(_temp_reg)) {
                    // XXX: handle error
                }
                target_reg = &_temp_reg;
                free_target_reg = true;
            }

            assembler.tag_register(
                register_tags_t::tag_rel_expr_target,
                target_reg);
            block->clr(vm::op_sizes::qword, *target_reg);
        }

        variable_handle_t lhs_var;
        if (!session.variable(_lhs, lhs_var))
            return;
        lhs_var->read();

        auto end_label_name = fmt::format("{}_end", label_name());
        auto end_label_ref = assembler.make_label_ref(end_label_name);

        auto is_short_circuited = false;
        switch (operator_type()) {
            case operator_type_t::logical_or: {
                is_short_circuited = true;
                block->bnz(*target_reg, end_label_ref);
                break;
            }
            case operator_type_t::logical_and: {
                is_short_circuited = true;
                block->bz(*target_reg, end_label_ref);
                break;
            }
            default: {
                break;
            }
        }

        variable_handle_t rhs_var;
        if (!session.variable(_rhs, rhs_var))
            return;
        rhs_var->read();

        if (!is_short_circuited) {
            block->cmp(lhs_var->value_reg(), rhs_var->value_reg());

            switch (operator_type()) {
                case operator_type_t::equals: {
                    block->setz(*target_reg);
                    break;
                }
                case operator_type_t::less_than: {
                    block->setb(*target_reg);
                    break;
                }
                case operator_type_t::not_equals: {
                    block->setnz(*target_reg);
                    break;
                }
                case operator_type_t::greater_than: {
                    block->seta(*target_reg);
                    break;
                }
                case operator_type_t::less_than_or_equal: {
                    block->setbe(*target_reg);
                    break;
                }
                case operator_type_t::greater_than_or_equal: {
                    block->setae(*target_reg);
                    break;
                }
                default: {
                    break;
                }
            }
        }

        block->label(assembler.make_label(end_label_name));
    }

    void binary_operator::emit_arithmetic_operator(compiler::session& session) {
        auto& assembler = session.assembler();
        auto block = assembler.current_block();
        auto result_reg = assembler.current_target_register();

        vm::register_t target_reg {
            .type = vm::register_type_t::none
        };
        defer({
            if (target_reg.type != vm::register_type_t::none) {
                assembler.free_reg(target_reg);
            }
        });

        variable_handle_t lhs_var;
        if (!session.variable(_lhs, lhs_var))
            return;

        lhs_var->read();

        variable_handle_t rhs_var;
        if (!session.variable(_rhs, rhs_var))
            return;

        rhs_var->read();

        if (result_reg == nullptr) {
            result_reg = &target_reg;
            result_reg->size = lhs_var->value_reg().size;
            result_reg->type = lhs_var->value_reg().type;
            if (!assembler.allocate_reg(*result_reg)) {

            }
        }

        switch (operator_type()) {
            case operator_type_t::add: {
                block->add_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::divide: {
                block->div_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::modulo: {
                block->mod_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::multiply: {
                block->mul_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::exponent: {
                block->pow_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::subtract: {
                block->sub_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::binary_or: {
                block->or_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::shift_left: {
                block->shl_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::binary_and: {
                block->and_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::binary_xor: {
                block->xor_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::rotate_left: {
                block->rol_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::shift_right: {
                block->shr_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            case operator_type_t::rotate_right: {
                block->ror_reg_by_reg(
                    *result_reg,
                    lhs_var->value_reg(),
                    rhs_var->value_reg());
                break;
            }
            default:
                break;
        }
    }

};