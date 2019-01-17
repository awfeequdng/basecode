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

#include <fmt/format.h>
#include <compiler/session.h>
#include <vm/instruction_block.h>
#include "elements.h"
#include "byte_code_emitter.h"

namespace basecode::compiler {

    byte_code_emitter::byte_code_emitter(compiler::session& session) : _session(session) {
    }

    ///////////////////////////////////////////////////////////////////////////

    vm::instruction_block* byte_code_emitter::pop_block() {
        if (_block_stack.empty())
            return nullptr;

        auto top = _block_stack.top();
        _block_stack.pop();
        return top;
    }

    vm::instruction_block* byte_code_emitter::current_block() {
        if (_block_stack.empty())
            return nullptr;

        return _block_stack.top();
    }

    void byte_code_emitter::push_block(vm::instruction_block* block) {
        _block_stack.push(block);
    }

    ///////////////////////////////////////////////////////////////////////////

    bool byte_code_emitter::emit() {
        identifier_by_section_t vars {};
        vars.insert(std::make_pair(vm::section_t::bss,     element_list_t()));
        vars.insert(std::make_pair(vm::section_t::ro_data, element_list_t()));
        vars.insert(std::make_pair(vm::section_t::data,    element_list_t()));
        vars.insert(std::make_pair(vm::section_t::text,    element_list_t()));

        intern_string_literals();

        if (!emit_bootstrap_block())
            return false;

        if (!emit_type_table())
            return false;

        if (!emit_interned_string_table())
            return false;

        if (!emit_section_tables(vars))
            return false;

        if (!emit_procedure_types())
            return false;

        if (!emit_start_block())
            return false;

        if (!emit_initializers(vars))
            return false;

//        if (!emit_implicit_blocks())
//            return false;

        if (!emit_finalizers(vars))
            return false;

        return emit_end_block();
    }

