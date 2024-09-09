// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: nvidia_ace.services.animation_data.v1.proto

#ifndef GRPC_MOCK_nvidia_5face_2eservices_2eanimation_5fdata_2ev1_2eproto__INCLUDED
#define GRPC_MOCK_nvidia_5face_2eservices_2eanimation_5fdata_2ev1_2eproto__INCLUDED

#include "nvidia_ace.services.animation_data.v1.pb.h"
#include "nvidia_ace.services.animation_data.v1.grpc.pb.h"

#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/sync_stream.h>
#include <gmock/gmock.h>
namespace nvidia_ace {
namespace services {
namespace animation_data {
namespace v1 {

class MockAnimationDataServiceStub : public AnimationDataService::StubInterface {
 public:
  MOCK_METHOD2(PushAnimationDataStreamRaw, ::grpc::ClientWriterInterface< ::nvidia_ace::animation_data::v1::AnimationDataStream>*(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response));
  MOCK_METHOD4(AsyncPushAnimationDataStreamRaw, ::grpc::ClientAsyncWriterInterface< ::nvidia_ace::animation_data::v1::AnimationDataStream>*(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::CompletionQueue* cq, void* tag));
  MOCK_METHOD3(PrepareAsyncPushAnimationDataStreamRaw, ::grpc::ClientAsyncWriterInterface< ::nvidia_ace::animation_data::v1::AnimationDataStream>*(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::CompletionQueue* cq));
  MOCK_METHOD2(PullAnimationDataStreamRaw, ::grpc::ClientReaderInterface< ::nvidia_ace::animation_data::v1::AnimationDataStream>*(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds& request));
  MOCK_METHOD4(AsyncPullAnimationDataStreamRaw, ::grpc::ClientAsyncReaderInterface< ::nvidia_ace::animation_data::v1::AnimationDataStream>*(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds& request, ::grpc::CompletionQueue* cq, void* tag));
  MOCK_METHOD3(PrepareAsyncPullAnimationDataStreamRaw, ::grpc::ClientAsyncReaderInterface< ::nvidia_ace::animation_data::v1::AnimationDataStream>*(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds& request, ::grpc::CompletionQueue* cq));
};

}  // namespace v1
}  // namespace animation_data
}  // namespace services
}  // namespace nvidia_ace


#endif  // GRPC_MOCK_nvidia_5face_2eservices_2eanimation_5fdata_2ev1_2eproto__INCLUDED
