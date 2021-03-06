// Copyright (c) 2020 Egor Tensin <Egor.Tensin@gmail.com>
// This file is part of the "winapi-debug" project.
// For details, see https://github.com/egor-tensin/winapi-debug.
// Distributed under the MIT License.

#pragma once

#include "paths.hpp"

#include <test_lib.hpp>

#include <winapi/debug.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

#include <iterator>
#include <string>
#include <unordered_set>
#include <utility>

template <typename T>
using Set = std::unordered_set<T>;

template <typename T>
Set<T> join(Set<T>&& xs, Set<T>&& ys) {
    xs.insert(std::make_move_iterator(ys.begin()), std::make_move_iterator(ys.end()));
    return std::move(xs);
}

class DbgHelp {
public:
    DbgHelp(winapi::DbgHelp&& dbghelp) : dbghelp{std::move(dbghelp)} {}

    ~DbgHelp() { BOOST_TEST_MESSAGE("Cleaning up DbgHelp"); }

    const winapi::DbgHelp dbghelp;

    static const std::string& get_module_name() {
        static const std::string name{"test_lib"};
        return name;
    }

    static const std::string& get_namespace() {
        static const std::string name{"test_ns"};
        return name;
    }

    typedef Set<std::string> SymbolList;
    typedef Set<winapi::Address> AddressList;

    static AddressList expected_function_addresses() {
        return cast({reinterpret_cast<void*>(&test_ns::foo),
                     reinterpret_cast<void*>(&test_ns::bar),
                     reinterpret_cast<void*>(&test_ns::baz)});
    }

    static SymbolList expected_functions() { return make_qualified({"foo", "bar", "baz"}); }

    static SymbolList expected_functions_full() { return add_module(expected_functions()); }

    static SymbolList expected_variables() { return make_qualified({"var"}); }

    static SymbolList expected_symbols() {
        return join(expected_functions(), expected_variables());
    }

protected:
    static winapi::DbgHelp init_dbghelp(bool current_process) {
        BOOST_TEST_MESSAGE("Initializing DbgHelp");
        if (current_process) {
            return winapi::DbgHelp::current_process();
        } else {
            return winapi::DbgHelp::post_mortem();
        }
    }

private:
    static AddressList cast(Set<void*>&& fs) {
        AddressList addresses;
        for (auto&& f : fs) {
            addresses.emplace(reinterpret_cast<winapi::Address>(f));
        }
        return addresses;
    }

    static SymbolList make_qualified(SymbolList&& plain) {
        SymbolList qualified;
        for (auto&& name : plain) {
            qualified.emplace(get_namespace() + "::" + std::move(name));
        }
        return qualified;
    }

    static SymbolList add_module(SymbolList&& plain) {
        SymbolList full;
        for (auto&& name : plain) {
            full.emplace(get_module_name() + "!" + std::move(name));
        }
        return full;
    }

    DbgHelp(const DbgHelp&) = delete;
    DbgHelp& operator=(const DbgHelp&) = delete;
};

class PostMortem : public DbgHelp {
public:
    PostMortem() : DbgHelp{init_dbghelp(false)} { load_module_pdb(); }

private:
    void load_module_pdb() {
        const auto pdb_path = get_module_pdb_path().string();
        BOOST_TEST_MESSAGE("Loading PDB: " << pdb_path);
        dbghelp.load_pdb(pdb_path);
    }

    static boost::filesystem::path get_module_pdb_path() {
        // That's pretty ad-hoc, but seems to work; based on
        // https://www.boost.org/doc/libs/1_70_0/libs/test/doc/html/boost_test/runtime_config/custom_command_line_arguments.html
        BOOST_TEST_REQUIRE(boost::unit_test::framework::master_test_suite().argc == 3);
        BOOST_TEST_REQUIRE(boost::unit_test::framework::master_test_suite().argv[1] ==
                           "--test_lib_pdb");
        return boost::unit_test::framework::master_test_suite().argv[2];
    }
};

class CurrentProcess : public DbgHelp {
public:
    CurrentProcess() : DbgHelp{init_dbghelp(true)} {}
};
