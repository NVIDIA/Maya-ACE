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
#pragma once
#include <string>

#include <grpcpp/grpcpp.h>

#include "status.h"


namespace mace {

const int TIMEOUT_SEC = 3;

// Delay the deadline in the gRPC context.
void ExtendDeadline(grpc::ClientContext &context);

std::string const SanitizeAddress(std::string const address);

bool IsHttps(std::string const address);

std::shared_ptr<grpc::Channel> CreateChannel(std::string const address);
void AddAuthentication(
    grpc::ClientContext &context, std::string const api_key, std::string const function_id);


class NVCFGrpcClient {

public:
    NVCFGrpcClient() {};
    NVCFGrpcClient(std::string address, std::string api_key="", std::string function_id="");
    ~NVCFGrpcClient() {};

    void SetAddress(std::string const &new_address);
    std::string const GetAddress();
    void SetAPIKey(std::string const &new_api_key);
    std::string const GetAPIKey();
    void SetFunctionId(std::string const &new_function_id);
    std::string const GetFunctionId();

protected:
    std::string m_address = "https://grpc.nvcf.nvidia.com:443";
    // a secret key from  https://build.nvidia.com/nvidia/audio2face-3d/api
    std::string m_api_key = "";
    // obtained from https://build.nvidia.com/nvidia/audio2face-3d/api
    std::string m_function_id = "462f7853-60e8-474a-9728-7b598e58472c";
};

}
