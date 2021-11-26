load("@rules_cc//cc:defs.bzl", "cc_library")
load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

cc_library(
    name = "ladder",
    srcs = glob(["ladder/src/*.cpp", "ladder/src/codec/*.cpp"]),
    hdrs = glob(["ladder/include/*.h", "ladder/include/codec/*.h"]),
    includes = ["ladder/include", "ladder/include/codec"],
    deps = ["@com_google_protobuf//:protobuf",
            "@zlib//:zlib"
    ],
    linkopts = ["-lpthread -lcrypto -lssl"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "ladder_client",
    srcs = glob(["ladder_client/src/*.cpp"]),
    hdrs = glob(["ladder_client/include/*.h"]),
    includes = ["ladder_client/include"],
    deps = [":ladder"],
    linkopts = ["-lpthread"],
    visibility = ["//visibility:public"],
)


# tests

cc_proto_library(
    name = "tests_cc_proto",
    deps = [":tests_proto"],
)

proto_library(
    name = "tests_proto",
    srcs = ["tests/proto/tests.proto"],
)

cc_binary(
    name = "ladder_unit_tests",
    srcs = glob(["tests/unittests/*.cpp"]),
    includes = ["ladder/include",
                "ladder/include/codec",
                "tests"
    ],
    deps = [":ladder",
            ":tests_cc_proto",
            "@com_google_googletest//:gtest",
            "@com_google_googletest//:gtest_main",
    ],
)

TESTS_SUB = ["logger", "server", "proto_server", "event_poller", "file_server", "ssl_server"]

[cc_binary(
     name = "ladder_tests_{}".format(sub),
     srcs = glob(["tests/{}/*.cpp".format(sub)]),
     includes = ["ladder/include", "ladder/include/codec", "tests"],
     linkopts = ["-lpthread"],
     deps = [":ladder", ":tests_cc_proto"],
 ) for sub in TESTS_SUB]

TEST_SUB_CLIENT = ["timer", "client", "mass_clients", "tcp_client", "event_loop_thread", "ssl_client"]

[cc_binary(
     name = "ladder_tests_client_{}".format(sub),
     srcs = glob(["tests_client/{}/*.cpp".format(sub)]),
     includes = ["ladder/include", "ladder/include/codec", "tests"],
     linkopts = ["-lpthread"],
     deps = [":ladder_client", ":tests_cc_proto"],
 ) for sub in TEST_SUB_CLIENT]

cc_binary(
    name = "ladder_test_http_server",
    srcs = glob(["examples/http/*.cpp", "examples/http/*.h"]),
    includes = ["ladder/include", "examples/http"],
    deps = [":ladder"],
)
