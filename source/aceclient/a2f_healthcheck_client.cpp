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
#include "a2f_healthcheck_client.h"

#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "grpc_client.h"
#include "logger.h"

using grpc::health::v1::HealthCheckRequest;
using grpc::health::v1::HealthCheckResponse;

namespace mace {

AceClientResult A2FHealthCheck(
    std::shared_ptr<Health::StubInterface> stub, std::string api_key, std::string function_id)
{
    grpc::ClientContext context;
    context.set_deadline(
        std::chrono::system_clock::now() + std::chrono::seconds(TIMEOUT_SEC));

    if (!api_key.empty()) {
      context.AddMetadata("authorization", "Bearer " + api_key);
    }
    if (!function_id.empty()) {
      context.AddMetadata("function-id", function_id);
    }

    LOG_DEBUG("A2FHealthCheck: Checking...");
    HealthCheckRequest request;
    HealthCheckResponse response;
    grpc::Status status = stub->Check(&context, request, &response);
    if (!status.ok()) {
        std::string errorMessage = status.error_message();
        LOG_ERROR("A2FHealthCheck: Failed. code: " << status.error_code() << ", reason: " << errorMessage);
        // The error code did not accurately reflect the actual cause of the failure; we need to parse the error message to identify the correct issue.
        if (errorMessage.find("Unauthenticated") != std::string::npos ||
            errorMessage.find("no authorization was passed") != std::string::npos) {
            // failed to open stateful work request: rpc error: code = Unauthenticated desc = invalid response from UAM
            // or
            // no authorization was passed in the metadata
            return AceClientResult{AceClientStatus::ERROR_UNAUTHENTICATED, errorMessage};
        } else if (errorMessage.find("SSL_ERROR_SSL") != std::string::npos) {
            // failed to connect to all addresses; last error: UNKNOWN: ipv4:xxx.xxx.xxx.xxx:port: Ssl handshake failed: SSL_ERROR_SSL: error:FFFFFFFF:SSL routines::wrong version number
            return AceClientResult{AceClientStatus::ERROR_SSL_HANDSHAKE, errorMessage};
        } else if (errorMessage.find("Deadline Exceeded") != std::string::npos) {
            // Deadline Exceeded
            // Possible causes:
            // - bad network connection
            // - invalid server port
            // - connecting to a https server using http:// protocol
            return AceClientResult{AceClientStatus::ERROR_CONNECTION, errorMessage};
        } else if (errorMessage.find("Cloud credits expired") != std::string::npos) {
            // failed to open stateful work request: rpc error: code = Unknown desc = Account 'xxx': Cloud credits expired - Please contact NVIDIA representatives
            return AceClientResult{AceClientStatus::ERROR_CREDITS_EXPIRED, errorMessage};
        } else if (errorMessage.find("DNS resolution failed") != std::string::npos) {
            // Failed. code: 14, reason: DNS resolution failed for domain.not.exist.com:443: C-ares status is not ARES_SUCCESS qtype=AAAA name=domain.not.exist.com is_balancer=0: Domain name not found
            return AceClientResult{AceClientStatus::ERROR_DNS_RESOLUTION, errorMessage};
        } else if (errorMessage.find("Connection refused") != std::string::npos) {
            // Failed. code: 14, reason: failed to connect to all addresses; last error: UNAVAILABLE: ipv4:127.0.0.127:4434: Connection refused
            return AceClientResult{AceClientStatus::ERROR_CONNECTION, errorMessage};
        }
        // The following is a list of known error types. However, the current error messages are vague and lack sufficient detail.
        // The server should be updated to provide more informative error messages instead of a generic internal server error.
        // - Invalid function_id:
        //   Failed. code: 13, reason: failed to open stateful work request: rpc error: code = Internal desc = There was a server error trying to handle an exception
        return AceClientResult{AceClientStatus::ERROR_UNKNOWN, errorMessage};
    }
    LOG_DEBUG("A2FHealthCheck: no grpc error.");
    if (response.status() != HealthCheckResponse::SERVING) {
        auto responseStatus = HealthCheckResponse::ServingStatus_descriptor()
            ->FindValueByNumber(response.status())->name();
        std::string errorMsg = "A2FHealthCheck: Expected status to be serving but got " + responseStatus;
        LOG_ERROR(errorMsg);
        return AceClientResult{AceClientStatus::ERROR_UNKNOWN, errorMsg};
    }
    LOG_DEBUG("A2FHealthCheck: Ok!");
    return AceClientResult{AceClientStatus::OK};
}

AceClientResult A2FHealthCheck(
    std::shared_ptr<grpc::Channel> channel, std::string api_key, std::string function_id)
{
    std::shared_ptr<Health::StubInterface> stub = Health::NewStub(channel);
    return A2FHealthCheck(stub, api_key, function_id);
}

} // namespace mace
