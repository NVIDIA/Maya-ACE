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
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "ace_grpc_cpp/nvidia_ace.services.a2x_export_config.v1.grpc.pb.h"
#include "ace_grpc_cpp/nvidia_ace.services.a2x_export_config.v1_mock.grpc.pb.h"

#include "ace_client/a2x_config_client.h"

using ::testing::_;

using ::grpc::ClientContext;
using ::grpc::Status;

using nvidia_ace::services::a2x_export_config::v1::A2XExportConfigService;
using nvidia_ace::services::a2x_export_config::v1::A2XConfig;
using nvidia_ace::services::a2x_export_config::v1::ConfigsTypeRequest;
using nvidia_ace::services::a2x_export_config::v1::MockA2XExportConfigServiceStub;

using mace::A2XConfigClient;
using mace::ConfigContent;
using mace::ConfigType;

extern std::string TEST_URL; // defined and populated in main.cpp
const std::string API_KEY = "myKey";
const std::string FUNCTION_ID = "myFunctionId";

class MockConfigReader : public grpc::ClientReaderInterface<A2XConfig> {
public:
    MOCK_METHOD(bool, Read, (A2XConfig *msg), (override));
    MOCK_METHOD(grpc::Status, Finish, (), (override));
    MOCK_METHOD(void, WaitForInitialMetadata, (), (override));
    MOCK_METHOD(bool, NextMessageSize, (uint32_t *sz), (override));
};

// ==============================
// TestA2XExportConfigStub
// ==============================
class TestA2XExportConfigStub : public ::testing::Test {
protected:
    std::unique_ptr<MockA2XExportConfigServiceStub> stub;
    ClientContext context;

    void SetUp() override {
        stub = std::make_unique<MockA2XExportConfigServiceStub>();
    }
};

TEST_F(TestA2XExportConfigStub, GetConfigOnce) {
    // Arrange
    // Prepare mock reader
    // It must not be deleted in this function, its ownership is handled by receiving types.
    auto reader = new MockConfigReader(); // Allocate on heap to match the return type

    // Mock the behavior of the reader
    A2XConfig response1;
    response1.set_name("config1");
    response1.set_content("key: value");

    // Mock the behavior of Read method
    EXPECT_CALL(*reader, Read(_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(response1), testing::Return(true)))
        .WillOnce(testing::Return(false)); // No more responses after the first one

    // Mock the Finish method to return an OK status
    EXPECT_CALL(*reader, Finish()).WillOnce(testing::Return(grpc::Status::OK));

    // Mock the gRPC stub behavior
    EXPECT_CALL(*stub, GetConfigsRaw(_, _)).WillOnce(testing::Return(reader));

    // Act & Assert
    ConfigsTypeRequest request;
    request.set_config_type(ConfigsTypeRequest::YAML);

    A2XConfig config;
    auto client_reader = stub->GetConfigs(&context, request);

    ASSERT_TRUE(client_reader->Read(&config));
    EXPECT_EQ(config.name(), "config1");
    EXPECT_EQ(config.content(), "key: value");

    // Ensure that there are no more messages
    EXPECT_FALSE(client_reader->Read(&config));
    EXPECT_EQ(client_reader->Finish().error_code(), grpc::StatusCode::OK);
}

TEST_F(TestA2XExportConfigStub, GetConfigsTwice) {
    // Arrange
    // Prepare mock reader
    // It must not be deleted in this function, its ownership is handled by receiving types.
    auto reader = new MockConfigReader(); // Allocate on heap to match the return type

    // Mock the behavior of the reader
    A2XConfig response1;
    response1.set_name("config1");
    response1.set_content("{\"key1\": \"value1\"}");

    // Mock the behavior of the reader
    A2XConfig response2;
    response2.set_name("config2");
    response2.set_content("{\"key2\": \"value2\"}");

    // Mock the behavior of Read method
    EXPECT_CALL(*reader, Read(_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(response1), testing::Return(true)))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(response2), testing::Return(true)))
        .WillOnce(testing::Return(false)); // No more responses after the first one

    // Mock the Finish method to return an OK status
    EXPECT_CALL(*reader, Finish()).WillOnce(testing::Return(grpc::Status::OK));

    // Mock the gRPC stub behavior
    EXPECT_CALL(*stub, GetConfigsRaw(_, _)).WillOnce(testing::Return(reader));

    // Act & Assert
    ConfigsTypeRequest request;
    request.set_config_type(ConfigsTypeRequest::JSON);

    A2XConfig config;
    auto client_reader = stub->GetConfigs(&context, request);

    ASSERT_TRUE(client_reader->Read(&config));
    EXPECT_EQ(config.name(), "config1");
    EXPECT_EQ(config.content(), "{\"key1\": \"value1\"}");

    ASSERT_TRUE(client_reader->Read(&config));
    EXPECT_EQ(config.name(), "config2");
    EXPECT_EQ(config.content(), "{\"key2\": \"value2\"}");

    // Ensure that there are no more messages
    EXPECT_FALSE(client_reader->Read(&config));
    EXPECT_EQ(client_reader->Finish().error_code(), grpc::StatusCode::OK);
}

// ==============================
// TestConfigClient
// ==============================
class TestConfigClient : public ::testing::Test {
protected:
    std::shared_ptr<MockA2XExportConfigServiceStub> stub;
    std::unique_ptr<A2XConfigClient> client;
    ClientContext context;

