// Copyright (c) 2017 Egor Tensin <Egor.Tensin@gmail.com>
// This file is part of the "PDB repository" project.
// For details, see https://github.com/egor-tensin/pdb-repo.
// Distributed under the MIT License.

#include <pdb/all.hpp>

#include <boost/nowide/convert.hpp>

#include <windows.h>

#include <sstream>
#include <string>
#include <system_error>

namespace pdb {
namespace error {
namespace {

std::wstring trim_trailing_newline(const std::wstring& s) {
    const auto last_pos = s.find_last_not_of(L"\r\n");
    if (std::wstring::npos == last_pos)
        return {};
    return s.substr(0, last_pos + 1);
}

std::string build_what(DWORD code, const char* function) {
    std::ostringstream what;
    what << "Function " << function << " failed with error code " << code;
    return what.str();
}

std::string format_message(int code) {
    wchar_t* buf;

    const auto len = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<wchar_t*>(&buf),
        0,
        NULL);

    if (0 == len) {
        LocalFree(buf);
        return "Couldn't format the error message";
    }

    std::wstring msg{buf, len};
    LocalFree(buf);
    return boost::nowide::narrow(trim_trailing_newline(msg));
}

} // namespace

std::string CategoryWindows::message(int code) const {
    return format_message(code);
}

std::system_error windows(DWORD code, const char* function) {
    static_assert(sizeof(DWORD) == sizeof(int), "Aren't DWORDs the same size as ints?");
    return std::system_error{
        static_cast<int>(code), category_windows(), build_what(code, function)};
}

} // namespace error
} // namespace pdb
