// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: nvidia_ace.services.animation_controller.v1.proto

#include "nvidia_ace.services.animation_controller.v1.pb.h"
#include "nvidia_ace.services.animation_controller.v1.grpc.pb.h"

#include <functional>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/impl/channel_interface.h>
#include <grpcpp/impl/client_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/rpc_service_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/sync_stream.h>
namespace nvidia_ace {
namespace services {
namespace animation_controller {
namespace v1 {

static const char* AnimationControllerService_method_names[] = {
  "/nvidia_ace.services.animation_controller.v1.AnimationControllerService/RequestAnimationIds",
  "/nvidia_ace.services.animation_controller.v1.AnimationControllerService/PullAnimationDataStream",
  "/nvidia_ace.services.animation_controller.v1.AnimationControllerService/PushAudioStream",
  "/nvidia_ace.services.animation_controller.v1.AnimationControllerService/UpdateAnimationGraphVariable",
};

std::unique_ptr< AnimationControllerService::Stub> AnimationControllerService::NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr< AnimationControllerService::Stub> stub(new AnimationControllerService::Stub(channel, options));
  return stub;
}

AnimationControllerService::Stub::Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options)
  : channel_(channel), rpcmethod_RequestAnimationIds_(AnimationControllerService_method_names[0], options.suffix_for_stats(),::grpc::internal::RpcMethod::NORMAL_RPC, channel)
  , rpcmethod_PullAnimationDataStream_(AnimationControllerService_method_names[1], options.suffix_for_stats(),::grpc::internal::RpcMethod::SERVER_STREAMING, channel)
  , rpcmethod_PushAudioStream_(AnimationControllerService_method_names[2], options.suffix_for_stats(),::grpc::internal::RpcMethod::CLIENT_STREAMING, channel)
  , rpcmethod_UpdateAnimationGraphVariable_(AnimationControllerService_method_names[3], options.suffix_for_stats(),::grpc::internal::RpcMethod::CLIENT_STREAMING, channel)
  {}

::grpc::Status AnimationControllerService::Stub::RequestAnimationIds(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus* response) {
  return ::grpc::internal::BlockingUnaryCall< ::google::protobuf::Empty, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), rpcmethod_RequestAnimationIds_, context, request, response);
}

void AnimationControllerService::Stub::async::RequestAnimationIds(::grpc::ClientContext* context, const ::google::protobuf::Empty* request, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus* response, std::function<void(::grpc::Status)> f) {
  ::grpc::internal::CallbackUnaryCall< ::google::protobuf::Empty, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_RequestAnimationIds_, context, request, response, std::move(f));
}

void AnimationControllerService::Stub::async::RequestAnimationIds(::grpc::ClientContext* context, const ::google::protobuf::Empty* request, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus* response, ::grpc::ClientUnaryReactor* reactor) {
  ::grpc::internal::ClientCallbackUnaryFactory::Create< ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(stub_->channel_.get(), stub_->rpcmethod_RequestAnimationIds_, context, request, response, reactor);
}

::grpc::ClientAsyncResponseReader< ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus>* AnimationControllerService::Stub::PrepareAsyncRequestAnimationIdsRaw(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncResponseReaderHelper::Create< ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus, ::google::protobuf::Empty, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(channel_.get(), cq, rpcmethod_RequestAnimationIds_, context, request);
}

::grpc::ClientAsyncResponseReader< ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus>* AnimationControllerService::Stub::AsyncRequestAnimationIdsRaw(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) {
  auto* result =
    this->PrepareAsyncRequestAnimationIdsRaw(context, request, cq);
  result->StartCall();
  return result;
}

::grpc::ClientReader< ::nvidia_ace::animation_data::v1::AnimationDataStream>* AnimationControllerService::Stub::PullAnimationDataStreamRaw(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds& request) {
  return ::grpc::internal::ClientReaderFactory< ::nvidia_ace::animation_data::v1::AnimationDataStream>::Create(channel_.get(), rpcmethod_PullAnimationDataStream_, context, request);
}

