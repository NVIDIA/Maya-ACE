# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: nvidia_ace.services.animation_data.v1.proto
# Protobuf Python Version: 5.26.1
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from nvidia_ace.animation_data import v1_pb2 as nvidia__ace_dot_animation__data_dot_v1__pb2
from nvidia_ace.animation_id import v1_pb2 as nvidia__ace_dot_animation__id_dot_v1__pb2
from nvidia_ace.status import v1_pb2 as nvidia__ace_dot_status_dot_v1__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n+nvidia_ace.services.animation_data.v1.proto\x12%nvidia_ace.services.animation_data.v1\x1a\"nvidia_ace.animation_data.v1.proto\x1a nvidia_ace.animation_id.v1.proto\x1a\x1anvidia_ace.status.v1.proto2\x82\x02\n\x14\x41nimationDataService\x12n\n\x17PushAnimationDataStream\x12\x31.nvidia_ace.animation_data.v1.AnimationDataStream\x1a\x1c.nvidia_ace.status.v1.Status\"\x00(\x01\x12z\n\x17PullAnimationDataStream\x12(.nvidia_ace.animation_id.v1.AnimationIds\x1a\x31.nvidia_ace.animation_data.v1.AnimationDataStream\"\x00\x30\x01\x62\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'nvidia_ace.services.animation_data.v1_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  DESCRIPTOR._loaded_options = None
  _globals['_ANIMATIONDATASERVICE']._serialized_start=185
  _globals['_ANIMATIONDATASERVICE']._serialized_end=443
# @@protoc_insertion_point(module_scope)
