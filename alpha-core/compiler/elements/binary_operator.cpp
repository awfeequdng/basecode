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
#include "type.h"
#include "element.h"
#include "program.h"
#include "identifier.h"
#include "symbol_element.h"
#include "binary_operator.h"
#include "integer_literal.h"
#include "identifier_reference.h"

namespace basecode::compiler {

    binary_operator::binary_operator(
        block* parent_scope,
        operator_type_t type,
        element* lhs,
        element* rhs) : operator_base(parent_scope, element_type_t::binary_operator, type),
                        _lhs(lhs),
                        _rhs(rhs) {
    }

    bool binary_operator::on_emit(
            common::result& r,
            emit_context_t& context) {
        auto instruction_block = context.assembler->current_block();
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
                emit_arithmetic_operator(
                    r,
                    context,
                    instruction_block);
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
                emit_relational_operator(
                    r,
                    context,
                    instruction_block);
                break;
            }
            case operator_type_t::assignment: {
                auto var = context.variable_for_element(_lhs);
                if (var == nullptr) {
                    context.program->error(
                        r,
                        _lhs,
                        "P051",
                        fmt::format("missing assembler variable for {}.", _lhs->label_name()),
                        _lhs->location());
                    return false;
                }

                var->make_live(context.assembler);
                defer({
                    var->make_dormat(context.assembler);
                });

                _lhs->emit(r, context);
                var->init(context.assembler, instruction_block);

                vm::register_t rhs_reg;
                rhs_reg.size = var->value_reg.reg.size;
                rhs_reg.type = var->value_reg.reg.type;

                if (!context.assembler->allocate_reg(rhs_reg)) {
                    context.program->error(
                        r,
                        _rhs,
                        "P052",
                        "assembler registers exhausted.",
                        _rhs->location());
                    return false;
                }
                context.assembler->push_target_register(rhs_reg);
                _rhs->emit(r, context);
                var->write(context.assembler, instruction_block);
                context.assembler->pop_target_register();
                context.assembler->free_reg(rhs_reg);
                break;
            }
            default:
                break;
        }
        return true;
    }

    element* binary_operator::on_fold(
            common::result& r,
            compiler::program* program) {
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

        return nullptr;
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

    void binary_operator::emit_relational_operator(
            common::result& r,
            emit_context_t& context,
            vm::instruction_block* instruction_block) {
        auto lhs_reg = register_for(r, context, _lhs);
        auto rhs_reg = register_for(r, context, _rhs);

        if (!lhs_reg.valid || !rhs_reg.valid)
            return;

        context.assembler->push_target_register(lhs_reg.reg);
        _lhs->emit(r, context);
        context.assembler->pop_target_register();

        context.assembler->push_target_register(rhs_reg.reg);
        _rhs->emit(r, context);
        context.assembler->pop_target_register();

        auto if_data = context.top<if_data_t>();
        switch (operator_type()) {
            case operator_type_t::equals: {
                instruction_block->cmp(lhs_reg.reg, rhs_reg.reg);
                if (if_data != nullptr) {
                    auto parent_op = parent_element_as<compiler::binary_operator>();
                    if (parent_op != nullptr
                    &&  parent_op->operator_type() == operator_type_t::logical_and) {
                        instruction_block->bne(if_data->false_branch_label);
                    } else {
                        instruction_block->beq(if_data->true_branch_label);
                    }
                } else {
                    auto target_reg = context.assembler->current_target_register();
                    instruction_block->setz(*target_reg);
                    context.push_scratch_register(*target_reg);
                }
                break;
            }
            case operator_type_t::less_than: {
                break;
            }
            case operator_type_t::not_equals: {
                break;
            }
            case operator_type_t::logical_or: {
                if (if_data != nullptr)
                    instruction_block->jump_direct(if_data->false_branch_label);
                else {
                    auto rhs_target_reg = context.pop_scratch_register();
                    auto lhs_target_reg = context.pop_scratch_register();
                    auto target_reg = context.assembler->current_target_register();
                    instruction_block->or_reg_by_reg(
                        *target_reg,
                        lhs_target_reg,
                        rhs_target_reg);
                }
                break;
            }
            case operator_type_t::logical_and: {
                if (if_data != nullptr)
                    instruction_block->jump_direct(if_data->true_branch_label);
                else {
                    auto rhs_target_reg = context.pop_scratch_register();
                    auto lhs_target_reg = context.pop_scratch_register();
                    auto target_reg = context.assembler->current_target_register();
                    instruction_block->and_reg_by_reg(
                        *target_reg,
                        lhs_target_reg,
                        rhs_target_reg);
                }
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
            default: {
                break;
            }
        }
    }

    void binary_operator::emit_arithmetic_operator(
            common::result& r,
            emit_context_t& context,
            vm::instruction_block* instruction_block) {
        auto result_reg = context.assembler->current_target_register();

        auto lhs_reg = register_for(r, context, _lhs);
        auto rhs_reg = register_for(r, context, _rhs);

        if (!lhs_reg.valid || !rhs_reg.valid)
            return;

        vm::register_t target_reg {
            .type = vm::register_type_t::none
        };
        defer({
            if (target_reg.type != vm::register_type_t::none) {
              context.assembler->free_reg(target_reg);
            }
        });

        if (result_reg == nullptr) {
            result_reg = &target_reg;
            result_reg->size = lhs_reg.size();
            result_reg->type = lhs_reg.reg.type;
            if (!context.assembler->allocate_reg(*result_reg)) {

            }
        }

        context.assembler->push_target_register(lhs_reg.reg);
        _lhs->emit(r, context);
        context.assembler->pop_target_register();

        context.assembler->push_target_register(rhs_reg.reg);
        _rhs->emit(r, context);
        context.assembler->pop_target_register();

        switch (operator_type()) {
            case operator_type_t::add: {
                instruction_block->add_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::divide: {
                instruction_block->div_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::modulo: {
                instruction_block->mod_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::multiply: {
                instruction_block->mul_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::exponent: {
//                instruction_block->pow_ireg_by_ireg_u64(
//                    *result_reg,
//                    lhs_reg.reg,
//                    rhs_reg.reg);
                break;
            }
            case operator_type_t::subtract: {
                instruction_block->sub_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_or: {
                instruction_block->or_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::shift_left: {
                instruction_block->shl_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_and: {
                instruction_block->and_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::binary_xor: {
                instruction_block->xor_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::rotate_left: {
                instruction_block->rol_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::shift_right: {
                instruction_block->shr_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            case operator_type_t::rotate_right: {
                instruction_block->ror_reg_by_reg(
                    *result_reg,
                    lhs_reg.reg,
                    rhs_reg.reg);
                break;
            }
            default:
                break;
        }
    }

    void binary_operator::on_owned_elements(element_list_t& list) {
        if (_lhs != nullptr)
            list.emplace_back(_lhs);
        if( _rhs != nullptr)
            list.emplace_back(_rhs);
    }

    compiler::type* binary_operator::on_infer_type(const compiler::program* program) {
        switch (operator_type()) {
            case operator_type_t::add:
            case operator_type_t::modulo:
            case operator_type_t::divide:
            case operator_type_t::subtract:
            case operator_type_t::multiply:
            case operator_type_t::exponent:
            case operator_type_t::binary_or:
            case operator_type_t::binary_and:
            case operator_type_t::binary_xor:
            case operator_type_t::shift_left:
            case operator_type_t::shift_right:
            case operator_type_t::rotate_left:
            case operator_type_t::rotate_right: {
                auto lhs_type = _lhs->infer_type(program);
                auto rhs_type = _rhs->infer_type(program);
                // XXX: need to type-check and possibly widen here
                return lhs_type;
            }
            case operator_type_t::equals:
            case operator_type_t::less_than:
            case operator_type_t::not_equals:
            case operator_type_t::logical_or:
            case operator_type_t::logical_and:
            case operator_type_t::greater_than:
            case operator_type_t::less_than_or_equal:
            case operator_type_t::greater_than_or_equal: {
                return program->find_type({.name = "bool"});
            }
            default:
                return nullptr;
        }
    }

};