// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: version.proto

#ifndef GRPC_MOCK_version_2eproto__INCLUDED
#define GRPC_MOCK_version_2eproto__INCLUDED

#include "version.pb.h"
#include "version.grpc.pb.h"

#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/sync_stream.h>
#include <gmock/gmock.h>
namespace nvidia_ace {
namespace services {
namespace version {
namespace v1 {

class MockVersionInformationServiceStub : public VersionInformationService::StubInterface {
 public:
  MOCK_METHOD3(GetVersionInformation, ::grpc::Status(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::nvidia_ace::services::version::v1::VersionInformation* response));
  MOCK_METHOD3(AsyncGetVersionInformationRaw, ::grpc::ClientAsyncResponseReaderInterface< ::nvidia_ace::services::version::v1::VersionInformation>*(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq));
  MOCK_METHOD3(PrepareAsyncGetVersionInformationRaw, ::grpc::ClientAsyncResponseReaderInterface< ::nvidia_ace::services::version::v1::VersionInformation>*(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq));
};

}  // namespace v1
}  // namespace version
}  // namespace services
}  // namespace nvidia_ace


#endif  // GRPC_MOCK_version_2eproto__INCLUDED
