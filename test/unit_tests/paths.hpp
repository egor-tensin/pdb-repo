// Copyright (c) 2020 Egor Tensin <Egor.Tensin@gmail.com>
// This file is part of the "winapi-debug" project.
// For details, see https://github.com/egor-tensin/winapi-debug.
// Distributed under the MIT License.

#pragma once

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

using path = boost::filesystem::path;

class Paths {
public:
    typedef boost::filesystem::path path;

    static Paths& get() {
        static Paths instance;
        return instance;
    }

    Paths() : exe_path{get_executable_path()}, exe_dir{exe_path.parent_path()} {}

    const path exe_path;
    const path exe_dir;

private:
    static path get_executable_path() {
        const auto argv0 = boost::unit_test::framework::master_test_suite().argv[0];
        return boost::filesystem::system_complete(argv0);
    }

    Paths(const Paths&) = delete;
    Paths& operator=(const Paths&) = delete;
};
