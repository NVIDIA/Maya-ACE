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
#include <memory>

#include <grpcpp/grpcpp.h>

#include "ace_grpc_cpp/nvidia_ace.services.a2x_export_config.v1.grpc.pb.h"

#include "grpc_client.h"
#include "status.h"

using nvidia_ace::services::a2x_export_config::v1::A2XExportConfigService;
using nvidia_ace::services::a2x_export_config::v1::ConfigsTypeRequest;

using mace::NVCFGrpcClient;

class TestConfigClient_TestConstructor_Test;


namespace mace {

enum class ConfigType {
    YAML,
    JSON
};
std::string GetConfigTypeName(ConfigType const config_type);
ConfigsTypeRequest::ConfigType GetRequestConfigType(ConfigType const config_type);

struct ConfigContent {
    ConfigType type;
    std::string name;
    std::string content;
};

AceClientResult GetA2XConfig(
    ConfigType const config_type,
    std::vector<ConfigContent> *out_configs,
    std::string address,
    std::string api_key="",
    std::string function_id=""
);

class A2XConfigClient : public NVCFGrpcClient{
public:
    A2XConfigClient(
        std::shared_ptr<grpc::Channel> channel,
        std::string api_key,
        std::string function_id
    );
    A2XConfigClient(
        std::shared_ptr<A2XExportConfigService::StubInterface> stub,
        std::string api_key,
        std::string function_id
    );

    AceClientResult GetConfigs(
        ConfigType const config_type,
        std::vector<ConfigContent> *out_configs
    );

protected:
    std::shared_ptr<A2XExportConfigService::StubInterface> m_stub;

// allow tests/test_a2f_controller_client.cpp to verify the private member variables
friend class TestConfigClient_TestConstructor_Test;
};

} // namespace mace
