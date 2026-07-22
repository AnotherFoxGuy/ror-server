package("socketw")
    set_homepage("https://github.com/RigsOfRods/socketw")
    set_description("SocketW is a library which provides cross-platform socket abstraction")
    set_license("GNU Lesser General Public License v2.1")

    add_urls("https://github.com/RigsOfRods/socketw/archive/refs/tags/$(version).tar.gz",
             "https://github.com/RigsOfRods/socketw.git")

    add_versions("3.11.0", "394203af6eabe27dec383fffbcb106bd75e91bf65a07d69e8f2d37d6c4f08371")

    on_install(function(package)
        io.writefile("src/sw_config.h", "#define _HAVE_SSL")

        io.writefile("xmake.lua", ([[
            add_rules("mode.debug", "mode.release")
            add_requires("openssl")
            target("socketw")
                set_version("%s.%s", {soname = true})
                set_languages("c11")
                set_kind("$(kind)")
                add_defines("WIN32_LEAN_AND_MEAN")
                add_files("src/*.cxx")
                add_headerfiles("src/*.h")
                add_packages("openssl")
        ]]):format(package:version():major(), package:version():minor()))
        import("package.tools.xmake").install(package)
    end)