    bool byte_code_emitter::emit_element(
            vm::instruction_block* block,
            compiler::element* e,
            emit_result_t& result) {
        auto& assembler = _session.assembler();

        switch (e->element_type()) {
            case element_type_t::cast: {
                break;
            }
            case element_type_t::if_e: {
                break;
            }
            case element_type_t::with: {
                break;
            }
            case element_type_t::for_e: {
                break;
            }
            case element_type_t::label: {
                break;
            }
            case element_type_t::block: {
                break;
            }
            case element_type_t::field: {
                break;
            }
            case element_type_t::defer: {
                break;
            }
            case element_type_t::symbol: {
                break;
            }
            case element_type_t::module: {
                break;
            }
            case element_type_t::case_e: {
                break;
            }
            case element_type_t::break_e: {
                break;
            }
            case element_type_t::comment: {
                break;
            }
            case element_type_t::program: {
                break;
            }
            case element_type_t::while_e: {
                break;
            }
            case element_type_t::element: {
                break;
            }
            case element_type_t::return_e: {
                break;
            }
            case element_type_t::import_e: {
                break;
            }
            case element_type_t::switch_e: {
                break;
            }
            case element_type_t::raw_block: {
                break;
            }
            case element_type_t::intrinsic: {
                break;
            }
            case element_type_t::directive: {
                break;
            }
            case element_type_t::attribute: {
                break;
            }
            case element_type_t::statement: {
                break;
            }
            case element_type_t::proc_call: {
                break;
            }
            case element_type_t::transmute: {
                break;
            }
            case element_type_t::continue_e: {
                break;
            }
            case element_type_t::identifier: {
                break;
            }
            case element_type_t::expression: {
                auto expr = dynamic_cast<compiler::expression*>(e);
                auto root = expr->root();
                if (root != nullptr)
                    return emit_element(block, root, result);
                break;
            }
            case element_type_t::assignment: {
                break;
            }
            case element_type_t::declaration: {
                break;
            }
            case element_type_t::namespace_e: {
                break;
            }
            case element_type_t::initializer: {
                auto init = dynamic_cast<compiler::initializer*>(e);
                auto expr = init->expression();
                if (expr != nullptr)
                    return emit_element(block, expr, result);
                break;
            }
            case element_type_t::fallthrough: {
                break;
            }
            case element_type_t::nil_literal: {
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<uint64_t>(0),
                    vm::op_sizes::qword));
                break;
            }
            case element_type_t::type_literal: {
                break;
            }
            case element_type_t::float_literal: {
                auto float_literal = dynamic_cast<compiler::float_literal*>(e);
                auto value = float_literal->value();
                auto is_float = numeric_type::narrow_to_value(value) == "f32";
                if (is_float) {
                    float temp_value = static_cast<float>(value);
                    result.operands.emplace_back(vm::instruction_operand_t(temp_value));
                } else {
                    result.operands.emplace_back(vm::instruction_operand_t(value));
                }
                break;
            }
            case element_type_t::string_literal: {
                result.operands.emplace_back(vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    interned_string_data_label(e->id()))));
                break;
            }
            case element_type_t::boolean_literal: {
                auto bool_literal = dynamic_cast<compiler::boolean_literal*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<uint64_t>(bool_literal->value() ? 1 : 0),
                    vm::op_sizes::byte));
                break;
            }
            case element_type_t::integer_literal: {
                auto integer_literal = dynamic_cast<compiler::integer_literal*>(e);
                infer_type_result_t type_result {};
                if (!e->infer_type(_session, type_result))
                    return false;
                result.operands.emplace_back(vm::instruction_operand_t(
                    integer_literal->value(),
                    vm::op_size_for_byte_size(type_result.inferred_type->size_in_bytes())));
                break;
            }
            case element_type_t::character_literal: {
                auto char_literal = dynamic_cast<compiler::character_literal*>(e);
                result.operands.emplace_back(vm::instruction_operand_t(
                    static_cast<int64_t>(char_literal->rune()),
                    vm::op_sizes::dword));
                break;
            }
            case element_type_t::uninitialized_literal: {
                break;
            }
            case element_type_t::rune_type:
            case element_type_t::proc_type:
            case element_type_t::bool_type:
            case element_type_t::tuple_type:
            case element_type_t::array_type:
            case element_type_t::module_type:
            case element_type_t::unknown_type:
            case element_type_t::numeric_type:
            case element_type_t::pointer_type:
            case element_type_t::generic_type:
            case element_type_t::namespace_type:
            case element_type_t::composite_type: {
                break;
            }
            case element_type_t::argument_pair: {
                break;
            }
            case element_type_t::argument_list: {
                break;
            }
            case element_type_t::proc_instance: {
                break;
            }
            case element_type_t::assembly_label: {
                break;
            }
            case element_type_t::unary_operator: {
                break;
            }
            case element_type_t::type_reference: {
                break;
            }
            case element_type_t::binary_operator: {
                break;
            }
            case element_type_t::spread_operator: {
                break;
            }
            case element_type_t::label_reference: {
                break;
            }
            case element_type_t::module_reference: {
                break;
            }
            case element_type_t::unknown_identifier: {
                break;
            }
            case element_type_t::identifier_reference: {
                break;
            }
            case element_type_t::assembly_literal_label: {
                break;
            }
        }
        return true;
    }

    bool byte_code_emitter::emit_type_info(
            vm::instruction_block* block,
            compiler::type* type) {
        if (type == nullptr)
            return false;

        if (type->element_type() == element_type_t::generic_type
        ||  type->element_type() == element_type_t::unknown_type) {
            return true;
        }

        auto& assembler = _session.assembler();

        auto type_name = type->name();
        auto type_name_len = static_cast<uint32_t>(type_name.length());
        auto label_name = type::make_info_label_name(type);

        block->blank_line();
        block->comment(fmt::format("type: {}", type_name), 0);
        block->label(assembler.make_label(label_name));

        block->dwords({type_name_len});
        block->dwords({type_name_len});
        block->qwords({assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            type::make_literal_data_label_name(type))});

        return true;
    }

    bool byte_code_emitter::emit_type_table() {
        auto& assembler = _session.assembler();

        auto type_info_block = assembler.make_basic_block();
        type_info_block->section(vm::section_t::ro_data);

        auto used_types = _session.used_types();
        for (auto type : used_types) {
            type_info_block->blank_line();
            type_info_block->align(4);
            type_info_block->string(
                assembler.make_label(compiler::type::make_literal_label_name(type)),
                assembler.make_label(compiler::type::make_literal_data_label_name(type)),
                type->name());
        }

        type_info_block->blank_line();
        type_info_block->align(8);
        type_info_block->label(assembler.make_label("_ti_array"));
        type_info_block->qwords({used_types.size()});
        for (auto type : used_types) {
            emit_type_info(type_info_block, type);
        }

        return true;
    }

    bool byte_code_emitter::emit_bootstrap_block() {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->jump_direct(vm::instruction_operand_t(assembler.make_named_ref(
            vm::assembler_named_ref_type_t::label,
            "_start")));

        return true;
    }

    void byte_code_emitter::intern_string_literals() {
        auto literals = _session
            .elements()
            .find_by_type<compiler::string_literal>(element_type_t::string_literal);
        for (auto literal : literals) {
            if (literal->is_parent_type_one_of({
                    element_type_t::attribute,
                    element_type_t::directive,
                    element_type_t::module_reference})) {
                continue;
            }

            _session.intern_string(literal);
        }
    }

    bool byte_code_emitter::emit_interned_string_table() {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->comment("interned string literals", 0);
        block->section(vm::section_t::ro_data);

        auto interned_strings = _session.interned_strings();
        for (const auto& kvp : interned_strings) {
            block->blank_line();
            block->align(4);
            block->comment(
                fmt::format("\"{}\"", kvp.first),
                0);

            std::string escaped {};
            if (!compiler::string_literal::escape(kvp.first, escaped)) {
                _session.error(
                    nullptr,
                    "X000",
                    fmt::format("invalid escape sequence: {}", kvp.first),
                    {});
                return false;
            }

            block->string(
                assembler.make_label(fmt::format("_intern_str_lit_{}", kvp.second)),
                assembler.make_label(fmt::format("_intern_str_lit_{}_data", kvp.second)),
                escaped);
        }

        return true;
    }

    bool byte_code_emitter::emit_section_tables(identifier_by_section_t& vars) {
        if (!group_identifiers(vars))
            return false;

        auto& assembler = _session.assembler();
        auto block = assembler.make_basic_block();

        for (const auto& section : vars) {
            block->blank_line();
            block->section(section.first);

            for (auto e : section.second)
                emit_section_variable(block, e);
        }

        return true;
    }

    element_list_t* byte_code_emitter::variable_section(
            identifier_by_section_t& groups,
            vm::section_t section) {
        auto it = groups.find(section);
        if (it == groups.end()) {
            return nullptr;
        }
        return &it->second;
    }

    bool byte_code_emitter::emit_end_block() {
        auto& assembler = _session.assembler();

        auto end_block = assembler.make_basic_block();
        end_block->align(vm::instruction_t::alignment);
        end_block->label(assembler.make_label("_end"));
        end_block->exit();

        return true;
    }

    bool byte_code_emitter::emit_start_block() {
        auto& assembler = _session.assembler();

        auto start_block = assembler.make_basic_block();
        start_block->align(vm::instruction_t::alignment);
        start_block->label(assembler.make_label("_start"));

        start_block->move(
            vm::instruction_operand_t::fp(),
            vm::instruction_operand_t::sp());

        auto address_registers = _session.address_registers();
        for (auto kvp : address_registers) {
            auto var = dynamic_cast<compiler::identifier*>(_session.elements().find(kvp.first));
            start_block->comment(
                var->label_name(),
                vm::comment_location_t::after_instruction);
            start_block->move(
                vm::instruction_operand_t(kvp.second),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        return true;
    }

    bool byte_code_emitter::emit_implicit_blocks() {
        auto& assembler = _session.assembler();

        block_list_t implicit_blocks {};
        auto module_refs = _session
            .elements()
            .find_by_type<compiler::module_reference>(element_type_t::module_reference);
        for (auto mod_ref : module_refs) {
            auto block = mod_ref->reference()->scope();
            // XXX: how can we check a block to determine if it will emit byte code?
            //      if it won't, then don't add it here
            implicit_blocks.emplace_back(block);
        }
        implicit_blocks.emplace_back(_session.program().module()->scope());

        for (auto block : implicit_blocks) {
            auto implicit_block = assembler.make_basic_block();
            implicit_block->blank_line();

            auto parent_element = block->parent_element();
            switch (parent_element->element_type()) {
                case element_type_t::namespace_e: {
                    auto parent_ns = dynamic_cast<compiler::namespace_element*>(parent_element);
                    implicit_block->comment(fmt::format(
                        "namespace: {}",
                        parent_ns->name()));
                    break;
                }
                case element_type_t::module: {
                    auto parent_module = dynamic_cast<compiler::module*>(parent_element);
                    implicit_block->comment(fmt::format(
                        "module: {}",
                        parent_module->source_file()->path().string()));
                    break;
                }
                default:
                    break;
            }

            implicit_block->label(assembler.make_label(block->label_name()));

            //assembler.push_block(implicit_block);
            //emit_result_t result {};
            // XXX: do we want these to be stack frames?
            //block->activate_stack_frame();
            //block->emit(_session, context, result);
            //assembler.pop_block();
        }

        return true;
    }

    bool byte_code_emitter::emit_procedure_types() {
        procedure_instance_set_t proc_instance_set {};

        auto proc_calls = _session
            .elements()
            .find_by_type<compiler::procedure_call>(element_type_t::proc_call);
        for (auto proc_call : proc_calls) {
            if (proc_call->is_foreign())
                continue;

            auto proc_type = proc_call->procedure_type();
            if (proc_type == nullptr)
                return false;

            auto instance = proc_type->instance_for(_session, proc_call);
            if (instance != nullptr)
                proc_instance_set.insert(instance);
        }

        if (_session.result().is_failed())
            return false;

//        auto& assembler = _session.assembler();
//        for (auto instance : proc_instance_set) {
//            auto proc_type_block = assembler.make_basic_block();
//            assembler.push_block(proc_type_block);
//            emit_context_t context {};
//            emit_result_t result(assembler);
//            instance->emit(_session, context, result);
//            assembler.pop_block();
//        }

        return true;
    }

    bool byte_code_emitter::emit_finalizers(identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label("_finalizer"));

        push_block(block);
        defer(pop_block());

        std::vector<compiler::identifier*> to_finalize {};
        for (const auto& section : vars) {
            for (compiler::element* e : section.second) {
                auto var = dynamic_cast<compiler::identifier*>(e);
                if (var == nullptr)
                    continue;

                if (!var->type_ref()->is_composite_type()) {
                    continue;
                }

                auto local_type = number_class_to_local_type(var->type_ref()->type()->number_class());
                block->local(local_type, var->symbol()->name());
                to_finalize.emplace_back(var);
            }
        }

        block->blank_line();

        for (auto var : to_finalize) {
            block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->symbol()->name())),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        for (auto var : to_finalize) {
            emit_finalizer(block, var);
        }

        return true;
    }

    bool byte_code_emitter::group_identifiers(identifier_by_section_t& vars) {
        auto& scope_manager = _session.scope_manager();

        auto ro_list = variable_section(vars, vm::section_t::ro_data);
        auto data_list = variable_section(vars, vm::section_t::data);

        std::set<common::id_t> processed_identifiers {};

        auto identifier_refs = _session
            .elements()
            .find_by_type<compiler::identifier_reference>(element_type_t::identifier_reference);
        for (auto ref : identifier_refs) {
            auto var = ref->identifier();
            if (processed_identifiers.count(var->id()) > 0)
                continue;

            processed_identifiers.insert(var->id());

            if (scope_manager.within_local_scope(var->parent_scope()))
                continue;

            auto var_parent = var->parent_element();
            if (var_parent != nullptr
            &&  var_parent->is_parent_type_one_of({element_type_t::field})) {
                continue;
            }

            auto var_type = var->type_ref()->type();
            if (var_type == nullptr) {
                // XXX: this is an error!
                return false;
            }

            if (var_type->element_type() == element_type_t::generic_type)
                continue;

            auto init = var->initializer();
            if (init != nullptr) {
                switch (init->expression()->element_type()) {
                    case element_type_t::directive: {
                        auto directive = dynamic_cast<compiler::directive*>(init->expression());
                        if (directive->name() == "type")
                            continue;
                    }
                    case element_type_t::proc_type:
                    case element_type_t::composite_type:
                    case element_type_t::type_reference:
                    case element_type_t::module_reference:
                        continue;
                    default:
                        break;
                }
            }

            if (var_type->element_type() == element_type_t::namespace_type
            ||  var_type->element_type() == element_type_t::module_reference) {
                continue;
            }

            if (var->is_constant()) {
                ro_list->emplace_back(var);
            } else {
                data_list->emplace_back(var);
            }
        }

        return true;
    }

    bool byte_code_emitter::emit_initializers(identifier_by_section_t& vars) {
        auto& assembler = _session.assembler();

        auto block = assembler.make_basic_block();
        block->align(vm::instruction_t::alignment);
        block->label(assembler.make_label("_initializer"));

        push_block(block);
        defer(pop_block());

        std::vector<compiler::identifier*> to_init {};
        for (const auto& section : vars) {
            for (compiler::element* e : section.second) {
                auto var = dynamic_cast<compiler::identifier*>(e);
                if (var == nullptr)
                    continue;

                if (var->is_constant()
                && !var->type_ref()->is_composite_type()) {
                    continue;
                }

                auto init = var->initializer();
                if (init != nullptr) {
                    if (init->expression()->element_type() == element_type_t::uninitialized_literal)
                        continue;
                }

                auto local_type = number_class_to_local_type(var->type_ref()->type()->number_class());
                block->local(local_type, var->symbol()->name());
                to_init.emplace_back(var);
            }
        }

        block->blank_line();

        for (auto var : to_init) {
            block->move(
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::local,
                    var->symbol()->name())),
                vm::instruction_operand_t(assembler.make_named_ref(
                    vm::assembler_named_ref_type_t::label,
                    var->label_name())));
        }

        for (auto var : to_init) {
            emit_initializer(block, var);
        }

        return true;
    }

    bool byte_code_emitter::emit_section_variable(
            vm::instruction_block* block,
            compiler::element* e) {
        auto& assembler = _session.assembler();

        switch (e->element_type()) {
            case element_type_t::type_literal: {
                auto type_literal = dynamic_cast<compiler::type_literal*>(e);
                block->blank_line();
                block->align(4);
                auto var_label = assembler.make_label(type_literal->label_name());
                block->label(var_label);
                // XXX: emit data
                break;
            }
            case element_type_t::identifier: {
                auto var = dynamic_cast<compiler::identifier*>(e);

                auto var_type = var->type_ref()->type();
                auto init = var->initializer();

                block->blank_line();

                auto type_alignment = static_cast<uint8_t>(var_type->alignment());
                if (type_alignment > 1)
                    block->align(type_alignment);

                block->comment(fmt::format(
                    "identifier type: {}",
                    var->type_ref()->name()));
                auto var_label = assembler.make_label(var->label_name());
                block->label(var_label);

                switch (var_type->element_type()) {
                    case element_type_t::bool_type: {
                        bool value = false;
                        var->as_bool(value);

                        if (init == nullptr)
                            block->reserve_byte(1);
                        else
                            block->bytes({static_cast<uint8_t>(value ? 1 : 0)});
                        break;
                    }
                    case element_type_t::rune_type: {
                        common::rune_t value = common::rune_invalid;
                        var->as_rune(value);

                        if (init == nullptr)
                            block->reserve_byte(4);
                        else
                            block->dwords({static_cast<uint32_t>(value)});
                        break;
                    }
                    case element_type_t::pointer_type: {
                        if (init == nullptr)
                            block->reserve_qword(1);
                        else
                            block->qwords({0});
                        break;
                    }
                    case element_type_t::numeric_type: {
                        uint64_t value = 0;
                        auto symbol_type = vm::integer_symbol_type_for_size(var_type->size_in_bytes());

                        if (var_type->number_class() == type_number_class_t::integer) {
                            var->as_integer(value);
                        } else {
                            double temp = 0;
                            if (var->as_float(temp)) {
                                vm::register_value_alias_t alias {};
                                if (symbol_type == vm::symbol_type_t::u32)
                                    alias.dwf = static_cast<float>(temp);
                                else
                                    alias.qwf = temp;
                                value = alias.qw;
                            }
                        }

                        switch (symbol_type) {
                            case vm::symbol_type_t::u8:
                                if (init == nullptr)
                                    block->reserve_byte(1);
                                else
                                    block->bytes({static_cast<uint8_t>(value)});
                                break;
                            case vm::symbol_type_t::u16:
                                if (init == nullptr)
                                    block->reserve_word(1);
                                else
                                    block->words({static_cast<uint16_t>(value)});
                                break;
                            case vm::symbol_type_t::f32:
                            case vm::symbol_type_t::u32:
                                if (init == nullptr)
                                    block->reserve_dword(1);
                                else
                                    block->dwords({static_cast<uint32_t>(value)});
                                break;
                            case vm::symbol_type_t::f64:
                            case vm::symbol_type_t::u64:
                                if (init == nullptr)
                                    block->reserve_qword(1);
                                else
                                    block->qwords({value});
                                break;
                            case vm::symbol_type_t::bytes:
                                break;
                            default:
                                break;
                        }
                        break;
                    }
                    case element_type_t::array_type:
                    case element_type_t::tuple_type:
                    case element_type_t::composite_type: {
                        block->reserve_byte(var_type->size_in_bytes());
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            default:
                break;
        }

        return true;
    }

    bool byte_code_emitter::emit_primitive_initializer(
            vm::instruction_block* block,
            const vm::instruction_operand_t& base_local,
            compiler::identifier* var,
            int64_t offset) {
        auto var_type = var->type_ref()->type();
        auto init = var->initializer();

        uint64_t default_value = var_type->element_type() == element_type_t::rune_type ?
                                 common::rune_invalid :
                                 0;
        vm::instruction_operand_t value(
            default_value,
            vm::op_size_for_byte_size(var_type->size_in_bytes()));
        vm::instruction_operand_t* value_ptr = &value;

        emit_result_t result {};
        if (init != nullptr) {
            if (!emit_element(block, init, result))
                return false;
            value_ptr = &result.operands.back();
        }

        block->comment(
            fmt::format("initializer: {}: {}", var->symbol()->name(), var_type->name()),
            vm::comment_location_t::after_instruction);
        block->store(
            base_local,
            *value_ptr,
            vm::instruction_operand_t::offset(offset));
        return true;
    }

    bool byte_code_emitter::emit_finalizer(
            vm::instruction_block* block,
            compiler::identifier* var) {
        auto var_type = var->type_ref()->type();

        block->comment(
            fmt::format("finalizer: {}: {}", var->symbol()->name(), var_type->name()),
            4);

        return true;
    }

    bool byte_code_emitter::emit_initializer(
            vm::instruction_block* block,
            compiler::identifier* var) {
        vm::instruction_operand_t base_local(_session.assembler().make_named_ref(
            vm::assembler_named_ref_type_t::local,
            var->symbol()->name()));

        std::vector<compiler::identifier*> list {};
        list.emplace_back(var);

        uint64_t offset = 0;

        while (!list.empty()) {
            auto next_var = list.front();
            list.erase(std::begin(list));

            auto var_type = next_var->type_ref()->type();

            switch (var_type->element_type()) {
                case element_type_t::rune_type:
                case element_type_t::bool_type:
                case element_type_t::numeric_type:
                case element_type_t::pointer_type: {
                    if (!emit_primitive_initializer(block, base_local, next_var, offset))
                        return false;
                    offset += var_type->size_in_bytes();
                    break;
                }
                case element_type_t::tuple_type:
                case element_type_t::composite_type: {
                    auto composite_type = dynamic_cast<compiler::composite_type*>(var_type);
                    switch (composite_type->type()) {
                        case composite_types_t::enum_type: {
                            if (!emit_primitive_initializer(block, base_local, next_var, offset))
                                return false;
                            offset += var_type->size_in_bytes();
                            break;
                        }
                        case composite_types_t::union_type: {
                            // XXX: intentional no-op
                            break;
                        }
                        case composite_types_t::struct_type: {
                            auto field_list = composite_type->fields().as_list();
                            size_t index = 0;
                            for (auto fld: field_list) {
                                list.emplace(std::begin(list) + index, fld->identifier());
                                index++;
                            }
                            offset = common::align(offset, composite_type->alignment());
                            break;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }
        }

        return true;
    }

    std::string byte_code_emitter::interned_string_data_label(common::id_t id) {
        common::id_t intern_id;
        _session.interned_strings().element_id_to_intern_id(id, intern_id);
        return fmt::format("_intern_str_lit_{}_data", intern_id);
    }

};
