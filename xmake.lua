add_rules("mode.debug", "mode.release")

add_repositories("local-repo dependencies")

-- add_requires("conan::angelscript/2.37.0", {alias = "angelscript"})
-- add_requires("conan::jsoncpp/1.9.5", {alias = "jsoncpp"})
-- add_requires("conan::socketw/3.11.0@anotherfoxguy/stable", {alias = "socketw"})
-- add_requires("conan::libcurl/8.10.1", {alias = "libcurl"})
add_requires("angelscript 2.37.*", "jsoncpp", "libcurl", "socketw", "openssl")

target("as_addons")
    set_kind("static")
    add_files("source/angelscript_add_on/**/*.cpp")
    add_packages("angelscript")

target("rorserver")
    set_kind("binary")
    add_deps("as_addons")
    add_defines("WIN32_LEAN_AND_MEAN", "NOMINMAX")
    add_defines("WITH_ANGELSCRIPT", "WITH_CURL")
    add_includedirs("source/angelscript_add_on/")
    add_includedirs("source/common", "source/protocol")
    add_files("source/server/*.cpp", "source/server/*.c")
    add_packages("angelscript", "jsoncpp", "socketw", "libcurl", "openssl")