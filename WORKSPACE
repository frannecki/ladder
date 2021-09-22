workspace(name="ladder")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

load("//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()

http_archive(
  name = "com_google_protobuf",
  strip_prefix = "protobuf-3.18.0",
  sha256 = "7308590dbb95e77066b99c5674eed855c8257e70658d2af586f4a81ff0eea2b1",
  type = "tar.gz",
  url = "https://github.com/protocolbuffers/protobuf/releases/download/v3.18.0/protobuf-cpp-3.18.0.tar.gz",
)

http_archive(
  name = "zlib",
  strip_prefix = "zlib-1.2.11",
  sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
  type = "tar.gz",
  url = "https://zlib.net/zlib-1.2.11.tar.gz",
  build_file = "@//:zlib.BUILD",
)

http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    urls = [
        "https://mirror.bazel.build/github.com/google/googletest/archive/release-1.10.0.tar.gz",
        "https://github.com/google/googletest/archive/release-1.10.0.tar.gz",
    ],
)