void AnimationControllerService::Stub::async::PullAnimationDataStream(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds* request, ::grpc::ClientReadReactor< ::nvidia_ace::animation_data::v1::AnimationDataStream>* reactor) {
  ::grpc::internal::ClientCallbackReaderFactory< ::nvidia_ace::animation_data::v1::AnimationDataStream>::Create(stub_->channel_.get(), stub_->rpcmethod_PullAnimationDataStream_, context, request, reactor);
}

::grpc::ClientAsyncReader< ::nvidia_ace::animation_data::v1::AnimationDataStream>* AnimationControllerService::Stub::AsyncPullAnimationDataStreamRaw(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds& request, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::nvidia_ace::animation_data::v1::AnimationDataStream>::Create(channel_.get(), cq, rpcmethod_PullAnimationDataStream_, context, request, true, tag);
}

::grpc::ClientAsyncReader< ::nvidia_ace::animation_data::v1::AnimationDataStream>* AnimationControllerService::Stub::PrepareAsyncPullAnimationDataStreamRaw(::grpc::ClientContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds& request, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncReaderFactory< ::nvidia_ace::animation_data::v1::AnimationDataStream>::Create(channel_.get(), cq, rpcmethod_PullAnimationDataStream_, context, request, false, nullptr);
}

::grpc::ClientWriter< ::nvidia_ace::a2f::v1::AudioStream>* AnimationControllerService::Stub::PushAudioStreamRaw(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response) {
  return ::grpc::internal::ClientWriterFactory< ::nvidia_ace::a2f::v1::AudioStream>::Create(channel_.get(), rpcmethod_PushAudioStream_, context, response);
}

void AnimationControllerService::Stub::async::PushAudioStream(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::ClientWriteReactor< ::nvidia_ace::a2f::v1::AudioStream>* reactor) {
  ::grpc::internal::ClientCallbackWriterFactory< ::nvidia_ace::a2f::v1::AudioStream>::Create(stub_->channel_.get(), stub_->rpcmethod_PushAudioStream_, context, response, reactor);
}

::grpc::ClientAsyncWriter< ::nvidia_ace::a2f::v1::AudioStream>* AnimationControllerService::Stub::AsyncPushAudioStreamRaw(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncWriterFactory< ::nvidia_ace::a2f::v1::AudioStream>::Create(channel_.get(), cq, rpcmethod_PushAudioStream_, context, response, true, tag);
}

::grpc::ClientAsyncWriter< ::nvidia_ace::a2f::v1::AudioStream>* AnimationControllerService::Stub::PrepareAsyncPushAudioStreamRaw(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncWriterFactory< ::nvidia_ace::a2f::v1::AudioStream>::Create(channel_.get(), cq, rpcmethod_PushAudioStream_, context, response, false, nullptr);
}

::grpc::ClientWriter< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>* AnimationControllerService::Stub::UpdateAnimationGraphVariableRaw(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response) {
  return ::grpc::internal::ClientWriterFactory< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>::Create(channel_.get(), rpcmethod_UpdateAnimationGraphVariable_, context, response);
}

void AnimationControllerService::Stub::async::UpdateAnimationGraphVariable(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::ClientWriteReactor< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>* reactor) {
  ::grpc::internal::ClientCallbackWriterFactory< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>::Create(stub_->channel_.get(), stub_->rpcmethod_UpdateAnimationGraphVariable_, context, response, reactor);
}

::grpc::ClientAsyncWriter< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>* AnimationControllerService::Stub::AsyncUpdateAnimationGraphVariableRaw(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::CompletionQueue* cq, void* tag) {
  return ::grpc::internal::ClientAsyncWriterFactory< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>::Create(channel_.get(), cq, rpcmethod_UpdateAnimationGraphVariable_, context, response, true, tag);
}

