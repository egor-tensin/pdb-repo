// Copyright (c) 2017 Egor Tensin <Egor.Tensin@gmail.com>
// This file is part of the "PDB repository" project.
// For details, see https://github.com/egor-tensin/pdb-repo.
// Distributed under the MIT License.

#include "command_line.hpp"
#include "pdb_descr.hpp"

#include "pdb/all.hpp"

#pragma warning(push, 0)
#include <boost/program_options.hpp>
#pragma warning(pop)

#include <exception>
#include <iostream>
#include <string>
#include <vector>

namespace
{
    class Name2Addr : public SettingsParser
    {
    public:
        explicit Name2Addr(const std::string& argv0)
            : SettingsParser{argv0}
        {
            visible.add_options()
                ("pdb",
                    boost::program_options::value<std::vector<PDB>>(&pdbs)
                        ->value_name("ADDR,PATH"),
                    "load a PDB file");
            hidden.add_options()
                ("name",
                    boost::program_options::value<std::vector<std::string>>(&names)
                        ->value_name("NAME"),
                    "add a name to resolve");
            positional.add("name", -1);
        }

        const char* get_short_description() const override
        {
            return "[-h|--help] [--pdb ADDR,PATH]... [--] [NAME]...";
        }

        std::vector<PDB> pdbs;
        std::vector<std::string> names;
    };

    void dump_error(const std::exception& e)
    {
        std::cerr << "error: " << e.what() << '\n';
    }

    void resolve_symbol(const pdb::Repo& repo, const std::string& name)
    {
        try
        {
            const auto address = repo.resolve_symbol(name).get_online_address();
            std::cout << pdb::format_address(address) << '\n';
        }
        catch (const std::exception& e)
        {
            dump_error(e);
            std::cout << name << '\n';
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Name2Addr settings{argv[0]};

        try
        {
            settings.parse(argc, argv);
        }
        catch (const boost::program_options::error& e)
        {
            settings.usage_error(e);
            return 1;
        }

        if (settings.exit_with_usage)
        {
            settings.usage();
            return 0;
        }

        pdb::Repo repo;

        for (const auto& pdb : settings.pdbs)
            repo.add_pdb(pdb.online_base, pdb.path);

        for (const auto& name : settings.names)
            resolve_symbol(repo, name);
    }
    catch (const std::exception& e)
    {
        dump_error(e);
        return 1;
    }
    return 0;
}
