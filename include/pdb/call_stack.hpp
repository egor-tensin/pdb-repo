// Copyright (c) 2020 Egor Tensin <Egor.Tensin@gmail.com>
// This file is part of the "PDB repository" project.
// For details, see https://github.com/egor-tensin/pdb-repo.
// Distributed under the MIT License.

#pragma once

#include "address.hpp"
#include "dbghelp.hpp"

#include <SafeInt.hpp>

#include <windows.h>

#include <array>
#include <cstddef>
#include <string>

namespace pdb {

class CallStack {
public:
    static constexpr std::size_t frames_to_skip = 0;
    static constexpr std::size_t frames_to_capture = 62;

    // Imposed by CaptureStackBackTrace:
    static constexpr std::size_t max_length = 62;

    static_assert(frames_to_skip + frames_to_capture <= max_length,
                  "Call stack length is too large");

    static CallStack capture();

    const std::array<Address, max_length> frames;
    const std::size_t length;

private:
    CallStack() = default;
};

namespace call_stack {

std::string pretty_print_address(const DbgHelp& dbghelp, Address addr);

} // namespace call_stack
} // namespace pdb