::grpc::ClientAsyncWriter< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>* AnimationControllerService::Stub::PrepareAsyncUpdateAnimationGraphVariableRaw(::grpc::ClientContext* context, ::nvidia_ace::status::v1::Status* response, ::grpc::CompletionQueue* cq) {
  return ::grpc::internal::ClientAsyncWriterFactory< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>::Create(channel_.get(), cq, rpcmethod_UpdateAnimationGraphVariable_, context, response, false, nullptr);
}

AnimationControllerService::Service::Service() {
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AnimationControllerService_method_names[0],
      ::grpc::internal::RpcMethod::NORMAL_RPC,
      new ::grpc::internal::RpcMethodHandler< AnimationControllerService::Service, ::google::protobuf::Empty, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus, ::grpc::protobuf::MessageLite, ::grpc::protobuf::MessageLite>(
          [](AnimationControllerService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::google::protobuf::Empty* req,
             ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus* resp) {
               return service->RequestAnimationIds(ctx, req, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AnimationControllerService_method_names[1],
      ::grpc::internal::RpcMethod::SERVER_STREAMING,
      new ::grpc::internal::ServerStreamingHandler< AnimationControllerService::Service, ::nvidia_ace::animation_id::v1::AnimationIds, ::nvidia_ace::animation_data::v1::AnimationDataStream>(
          [](AnimationControllerService::Service* service,
             ::grpc::ServerContext* ctx,
             const ::nvidia_ace::animation_id::v1::AnimationIds* req,
             ::grpc::ServerWriter<::nvidia_ace::animation_data::v1::AnimationDataStream>* writer) {
               return service->PullAnimationDataStream(ctx, req, writer);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AnimationControllerService_method_names[2],
      ::grpc::internal::RpcMethod::CLIENT_STREAMING,
      new ::grpc::internal::ClientStreamingHandler< AnimationControllerService::Service, ::nvidia_ace::a2f::v1::AudioStream, ::nvidia_ace::status::v1::Status>(
          [](AnimationControllerService::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReader<::nvidia_ace::a2f::v1::AudioStream>* reader,
             ::nvidia_ace::status::v1::Status* resp) {
               return service->PushAudioStream(ctx, reader, resp);
             }, this)));
  AddMethod(new ::grpc::internal::RpcServiceMethod(
      AnimationControllerService_method_names[3],
      ::grpc::internal::RpcMethod::CLIENT_STREAMING,
      new ::grpc::internal::ClientStreamingHandler< AnimationControllerService::Service, ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest, ::nvidia_ace::status::v1::Status>(
          [](AnimationControllerService::Service* service,
             ::grpc::ServerContext* ctx,
             ::grpc::ServerReader<::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>* reader,
             ::nvidia_ace::status::v1::Status* resp) {
               return service->UpdateAnimationGraphVariable(ctx, reader, resp);
             }, this)));
}

AnimationControllerService::Service::~Service() {
}

::grpc::Status AnimationControllerService::Service::RequestAnimationIds(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, ::nvidia_ace::services::animation_controller::v1::AnimationIdsOrStatus* response) {
  (void) context;
  (void) request;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status AnimationControllerService::Service::PullAnimationDataStream(::grpc::ServerContext* context, const ::nvidia_ace::animation_id::v1::AnimationIds* request, ::grpc::ServerWriter< ::nvidia_ace::animation_data::v1::AnimationDataStream>* writer) {
  (void) context;
  (void) request;
  (void) writer;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status AnimationControllerService::Service::PushAudioStream(::grpc::ServerContext* context, ::grpc::ServerReader< ::nvidia_ace::a2f::v1::AudioStream>* reader, ::nvidia_ace::status::v1::Status* response) {
  (void) context;
  (void) reader;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

::grpc::Status AnimationControllerService::Service::UpdateAnimationGraphVariable(::grpc::ServerContext* context, ::grpc::ServerReader< ::nvidia_ace::services::animation_controller::v1::AnimationGraphRequest>* reader, ::nvidia_ace::status::v1::Status* response) {
  (void) context;
  (void) reader;
  (void) response;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}


}  // namespace nvidia_ace
}  // namespace services
}  // namespace animation_controller
}  // namespace v1

