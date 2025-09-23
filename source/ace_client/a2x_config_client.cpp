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
#include "a2x_config_client.h"

#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include "ace_grpc_cpp/nvidia_ace.services.a2x_export_config.v1.pb.h"

#include "grpc_client.h"
#include "logger.h"

using nvidia_ace::services::a2x_export_config::v1::A2XConfig;
using nvidia_ace::services::a2x_export_config::v1::ConfigsTypeRequest;

typedef std::unique_ptr<grpc::ClientReaderInterface<A2XConfig>> ConfigStream;


namespace mace {

AceClientResult GetA2XConfig(
    ConfigType const config_type,
    std::vector<ConfigContent> *out_configs,
    std::string address,
    std::string api_key,
    std::string function_id
) {
    AceClientResult result;

    std::shared_ptr<grpc::Channel> channel = CreateChannel(address);
    A2XConfigClient client(channel, api_key, function_id);

    result = client.GetConfigs(config_type, out_configs);

    return result;
}

AceClientResult A2XConfigClient::GetConfigs(
    ConfigType const config_type, std::vector<ConfigContent> *out_configs)
{
    grpc::ClientContext context;
    ExtendDeadline(context);
    AddAuthentication(context, m_api_key, m_function_id);

    ConfigsTypeRequest request;
    request.set_config_type(GetRequestConfigType(config_type));

    ConfigStream stream = m_stub->GetConfigs(&context, request);
    ExtendDeadline(context);  // extend timeout during read

    A2XConfig response;
    while (stream->Read(&response)) {
        out_configs->push_back(
            ConfigContent{config_type, response.name(), response.content()});
        ExtendDeadline(context);  // extend timeout during read
    }

    // Finish stream
    grpc::Status status = stream->Finish();
    if (!status.ok()) {
        LOG_DEBUG("A2XExportConfigClient: RPC failed: " << status.error_message());
        return AceClientResult{AceClientStatus::ERROR_CONNECTION, status.error_message()};
    }

    LOG_DEBUG("A2XExportConfigClient: End Receiving all Configs.");

    return AceClientResult{AceClientStatus::OK};
}

A2XConfigClient::A2XConfigClient(
    std::shared_ptr<grpc::Channel> channel, std::string api_key, std::string function_id
) : m_stub(A2XExportConfigService::NewStub(channel).release())
{
    m_api_key = api_key;
    m_function_id = function_id;
}

A2XConfigClient::A2XConfigClient(
    std::shared_ptr<A2XExportConfigService::StubInterface> stub,
    std::string api_key,
    std::string function_id
) : m_stub(stub)
{
    m_api_key = api_key;
    m_function_id = function_id;
}

std::string GetConfigTypeName(ConfigType const config_type) {
    switch (config_type) {
        case ConfigType::YAML: return "YAML";
        case ConfigType::JSON: return "JSON";
        default: return "Unknown";
    }
}

ConfigsTypeRequest::ConfigType GetRequestConfigType(ConfigType const config_type) {
    switch (config_type) {
        case ConfigType::YAML: return ConfigsTypeRequest::YAML;
        case ConfigType::JSON: return ConfigsTypeRequest::JSON;
        default: throw std::invalid_argument("Invalid config type");
    }
}

} // namespace mace
