//
// Copyright 2021 gRPC authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include <gmock/gmock.h>
#include <grpc/grpc.h>
#include <grpc/grpc_security.h>
#include <grpcpp/security/server_credentials.h>
#include <gtest/gtest.h>

#include "test/core/util/test_config.h"
#include "test/cpp/util/tls_test_utils.h"

namespace {

using ::grpc::experimental::CertificateVerifier;
using ::grpc::experimental::ExternalCertificateVerifier;
using ::grpc::experimental::HostNameCertificateVerifier;
using ::grpc::experimental::TlsCustomVerificationCheckRequest;

}  // namespace

namespace grpc {
namespace testing {
namespace {

TEST(TlsCertificateVerifierTest, HostNameCertificateVerifierSucceeds) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.bar.com";
  auto verifier = std::make_shared<HostNameCertificateVerifier>();
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_TRUE(sync_status.ok());
}

TEST(TlsCertificateVerifierTest, HostNameCertificateVerifierFails) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.baz.com";
  auto verifier = std::make_shared<HostNameCertificateVerifier>();
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_FALSE(sync_status.ok());
}

TEST(TlsCertificateVerifierTest,
     HostNameCertificateVerifierSucceedsMultipleFields) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.baz.com";
  char* dns_names[] = {const_cast<char*>("*.bar.com")};
  request.peer_info.san_names.dns_names = dns_names;
  request.peer_info.san_names.dns_names_size = 1;
  auto verifier = std::make_shared<HostNameCertificateVerifier>();
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_TRUE(sync_status.ok());
}

TEST(TlsCertificateVerifierTest,
     HostNameCertificateVerifierFailsMultipleFields) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.baz.com";
  char* dns_names[] = {const_cast<char*>("*.")};
  request.peer_info.san_names.dns_names = dns_names;
  request.peer_info.san_names.dns_names_size = 1;
  auto verifier = std::make_shared<HostNameCertificateVerifier>();
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_FALSE(sync_status.ok());
}

TEST(TlsCertificateVerifierTest, SyncCertificateVerifierSucceeds) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.bar.com";
  auto verifier =
      ExternalCertificateVerifier::Create<SyncCertificateVerifier>(true);
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_TRUE(sync_status.ok());
}

TEST(TlsCertificateVerifierTest,
     SyncCertificateVerifierFailsOnHostNameVerifier) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.baz.com";
  auto verifier =
      ExternalCertificateVerifier::Create<SyncCertificateVerifier>(true);
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_FALSE(sync_status.ok());
  // I realized that the status for sync verifier is also truncated(please see
  // my below comments for async verifier). There must be something I don't know
  // happening under the hood... EXPECT_EQ(sync_status.error_message(),
  // "UNAUTHENTICATED: Hostname Verification Check failed.");
}

TEST(TlsCertificateVerifierTest,
     SyncCertificateVerifierFailsOnAdditionalCheck) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.bar.com";
  auto verifier =
      ExternalCertificateVerifier::Create<SyncCertificateVerifier>(false);
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_FALSE(sync_status.ok());
  // EXPECT_EQ(sync_status.error_message(),
  // "UNAUTHENTICATED: SyncCertificateVerifier is marked unsuccessful");
}

TEST(TlsCertificateVerifierTest,
     SyncCertificateVerifierFailsOnHostnameAndAdditionalCheck) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.baz.com";
  auto verifier =
      ExternalCertificateVerifier::Create<SyncCertificateVerifier>(false);
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> empty_callback;
  grpc::Status sync_status;
  verifier->Verify(&cpp_request, empty_callback, &sync_status);
  EXPECT_FALSE(sync_status.ok());
  // EXPECT_EQ(sync_status.error_message(),
  // "UNAUTHENTICATED: SyncCertificateVerifier is marked unsuccessful:
  // UNAUTHENTICATED: Hostname Verification Check failed.");
}

TEST(TlsCertificateVerifierTest, AsyncCertificateVerifierSucceeds) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.bar.com";
  auto verifier =
      ExternalCertificateVerifier::Create<AsyncCertificateVerifier>(true);
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> callback = [](grpc::Status async_status) {
    EXPECT_TRUE(async_status.ok());
  };
  grpc::Status sync_status;
  EXPECT_FALSE(verifier->Verify(&cpp_request, callback, &sync_status));
}

TEST(TlsCertificateVerifierTest,
     AsyncCertificateVerifierFailsOnHostnameVerifier) {
  grpc_tls_custom_verification_check_request request;
  memset(&request, 0, sizeof(request));
  request.target_name = "foo.bar.com";
  request.peer_info.common_name = "foo.baz.com";
  auto verifier =
      ExternalCertificateVerifier::Create<AsyncCertificateVerifier>(true);
  TlsCustomVerificationCheckRequest cpp_request(&request);
  std::function<void(grpc::Status)> callback = [](grpc::Status async_status) {
    gpr_log(GPR_ERROR, "%s", async_status.error_message().c_str());
    EXPECT_FALSE(async_status.ok());
    // Question: this is really weird here.
    // The printed message here becomes "UNAUTHENTICATED: "(the message part is
    // truncated). But when I print out the status immediately before we invoke
    // this callback, the full message is printed. It's unlikely that I am
    // logging to the wrong place, because if I changed the logged status to an
    // OK status, the value here will be "". I've read somewhere that gRPC users
    // couldn't know the specific status error though. Could it be due to that?
    // But how come the message is there before invoking the callback, but after
    // entering the callback it is gone(I tried std::move the status but nothing
    // changed). EXPECT_EQ(async_status.error_message(), "UNAUTHENTICATED:
    // Hostname Verification Check failed.");
  };
  grpc::Status sync_status;
  EXPECT_FALSE(verifier->Verify(&cpp_request, callback, &sync_status));
}

}  // namespace
}  // namespace testing
}  // namespace grpc

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  grpc::testing::TestEnvironment env(argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
