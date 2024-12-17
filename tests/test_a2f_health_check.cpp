// SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "grpc++/grpc++.h"

#include "ace_grpc_cpp/health_mock.grpc.pb.h"

#include "aceclient/animation.h"
#include "aceclient/parameters.h"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::DoAll;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::WithArg;

using grpc::health::v1::MockHealthStub;
using grpc::health::v1::HealthCheckResponse;
using mace::A2FControllerClient;

const std::string API_KEY = "myKey";
const std::string FUNCTION_ID = "myFunctionId";


// ==============================
// TestA2FControllerHealthChecks
// ==============================

TEST(TestA2FControllerHealthCheck, HealthCheckSucceeded) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  HealthCheckResponse fakeResponse;
  fakeResponse.set_status(HealthCheckResponse::SERVING);
  EXPECT_CALL(*stub, Check(_, _, _)).Times(1).WillOnce(
    testing::DoAll(testing::SetArgPointee<2>(testing::ByRef(fakeResponse)), testing::Return(grpc::Status::OK)));

  // Act
  auto result = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result.status, AceClientStatus::OK);
}

TEST(TestA2FControllerHealthCheck, HealthCheckUnauthenticated) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  EXPECT_CALL(*stub, Check(_, _, _)).Times(2)
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "failed to open stateful work request: rpc error: code = Unauthenticated desc = invalid response from UAM")))
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "no authorization was passed")));

  // Act
  auto result1 = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);
  auto result2 = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result1.status, AceClientStatus::ERROR_UNAUTHENTICATED);
  EXPECT_EQ(result2.status, AceClientStatus::ERROR_UNAUTHENTICATED);
}

TEST(TestA2FControllerHealthCheck, HealthCheckSSLError) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  EXPECT_CALL(*stub, Check(_, _, _)).Times(1)
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "failed to connect to all addresses; last error: UNKNOWN: ipv4:xxx.xxx.xxx.xxx:port: Ssl handshake failed: SSL_ERROR_SSL: error:FFFFFFFF:SSL routines::wrong version number")));

  // Act
  auto result = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result.status, AceClientStatus::ERROR_SSL_HANDSHAKE);
}

TEST(TestA2FControllerHealthCheck, HealthCheckDeadlineExceeded) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  EXPECT_CALL(*stub, Check(_, _, _)).Times(2)
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "Deadline Exceeded")))
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "Connection refused")));

  // Act
  auto result1 = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);
  auto result2 = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result1.status, AceClientStatus::ERROR_CONNECTION);
  EXPECT_EQ(result2.status, AceClientStatus::ERROR_CONNECTION);
}

TEST(TestA2FControllerHealthCheck, HealthCheckOutOfCredits) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  EXPECT_CALL(*stub, Check(_, _, _)).Times(1)
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "failed to open stateful work request: rpc error: code = Unknown desc = Account 'xxx': Cloud credits expired - Please contact NVIDIA representatives")));

  // Act
  auto result = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result.status, AceClientStatus::ERROR_CREDITS_EXPIRED);
}

TEST(TestA2FControllerHealthCheck, HealthCheckDNSResolutionFailed) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  EXPECT_CALL(*stub, Check(_, _, _)).Times(1)
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "DNS resolution failed for domain.not.exist.com:443: C-ares status is not ARES_SUCCESS qtype=AAAA name=domain.not.exist.com is_balancer=0: Domain name not found")));

  // Act
  auto result = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result.status, AceClientStatus::ERROR_DNS_RESOLUTION);
}

TEST(TestA2FControllerHealthCheck, HealthCheckUnknownError) {
  std::shared_ptr<MockHealthStub> stub = std::make_shared<MockHealthStub>();
  EXPECT_CALL(*stub, Check(_, _, _)).Times(1)
    .WillOnce(testing::Return(grpc::Status(grpc::StatusCode::INTERNAL,
      "For some unknown reason")));

  // Act
  auto result = mace::A2FHealthCheck(stub, API_KEY, FUNCTION_ID);

  // Assert
  EXPECT_EQ(result.status, AceClientStatus::ERROR_UNKNOWN);
}
