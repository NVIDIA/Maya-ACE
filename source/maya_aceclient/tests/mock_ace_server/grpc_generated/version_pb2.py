# -*- coding: utf-8 -*-
# Generated by the protocol buffer compiler.  DO NOT EDIT!
# source: version.proto
# Protobuf Python Version: 5.26.1
"""Generated protocol buffer code."""
from google.protobuf import descriptor as _descriptor
from google.protobuf import descriptor_pool as _descriptor_pool
from google.protobuf import symbol_database as _symbol_database
from google.protobuf.internal import builder as _builder
# @@protoc_insertion_point(imports)

_sym_db = _symbol_database.Default()


from google.protobuf import empty_pb2 as google_dot_protobuf_dot_empty__pb2


DESCRIPTOR = _descriptor_pool.Default().AddSerializedFile(b'\n\rversion.proto\x12\x1envidia_ace.services.version.v1\x1a\x1bgoogle/protobuf/empty.proto\"_\n\x12VersionInformation\x12I\n\x13version_information\x18\x01 \x03(\x0b\x32,.nvidia_ace.services.version.v1.VersionEntry\"5\n\x0cVersionEntry\x12\x14\n\x0cservice_name\x18\x01 \x01(\t\x12\x0f\n\x07version\x18\x02 \x01(\t2\x82\x01\n\x19VersionInformationService\x12\x65\n\x15GetVersionInformation\x12\x16.google.protobuf.Empty\x1a\x32.nvidia_ace.services.version.v1.VersionInformation\"\x00\x62\x06proto3')

_globals = globals()
_builder.BuildMessageAndEnumDescriptors(DESCRIPTOR, _globals)
_builder.BuildTopDescriptorsAndMessages(DESCRIPTOR, 'version_pb2', _globals)
if not _descriptor._USE_C_DESCRIPTORS:
  DESCRIPTOR._loaded_options = None
  _globals['_VERSIONINFORMATION']._serialized_start=78
  _globals['_VERSIONINFORMATION']._serialized_end=173
  _globals['_VERSIONENTRY']._serialized_start=175
  _globals['_VERSIONENTRY']._serialized_end=228
  _globals['_VERSIONINFORMATIONSERVICE']._serialized_start=231
  _globals['_VERSIONINFORMATIONSERVICE']._serialized_end=361
# @@protoc_insertion_point(module_scope)
