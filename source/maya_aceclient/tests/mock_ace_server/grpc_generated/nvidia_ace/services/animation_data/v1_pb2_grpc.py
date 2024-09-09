# Generated by the gRPC Python protocol compiler plugin. DO NOT EDIT!
"""Client and server classes corresponding to protobuf-defined services."""
import grpc
import warnings

from nvidia_ace.animation_data import v1_pb2 as nvidia__ace_dot_animation__data_dot_v1__pb2
from nvidia_ace.animation_id import v1_pb2 as nvidia__ace_dot_animation__id_dot_v1__pb2
from nvidia_ace.status import v1_pb2 as nvidia__ace_dot_status_dot_v1__pb2

GRPC_GENERATED_VERSION = '1.64.0'
GRPC_VERSION = grpc.__version__
EXPECTED_ERROR_RELEASE = '1.65.0'
SCHEDULED_RELEASE_DATE = 'June 25, 2024'
_version_not_supported = False

try:
    from grpc._utilities import first_version_is_lower
    _version_not_supported = first_version_is_lower(GRPC_VERSION, GRPC_GENERATED_VERSION)
except ImportError:
    _version_not_supported = True

if _version_not_supported:
    warnings.warn(
        f'The grpc package installed is at version {GRPC_VERSION},'
        + f' but the generated code in nvidia_ace.services.animation_data.v1_pb2_grpc.py depends on'
        + f' grpcio>={GRPC_GENERATED_VERSION}.'
        + f' Please upgrade your grpc module to grpcio>={GRPC_GENERATED_VERSION}'
        + f' or downgrade your generated code using grpcio-tools<={GRPC_VERSION}.'
        + f' This warning will become an error in {EXPECTED_ERROR_RELEASE},'
        + f' scheduled for release on {SCHEDULED_RELEASE_DATE}.',
        RuntimeWarning
    )


class AnimationDataServiceStub(object):
    """2 RPC exist to provide a stream of animation data
    The RPC to implement depends on if the part of the service
    is a client or a server.
    E.g.: In the case of Animation Graph Microservice, we implement both RPCs.
    One to receive and one to send.
    """

    def __init__(self, channel):
        """Constructor.

        Args:
            channel: A grpc.Channel.
        """
        self.PushAnimationDataStream = channel.stream_unary(
                '/nvidia_ace.services.animation_data.v1.AnimationDataService/PushAnimationDataStream',
                request_serializer=nvidia__ace_dot_animation__data_dot_v1__pb2.AnimationDataStream.SerializeToString,
                response_deserializer=nvidia__ace_dot_status_dot_v1__pb2.Status.FromString,
                _registered_method=True)
        self.PullAnimationDataStream = channel.unary_stream(
                '/nvidia_ace.services.animation_data.v1.AnimationDataService/PullAnimationDataStream',
                request_serializer=nvidia__ace_dot_animation__id_dot_v1__pb2.AnimationIds.SerializeToString,
                response_deserializer=nvidia__ace_dot_animation__data_dot_v1__pb2.AnimationDataStream.FromString,
                _registered_method=True)


class AnimationDataServiceServicer(object):
    """2 RPC exist to provide a stream of animation data
    The RPC to implement depends on if the part of the service
    is a client or a server.
    E.g.: In the case of Animation Graph Microservice, we implement both RPCs.
    One to receive and one to send.
    """

    def PushAnimationDataStream(self, request_iterator, context):
        """When the service creating the animation data is a client from the service receiving them
        This push RPC must be used.
        An example for that is Audio2Face Microservice creating animation data and sending them
        to Animation Graph Microservice
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')

    def PullAnimationDataStream(self, request, context):
        """When the service creating the animation data is a server from the service receiving them
        This pull RPC must be used.
        An example for that is the Omniverse Renderer Microservice requesting animation data to the
        Animation Graph Microservice.
        """
        context.set_code(grpc.StatusCode.UNIMPLEMENTED)
        context.set_details('Method not implemented!')
        raise NotImplementedError('Method not implemented!')


def add_AnimationDataServiceServicer_to_server(servicer, server):
    rpc_method_handlers = {
            'PushAnimationDataStream': grpc.stream_unary_rpc_method_handler(
                    servicer.PushAnimationDataStream,
                    request_deserializer=nvidia__ace_dot_animation__data_dot_v1__pb2.AnimationDataStream.FromString,
                    response_serializer=nvidia__ace_dot_status_dot_v1__pb2.Status.SerializeToString,
            ),
            'PullAnimationDataStream': grpc.unary_stream_rpc_method_handler(
                    servicer.PullAnimationDataStream,
                    request_deserializer=nvidia__ace_dot_animation__id_dot_v1__pb2.AnimationIds.FromString,
                    response_serializer=nvidia__ace_dot_animation__data_dot_v1__pb2.AnimationDataStream.SerializeToString,
            ),
    }
    generic_handler = grpc.method_handlers_generic_handler(
            'nvidia_ace.services.animation_data.v1.AnimationDataService', rpc_method_handlers)
    server.add_generic_rpc_handlers((generic_handler,))
    server.add_registered_method_handlers('nvidia_ace.services.animation_data.v1.AnimationDataService', rpc_method_handlers)


 # This class is part of an EXPERIMENTAL API.
class AnimationDataService(object):
    """2 RPC exist to provide a stream of animation data
    The RPC to implement depends on if the part of the service
    is a client or a server.
    E.g.: In the case of Animation Graph Microservice, we implement both RPCs.
    One to receive and one to send.
    """

    @staticmethod
    def PushAnimationDataStream(request_iterator,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.stream_unary(
            request_iterator,
            target,
            '/nvidia_ace.services.animation_data.v1.AnimationDataService/PushAnimationDataStream',
            nvidia__ace_dot_animation__data_dot_v1__pb2.AnimationDataStream.SerializeToString,
            nvidia__ace_dot_status_dot_v1__pb2.Status.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)

    @staticmethod
    def PullAnimationDataStream(request,
            target,
            options=(),
            channel_credentials=None,
            call_credentials=None,
            insecure=False,
            compression=None,
            wait_for_ready=None,
            timeout=None,
            metadata=None):
        return grpc.experimental.unary_stream(
            request,
            target,
            '/nvidia_ace.services.animation_data.v1.AnimationDataService/PullAnimationDataStream',
            nvidia__ace_dot_animation__id_dot_v1__pb2.AnimationIds.SerializeToString,
            nvidia__ace_dot_animation__data_dot_v1__pb2.AnimationDataStream.FromString,
            options,
            channel_credentials,
            insecure,
            call_credentials,
            compression,
            wait_for_ready,
            timeout,
            metadata,
            _registered_method=True)