    void SetUp() override {
        stub = std::make_shared<MockA2XExportConfigServiceStub>();
        client = std::make_unique<A2XConfigClient>(stub, API_KEY, FUNCTION_ID);
    }
};

TEST_F(TestConfigClient, TestConstructor) {
    const std::string test_api_key = "_api_key_";
    const std::string test_function_id = "_function_id_";

    // Act
    std::unique_ptr<A2XConfigClient> client(new A2XConfigClient(stub, test_api_key, test_function_id));

    // Assert
    EXPECT_EQ(client->m_api_key, test_api_key);
    EXPECT_EQ(client->m_function_id, test_function_id);
}

TEST_F(TestConfigClient, ClientGetConfig) {
    // Arrange
    // Prepare mock reader
    // It must not be deleted in this function, its ownership is handled by receiving types.
    auto reader = new MockConfigReader(); // Allocate on heap to match the return type

    // Mock the behavior of the reader
    A2XConfig response1;
    response1.set_name("config1");
    response1.set_content("key: value");

    // Mock the behavior of Read method
    EXPECT_CALL(*reader, Read(_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(response1), testing::Return(true)))
        .WillOnce(testing::Return(false)); // No more responses after the first one

    // Mock the Finish method to return an OK status
    EXPECT_CALL(*reader, Finish()).WillOnce(testing::Return(grpc::Status::OK));

    // Mock the gRPC stub behavior
    EXPECT_CALL(*stub, GetConfigsRaw(_, _)).WillOnce(testing::Return(reader));

    // Act & Assert
    std::vector<ConfigContent> configs;
    auto result = client->GetConfigs(ConfigType::JSON, &configs);

    EXPECT_EQ(result.status, AceClientStatus::OK);
    EXPECT_EQ(configs.size(), 1);
}

TEST_F(TestConfigClient, ClientGetTwoConfigs) {
    // Arrange
    // Prepare mock reader
    // It must not be deleted in this function, its ownership is handled by receiving types.
    auto reader = new MockConfigReader(); // Allocate on heap to match the return type

    // Mock the behavior of the reader
    A2XConfig response1;
    response1.set_name("config-b-1");
    response1.set_content("{\"key1\": \"value1\"}");

    // Mock the behavior of the reader
    A2XConfig response2;
    response2.set_name("config-b-2");
    response2.set_content("{\"key2\": \"value2\"}");

    // Mock the behavior of Read method
    EXPECT_CALL(*reader, Read(_))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(response1), testing::Return(true)))
        .WillOnce(testing::DoAll(testing::SetArgPointee<0>(response2), testing::Return(true)))
        .WillOnce(testing::Return(false)); // No more responses after the first one

    // Mock the Finish method to return an OK status
    EXPECT_CALL(*reader, Finish()).WillOnce(testing::Return(grpc::Status::OK));

    // Mock the gRPC stub behavior
    EXPECT_CALL(*stub, GetConfigsRaw(_, _)).WillOnce(testing::Return(reader));

    // Act & Assert
    std::vector<ConfigContent> configs;
    auto result = client->GetConfigs(ConfigType::JSON, &configs);

    EXPECT_EQ(result.status, AceClientStatus::OK);
    EXPECT_EQ(configs.size(), 2);

    auto entry1 = configs[0];
    EXPECT_EQ(entry1.type, ConfigType::JSON);
    EXPECT_EQ(entry1.name, "config-b-1");
    EXPECT_EQ(entry1.content, "{\"key1\": \"value1\"}");
    auto entry2 = configs[1];
    EXPECT_EQ(entry2.type, ConfigType::JSON);
    EXPECT_EQ(entry2.name, "config-b-2");
    EXPECT_EQ(entry2.content, "{\"key2\": \"value2\"}");
}

TEST(TestConfigClientFunctions, GetA2XConfig) {
    // Act & Assert
    std::vector<ConfigContent> configs1;
    auto result1 = mace::GetA2XConfig(
        ConfigType::JSON, &configs1, TEST_URL, API_KEY, FUNCTION_ID
    );

    EXPECT_EQ(result1.status, AceClientStatus::OK);
    EXPECT_GE(configs1.size(), 1);

    std::vector<ConfigContent> configs2;
    auto result2 = mace::GetA2XConfig(
        ConfigType::YAML, &configs2, TEST_URL, API_KEY, FUNCTION_ID
    );

    EXPECT_EQ(result2.status, AceClientStatus::OK);
    EXPECT_GE(configs2.size(), 1);
}

TEST(TestConfigClientFunctions, GetConfigTypeName) {
    // Act & Assert
    EXPECT_EQ(mace::GetConfigTypeName(ConfigType::JSON), "JSON");
    EXPECT_EQ(mace::GetConfigTypeName(ConfigType::YAML), "YAML");
    EXPECT_EQ(mace::GetConfigTypeName((ConfigType)3), "Unknown");
}

TEST(TestConfigClientFunctions, GetRequestConfigType) {
    // Act & Assert
    EXPECT_EQ(mace::GetRequestConfigType(ConfigType::JSON), ConfigsTypeRequest::JSON);
    EXPECT_EQ(mace::GetRequestConfigType(ConfigType::YAML), ConfigsTypeRequest::YAML);
}
