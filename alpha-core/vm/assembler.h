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

#include "instruction_emitter.h"

namespace basecode::vm {

    enum class segment_type_t {
        code,
        data,
        stack,
        constant,
    };

    static inline std::unordered_map<segment_type_t, std::string> s_segment_type_names = {
        {segment_type_t::code,     "code"},
        {segment_type_t::data,     "data"},
        {segment_type_t::stack,    "stack"},
        {segment_type_t::constant, "constant"}
    };

    static inline std::string segment_type_name(segment_type_t type) {
        auto it = s_segment_type_names.find(type);
        if (it == s_segment_type_names.end())
            return "unknown";
        return it->second;
    }

    enum class symbol_type_t {
        u8,
        u16,
        u32,
        u64,
        f32,
        f64,
        bytes
    };

    static inline size_t size_of_symbol_type(symbol_type_t type) {
        switch (type) {
            case symbol_type_t::u8:    return 1;
            case symbol_type_t::u16:   return 2;
            case symbol_type_t::u32:   return 4;
            case symbol_type_t::u64:   return 8;
            case symbol_type_t::f32:   return 4;
            case symbol_type_t::f64:   return 8;
            default:
                return 8;
        }
    }

    struct symbol_t {
        symbol_t(
            const std::string& name,
            symbol_type_t type,
            uint64_t address,
            size_t size = 0);

        size_t size;
        uint64_t address;
        std::string name;
        symbol_type_t type;

        union {
            double float_value;
            uint64_t int_value;
            void* byte_array_value;
        } value;
    };

    struct segment_t {
        segment_t(
            const std::string& name,
            segment_type_t type,
            uint64_t address);

        symbol_t* symbol(
            const std::string& name,
            symbol_type_t type,
            size_t size = 0);

        size_t size() const;

        symbol_t* symbol(const std::string& name);

        uint64_t address = 0;
        uint64_t offset = 0;
        std::string name;
        segment_type_t type;

    private:
        std::unordered_map<std::string, symbol_t> _symbols {};
    };

    class assembler {
    public:
        explicit assembler(vm::terp* terp);

        void segment(
            const std::string& name,
            segment_type_t type,
            uint64_t address);

        bool assemble(
            common::result& r,
            std::istream& source);

        void define_data(float value);

        void define_data(double value);

        instruction_emitter& emitter();

        void define_data(uint8_t value);

        void define_data(uint16_t value);

        void define_data(uint32_t value);

        void define_data(uint64_t value);

        uint64_t location_counter() const;

        void location_counter(uint64_t value);

        segment_t* segment(const std::string& name);

        void define_string(const std::string& value);

    private:
        vm::terp* _terp = nullptr;
        instruction_emitter _emitter;
        uint64_t _location_counter = 0;
        std::unordered_map<std::string, segment_t> _segments {};
    };

};
