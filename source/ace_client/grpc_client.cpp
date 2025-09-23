// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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
#include "grpc_client.h"
#include "utility.h"

#include <chrono>
#include <memory>

#include <grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/security/credentials.h>

namespace mace {

NVCFGrpcClient::NVCFGrpcClient(std::string address, std::string api_key, std::string function_id)
    : m_address(address), m_api_key(api_key), m_function_id(function_id) {}

void NVCFGrpcClient::SetAddress(std::string const &new_address) {m_address = new_address;}

std::string const NVCFGrpcClient::GetAddress() {return m_address;}

void NVCFGrpcClient::SetAPIKey(std::string const &new_api_key) {m_api_key = new_api_key;}

std::string const NVCFGrpcClient::GetAPIKey() {return m_api_key;}

void NVCFGrpcClient::SetFunctionId(std::string const &new_function_id) {m_function_id = new_function_id;}

std::string const NVCFGrpcClient::GetFunctionId() {return m_function_id;}

void AddAuthentication(
    grpc::ClientContext &context, std::string const api_key, std::string const function_id) {
    /* Do authentication tasks with nvcf grpc api.
    */
    if (!api_key.empty()) {
      context.AddMetadata("authorization", "Bearer " + api_key);
    }
    if (!function_id.empty()) {
      context.AddMetadata("function-id", function_id);
    }
}

std::shared_ptr<grpc::Channel> CreateChannel(std::string const address) {
    // establish connection to a2f controller using grpc client
    std::shared_ptr<grpc::Channel> channel;
    std::string clean_address = SanitizeAddress(address);

    if (IsHttps(address)) {
        grpc::SslCredentialsOptions ssl_opts;
        auto creds = grpc::SslCredentials(ssl_opts);
        channel = grpc::CreateChannel(clean_address, creds);
    }
    else {
        channel = grpc::CreateChannel(clean_address, grpc::InsecureChannelCredentials());
    }
    return channel;
}

void ExtendDeadline(grpc::ClientContext &context) {
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(mace::TIMEOUT_SEC));
}

std::string const SanitizeAddress(std::string const address) {
    // remove the protocol prefix
    std::string clean_address = address;
    if (clean_address.length() >= 7 && clean_address.substr(0, 7) == "http://") {
        clean_address.erase(0, 7);
    } else if (clean_address.length() >= 8 && clean_address.substr(0, 8) == "https://") {
        clean_address.erase(0, 8);
    }
    return SanitizeString(clean_address);
}

bool IsHttps(std::string const address) {
    // return true if address starts with https://
    return (address.length() >= 8 && address.substr(0, 8) == "https://");
}

}
