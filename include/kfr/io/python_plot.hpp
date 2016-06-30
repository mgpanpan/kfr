/**
 * Copyright (C) 2016 D Levin (http://www.kfrlib.com)
 * This file is part of KFR
 *
 * KFR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * KFR is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with KFR.
 *
 * If GPL is not suitable for your project, you must purchase a commercial license to use KFR.
 * Buying a commercial license is mandatory as soon as you develop commercial activities without
 * disclosing the source code of your own applications.
 * See http://www.kfrlib.com for details.
 */
#pragma once
#include "../base/vec.hpp"
#include "../cometa/string.hpp"
#include <cstdlib>

#ifdef KFR_OS_WIN
#include <direct.h>
#define cross_getcwd _getcwd
#else
#include <unistd.h>
#define cross_getcwd getcwd
#endif

namespace kfr
{
namespace internal
{

void python(const std::string& name, const std::string& code)
{
    std::string filename;
    {
        char curdir[1024];
        cross_getcwd(curdir, arraysize(curdir));
        filename = curdir;
    }
#ifdef KFR_OS_WIN
    const char* slash = "\\";
#else
    const char* slash = "/";
#endif
    filename = filename + slash + name + ".py";

    FILE* f = fopen(filename.c_str(), "w");
    fwrite(code.c_str(), 1, code.size(), f);
    fclose(f);
    std::system(("python \"" + filename + "\"").c_str());
}
}

static std::string concat_args() { return {}; }

template <typename... Ts>
static std::string concat_args(const std::string& left, const Ts&... rest)
{
    const std::string right = concat_args(rest...);
    return left.empty() ? right : right.empty() ? left : left + ", " + right;
}

static void plot_show(const std::string& name, const std::string& wavfile, const std::string& options = "")
{
    print(name, "...");
    std::string ss;
    ss += "#!/usr/bin/env python\n"
          "import dspplot\n\n"
          "dspplot.plot(" +
          concat_args("r'" + wavfile + "'", options) + ")\n";

    internal::python(name, ss);
    print("done\n");
}

static void plot_show(const std::string& name, const char* x, const std::string& options = "")
{
    plot_show(name, std::string(x), options);
}

template <typename T>
void plot_show(const std::string& name, T&& x, const std::string& options = "")
{
    print(name, "...");
    auto array = make_array_ref(std::forward<T>(x));
    std::string ss;
    ss += "#!/usr/bin/env python\n"
          "import dspplot\n\n"
          "data = [\n";
    for (size_t i = 0; i < array.size(); i++)
        ss += as_string(fmt<'g', 20, 17>(array[i]), ",\n");
    ss += "]\n";

    ss += "dspplot.plot(" + concat_args("data", options) + ")\n";

    internal::python(name, ss);
    print("done\n");
}

template <typename T>
void plot_save(const std::string& name, T&& x, const std::string& options = "")
{
    plot_show(name, std::forward<T>(x), concat_args(options, "file='../svg/" + name + ".svg'"));
}

template <typename T1, typename T2>
void perfplot_show(const std::string& name, T1&& data, T2&& labels, const std::string& options = "")
{
    print(name, "...");
    auto array        = make_array_ref(std::forward<T1>(data));
    auto labels_array = make_array_ref(std::forward<T2>(labels));
    std::string ss;
    ss += "#!/usr/bin/env python\n";
    ss += "import dspplot\n\n";
    ss += "data = [\n";
    for (size_t i = 0; i < array.size(); i++)
    {
        auto subarray = make_array_ref(array[i]);
        ss += "[\n";
        for (size_t i = 0; i < subarray.size(); i++)
            ss += as_string("    ", fmt<'g', 20, 17>(subarray[i]), ",\n");
        ss += "],";
    }
    ss += "]\n";

    ss += "labels = [\n";
    for (size_t i = 0; i < labels_array.size(); i++)
    {
        const std::string label = labels_array[i];
        ss += "    '" + label + "',";
    }
    ss += "]\n";

    ss += "dspplot.perfplot(" + concat_args("data, labels", options) + ")\n";

    internal::python(name, ss);
    print("done\n");
}

template <typename T1, typename T2>
void perfplot_save(const std::string& name, T1&& data, T2&& labels, const std::string& options = "")
{
    perfplot_show(name, std::forward<T1>(data), std::forward<T2>(labels),
                  concat_args(options, "file='../perf/" + name + ".svg'"));
}
}