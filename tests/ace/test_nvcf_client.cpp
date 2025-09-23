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
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <optional>

#include "ace_client/grpc_client.h"
#include "ace_client/logger.h"

#include <gtest/gtest.h>

#pragma warning(disable : 4305)

using mace::NVCFGrpcClient;


class MockNVCFGrpcClient : public NVCFGrpcClient {
    // dummy class to access protected members
    public:
    MockNVCFGrpcClient() : NVCFGrpcClient() {};

    using NVCFGrpcClient::m_address;
    using NVCFGrpcClient::m_api_key;
    using NVCFGrpcClient::m_function_id;

    using NVCFGrpcClient::GetAddress;
    using NVCFGrpcClient::SetAddress;
    using NVCFGrpcClient::GetAPIKey;
    using NVCFGrpcClient::SetAPIKey;
    using NVCFGrpcClient::GetFunctionId;
    using NVCFGrpcClient::SetFunctionId;
};

class TestNVCFGrpcClient : public ::testing::Test {
protected:
    std::unique_ptr<MockNVCFGrpcClient> client;

    void SetUp() override {
        client = std::make_unique<MockNVCFGrpcClient>();
    }
};

TEST_F(TestNVCFGrpcClient, TestGetSet) {
    const std::string test_address = "_address_";
    const std::string test_api_key = "_api_key_";
    const std::string test_function_id = "_function_id_";

    client->SetAddress(test_address);
    client->SetAPIKey(test_api_key);
    client->SetFunctionId(test_function_id);

    // Assert
    EXPECT_EQ(client->m_address, test_address);
    EXPECT_EQ(client->m_api_key, test_api_key);
    EXPECT_EQ(client->m_function_id, test_function_id);
    EXPECT_EQ(client->GetAddress(), test_address);
    EXPECT_EQ(client->GetAPIKey(), test_api_key);
    EXPECT_EQ(client->GetFunctionId(), test_function_id);
}

TEST(TestNvcfClient, TestSanitizeAddress) {
    EXPECT_EQ(mace::SanitizeAddress("http://test_url"), "test_url");
    EXPECT_EQ(mace::SanitizeAddress("https://test_url"), "test_url");
    EXPECT_EQ(mace::SanitizeAddress("http://test_url "), "test_url");
    EXPECT_EQ(mace::SanitizeAddress("http://test_url/"), "test_url");
}

TEST(TestNvcfClient, TestIsHttps) {
    EXPECT_EQ(mace::IsHttps("http://test_url"), false);
    EXPECT_EQ(mace::IsHttps("https://test_url"), true);
}
