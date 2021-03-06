# gRPC Bazel BUILD file.
#
# Copyright 2019 The gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

load("@com_github_grpc_grpc//bazel:python_rules.bzl", "py_proto_library", "py_grpc_library")

package(default_testonly = 1)

proto_library(
    name = "helloworld_proto",
    srcs = ["helloworld.proto"],
    deps = [
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
)

py_proto_library(
    name = "helloworld_py_pb2",
    srcs = [":helloworld_proto"],
)

py_grpc_library(
    name = "helloworld_py_pb2_grpc",
    srcs = [":helloworld_proto"],
    deps = [":helloworld_py_pb2"],
)

py_proto_library(
    name = "duration_py_pb2",
    srcs = ["@com_google_protobuf//:duration_proto"],
)

py_proto_library(
    name = "timestamp_py_pb2",
    srcs = ["@com_google_protobuf//:timestamp_proto"],
)

py_test(
    name = "import_test",
    main = "helloworld.py",
    srcs = ["helloworld.py"],
    deps = [
        ":helloworld_py_pb2",
        ":helloworld_py_pb2_grpc",
        ":duration_py_pb2",
        ":timestamp_py_pb2",
    ],
    python_version = "PY3",
)
