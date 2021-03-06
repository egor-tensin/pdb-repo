// Copyright (c) 2020 Egor Tensin <Egor.Tensin@gmail.com>
// This file is part of the "winapi-debug" project.
// For details, see https://github.com/egor-tensin/winapi-debug.
// Distributed under the MIT License.

#include "fixtures.hpp"

#include <test_lib.hpp>

#include <winapi/debug.hpp>

#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>

#include <string>
#include <system_error>
#include <vector>

BOOST_FIXTURE_TEST_SUITE(call_stack_tests, CurrentProcess)

BOOST_AUTO_TEST_CASE(call_stack) {
    try {
        test_ns::throw_call_stack();
    } catch (const winapi::CallStack& call_stack) {
        // First, check that the call stack has been caught.
        BOOST_TEST(true, "Caught the call stack");

        // Debug output:
        std::vector<std::string> pretty;
        pretty.reserve(call_stack.length);

        BOOST_TEST_MESSAGE("Call stack:");
        for (const auto& addr : call_stack) {
            pretty.emplace_back(call_stack.pretty_print_address(dbghelp, addr));
            BOOST_TEST_MESSAGE('\t' << winapi::address::format(addr) << ' ' << pretty.back());
        }

        // Second, resolve the symbols:
        std::vector<boost::optional<winapi::SymbolInfo>> symbols;
        symbols.reserve(call_stack.length);

        BOOST_TEST_MESSAGE("Resolved symbols:");
        for (const auto& addr : call_stack) {
            try {
                auto symbol = dbghelp.resolve_symbol(addr);
                BOOST_TEST_MESSAGE('\t' << winapi::address::format(symbol.get_offline_address())
                                        << ' ' << symbol.get_name());
                symbols.emplace_back(std::move(symbol));
            } catch (const std::system_error& e) {
                symbols.emplace_back(boost::none);
                BOOST_TEST_MESSAGE('\t' << winapi::address::format(addr)
                                        << " Couldn't resolve symbol: " << e.what());
            }
        }

        {
            // Third, check that the expected function addresses are in the call stack.
            const auto expected = expected_function_addresses();

            const auto check = [&](winapi::Address addr) {
                for (const auto& symbol : symbols)
                    if (symbol && symbol->get_offline_address() == addr)
                        return true;
                return false;
            };
            for (const auto& addr : expected) {
                BOOST_TEST(check(addr),
                           "Function frame captured: " << winapi::address::format(addr));
            }
        }

        {
            // Fourth, check that the expected function names are in the call stack.
            const auto expected = expected_functions_full();

            const auto check = [&](const std::string& name) {
                for (const auto& actual : pretty)
                    if (actual.find(name) != std::string::npos)
                        return true;
                return false;
            };
            for (const auto& name : expected) {
                BOOST_TEST(check(name), "Function frame captured: " << name);
            }
        }
        return;
    }
    // We must catch the call stack.
    BOOST_TEST(false, "Didn't catch the call stack");
}

BOOST_AUTO_TEST_SUITE_END()
