config_setting(
    name = "opt",
    values = {"compilation_mode": "opt"}
)

cc_library(
    name = "honggfuzz",
    srcs = glob([
            "libhfuzz/*.c",
            "libhfcommon/*.c",
            ],
            exclude = ["libhfuzz/linux.c"],
        ) + select({
            "@bazel_tools//src/conditions:darwin_x86_64": [],
            "@bazel_tools//src/conditions:darwin": [],
             "//conditions:default": ["libhfuzz/linux.c"],
        }),
    hdrs = glob([
        "libhfuzz/*.h",
        "libhfcommon/*.h",
        "honggfuzz.h",
    ]),
    copts = [
        "-std=c11",
    ],
    defines = select({
        "@bazel_tools//src/conditions:darwin_x86_64": ["_HF_ARCH_DARWIN"],
        "@bazel_tools//src/conditions:darwin": ["_HF_ARCH_DARWIN"],
        "//conditions:default": ["_HF_ARCH_LINUX"],
    }) + select({
        ":opt": [],
         "//conditions:default": ["DEBUG=DEBUG"],
    }) + [
        "_GNU_SOURCE",
    ],
    includes = ["."],
    visibility = ["//visibility:public"],
    linkstatic = 1
)
