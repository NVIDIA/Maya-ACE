// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: health.proto
// Protobuf C++ Version: 5.26.0

#include "health.pb.h"

#include <algorithm>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
#include "google/protobuf/generated_message_tctable_impl.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace grpc {
namespace health {
namespace v1 {

inline constexpr HealthCheckResponse::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : status_{static_cast< ::grpc::health::v1::HealthCheckResponse_ServingStatus >(0)},
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR HealthCheckResponse::HealthCheckResponse(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct HealthCheckResponseDefaultTypeInternal {
  PROTOBUF_CONSTEXPR HealthCheckResponseDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~HealthCheckResponseDefaultTypeInternal() {}
  union {
    HealthCheckResponse _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 HealthCheckResponseDefaultTypeInternal _HealthCheckResponse_default_instance_;

inline constexpr HealthCheckRequest::Impl_::Impl_(
    ::_pbi::ConstantInitialized) noexcept
      : service_(
            &::google::protobuf::internal::fixed_address_empty_string,
            ::_pbi::ConstantInitialized()),
        _cached_size_{0} {}

template <typename>
PROTOBUF_CONSTEXPR HealthCheckRequest::HealthCheckRequest(::_pbi::ConstantInitialized)
    : _impl_(::_pbi::ConstantInitialized()) {}
struct HealthCheckRequestDefaultTypeInternal {
  PROTOBUF_CONSTEXPR HealthCheckRequestDefaultTypeInternal() : _instance(::_pbi::ConstantInitialized{}) {}
  ~HealthCheckRequestDefaultTypeInternal() {}
  union {
    HealthCheckRequest _instance;
  };
};

PROTOBUF_ATTRIBUTE_NO_DESTROY PROTOBUF_CONSTINIT
    PROTOBUF_ATTRIBUTE_INIT_PRIORITY1 HealthCheckRequestDefaultTypeInternal _HealthCheckRequest_default_instance_;
}  // namespace v1
}  // namespace health
}  // namespace grpc
static ::_pb::Metadata file_level_metadata_health_2eproto[2];
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_health_2eproto[1];
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_health_2eproto = nullptr;
const ::uint32_t
    TableStruct_health_2eproto::offsets[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
        protodesc_cold) = {
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::grpc::health::v1::HealthCheckRequest, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::grpc::health::v1::HealthCheckRequest, _impl_.service_),
        ~0u,  // no _has_bits_
        PROTOBUF_FIELD_OFFSET(::grpc::health::v1::HealthCheckResponse, _internal_metadata_),
        ~0u,  // no _extensions_
        ~0u,  // no _oneof_case_
        ~0u,  // no _weak_field_map_
        ~0u,  // no _inlined_string_donated_
        ~0u,  // no _split_
        ~0u,  // no sizeof(Split)
        PROTOBUF_FIELD_OFFSET(::grpc::health::v1::HealthCheckResponse, _impl_.status_),
};

static const ::_pbi::MigrationSchema
    schemas[] ABSL_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
        {0, -1, -1, sizeof(::grpc::health::v1::HealthCheckRequest)},
        {9, -1, -1, sizeof(::grpc::health::v1::HealthCheckResponse)},
};
static const ::_pb::Message* const file_default_instances[] = {
    &::grpc::health::v1::_HealthCheckRequest_default_instance_._instance,
    &::grpc::health::v1::_HealthCheckResponse_default_instance_._instance,
};
const char descriptor_table_protodef_health_2eproto[] ABSL_ATTRIBUTE_SECTION_VARIABLE(
    protodesc_cold) = {
    "\n\014health.proto\022\016grpc.health.v1\"%\n\022Health"
    "CheckRequest\022\017\n\007service\030\001 \001(\t\"\251\001\n\023Health"
    "CheckResponse\022A\n\006status\030\001 \001(\01621.grpc.hea"
    "lth.v1.HealthCheckResponse.ServingStatus"
    "\"O\n\rServingStatus\022\013\n\007UNKNOWN\020\000\022\013\n\007SERVIN"
    "G\020\001\022\017\n\013NOT_SERVING\020\002\022\023\n\017SERVICE_UNKNOWN\020"
    "\0032\256\001\n\006Health\022P\n\005Check\022\".grpc.health.v1.H"
    "ealthCheckRequest\032#.grpc.health.v1.Healt"
    "hCheckResponse\022R\n\005Watch\022\".grpc.health.v1"
    ".HealthCheckRequest\032#.grpc.health.v1.Hea"
    "lthCheckResponse0\001Ba\n\021io.grpc.health.v1B"
    "\013HealthProtoP\001Z,google.golang.org/grpc/h"
    "ealth/grpc_health_v1\252\002\016Grpc.Health.V1b\006p"
    "roto3"
};
static ::absl::once_flag descriptor_table_health_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_health_2eproto = {
    false,
    false,
    525,
    descriptor_table_protodef_health_2eproto,
    "health.proto",
    &descriptor_table_health_2eproto_once,
    nullptr,
    0,
    2,
    schemas,
    file_default_instances,
    TableStruct_health_2eproto::offsets,
    file_level_metadata_health_2eproto,
    file_level_enum_descriptors_health_2eproto,
    file_level_service_descriptors_health_2eproto,
};

// This function exists to be marked as weak.
// It can significantly speed up compilation by breaking up LLVM's SCC
// in the .pb.cc translation units. Large translation units see a
// reduction of more than 35% of walltime for optimized builds. Without
// the weak attribute all the messages in the file, including all the
// vtables and everything they use become part of the same SCC through
// a cycle like:
// GetMetadata -> descriptor table -> default instances ->
//   vtables -> GetMetadata
// By adding a weak function here we break the connection from the
// individual vtables back into the descriptor table.
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_health_2eproto_getter() {
  return &descriptor_table_health_2eproto;
}
namespace grpc {
namespace health {
namespace v1 {
const ::google::protobuf::EnumDescriptor* HealthCheckResponse_ServingStatus_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_health_2eproto);
  return file_level_enum_descriptors_health_2eproto[0];
}
PROTOBUF_CONSTINIT const uint32_t HealthCheckResponse_ServingStatus_internal_data_[] = {
    262144u, 0u, };
bool HealthCheckResponse_ServingStatus_IsValid(int value) {
  return 0 <= value && value <= 3;
}
#if (__cplusplus < 201703) && \
  (!defined(_MSC_VER) || (_MSC_VER >= 1900 && _MSC_VER < 1912))

constexpr HealthCheckResponse_ServingStatus HealthCheckResponse::UNKNOWN;
constexpr HealthCheckResponse_ServingStatus HealthCheckResponse::SERVING;
constexpr HealthCheckResponse_ServingStatus HealthCheckResponse::NOT_SERVING;
constexpr HealthCheckResponse_ServingStatus HealthCheckResponse::SERVICE_UNKNOWN;
constexpr HealthCheckResponse_ServingStatus HealthCheckResponse::ServingStatus_MIN;
constexpr HealthCheckResponse_ServingStatus HealthCheckResponse::ServingStatus_MAX;
constexpr int HealthCheckResponse::ServingStatus_ARRAYSIZE;

#endif  // (__cplusplus < 201703) &&
        // (!defined(_MSC_VER) || (_MSC_VER >= 1900 && _MSC_VER < 1912))
// ===================================================================

class HealthCheckRequest::_Internal {
 public:
};

HealthCheckRequest::HealthCheckRequest(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:grpc.health.v1.HealthCheckRequest)
}
inline PROTOBUF_NDEBUG_INLINE HealthCheckRequest::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility, ::google::protobuf::Arena* arena,
    const Impl_& from)
      : service_(arena, from.service_),
        _cached_size_{0} {}

HealthCheckRequest::HealthCheckRequest(
    ::google::protobuf::Arena* arena,
    const HealthCheckRequest& from)
    : ::google::protobuf::Message(arena) {
  HealthCheckRequest* const _this = this;
  (void)_this;
  _internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(
      from._internal_metadata_);
  new (&_impl_) Impl_(internal_visibility(), arena, from._impl_);

  // @@protoc_insertion_point(copy_constructor:grpc.health.v1.HealthCheckRequest)
}
inline PROTOBUF_NDEBUG_INLINE HealthCheckRequest::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : service_(arena),
        _cached_size_{0} {}

inline void HealthCheckRequest::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
}
HealthCheckRequest::~HealthCheckRequest() {
  // @@protoc_insertion_point(destructor:grpc.health.v1.HealthCheckRequest)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void HealthCheckRequest::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.service_.Destroy();
  _impl_.~Impl_();
}

const ::google::protobuf::MessageLite::ClassData*
HealthCheckRequest::GetClassData() const {
  PROTOBUF_CONSTINIT static const ::google::protobuf::MessageLite::
      ClassDataFull _data_ = {
          {
              nullptr,  // OnDemandRegisterArenaDtor
              PROTOBUF_FIELD_OFFSET(HealthCheckRequest, _impl_._cached_size_),
              false,
          },
          &HealthCheckRequest::MergeImpl,
          &HealthCheckRequest::kDescriptorMethods,
      };
  return &_data_;
}
PROTOBUF_NOINLINE void HealthCheckRequest::Clear() {
// @@protoc_insertion_point(message_clear_start:grpc.health.v1.HealthCheckRequest)
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.service_.ClearToEmpty();
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

const char* HealthCheckRequest::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<0, 1, 0, 49, 2> HealthCheckRequest::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    1, 0,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967294,  // skipmap
    offsetof(decltype(_table_), field_entries),
    1,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    &_HealthCheckRequest_default_instance_._instance,
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::grpc::health::v1::HealthCheckRequest>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    // string service = 1;
    {::_pbi::TcParser::FastUS1,
     {10, 63, 0, PROTOBUF_FIELD_OFFSET(HealthCheckRequest, _impl_.service_)}},
  }}, {{
    65535, 65535
  }}, {{
    // string service = 1;
    {PROTOBUF_FIELD_OFFSET(HealthCheckRequest, _impl_.service_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kUtf8String | ::_fl::kRepAString)},
  }},
  // no aux_entries
  {{
    "\41\7\0\0\0\0\0\0"
    "grpc.health.v1.HealthCheckRequest"
    "service"
  }},
};

::uint8_t* HealthCheckRequest::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:grpc.health.v1.HealthCheckRequest)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // string service = 1;
  if (!this->_internal_service().empty()) {
    const std::string& _s = this->_internal_service();
    ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(
        _s.data(), static_cast<int>(_s.length()), ::google::protobuf::internal::WireFormatLite::SERIALIZE, "grpc.health.v1.HealthCheckRequest.service");
    target = stream->WriteStringMaybeAliased(1, _s, target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:grpc.health.v1.HealthCheckRequest)
  return target;
}

::size_t HealthCheckRequest::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:grpc.health.v1.HealthCheckRequest)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // string service = 1;
  if (!this->_internal_service().empty()) {
    total_size += 1 + ::google::protobuf::internal::WireFormatLite::StringSize(
                                    this->_internal_service());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}


void HealthCheckRequest::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<HealthCheckRequest*>(&to_msg);
  auto& from = static_cast<const HealthCheckRequest&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:grpc.health.v1.HealthCheckRequest)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (!from._internal_service().empty()) {
    _this->_internal_set_service(from._internal_service());
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void HealthCheckRequest::CopyFrom(const HealthCheckRequest& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:grpc.health.v1.HealthCheckRequest)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool HealthCheckRequest::IsInitialized() const {
  return true;
}

void HealthCheckRequest::InternalSwap(HealthCheckRequest* PROTOBUF_RESTRICT other) {
  using std::swap;
  auto* arena = GetArena();
  ABSL_DCHECK_EQ(arena, other->GetArena());
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  ::_pbi::ArenaStringPtr::InternalSwap(&_impl_.service_, &other->_impl_.service_, arena);
}

::google::protobuf::Metadata HealthCheckRequest::GetMetadata() const {
  return ::_pbi::AssignDescriptors(&descriptor_table_health_2eproto_getter,
                                   &descriptor_table_health_2eproto_once,
                                   file_level_metadata_health_2eproto[0]);
}
// ===================================================================

class HealthCheckResponse::_Internal {
 public:
};

HealthCheckResponse::HealthCheckResponse(::google::protobuf::Arena* arena)
    : ::google::protobuf::Message(arena) {
  SharedCtor(arena);
  // @@protoc_insertion_point(arena_constructor:grpc.health.v1.HealthCheckResponse)
}
HealthCheckResponse::HealthCheckResponse(
    ::google::protobuf::Arena* arena, const HealthCheckResponse& from)
    : HealthCheckResponse(arena) {
  MergeFrom(from);
}
inline PROTOBUF_NDEBUG_INLINE HealthCheckResponse::Impl_::Impl_(
    ::google::protobuf::internal::InternalVisibility visibility,
    ::google::protobuf::Arena* arena)
      : _cached_size_{0} {}

inline void HealthCheckResponse::SharedCtor(::_pb::Arena* arena) {
  new (&_impl_) Impl_(internal_visibility(), arena);
  _impl_.status_ = {};
}
HealthCheckResponse::~HealthCheckResponse() {
  // @@protoc_insertion_point(destructor:grpc.health.v1.HealthCheckResponse)
  _internal_metadata_.Delete<::google::protobuf::UnknownFieldSet>();
  SharedDtor();
}
inline void HealthCheckResponse::SharedDtor() {
  ABSL_DCHECK(GetArena() == nullptr);
  _impl_.~Impl_();
}

const ::google::protobuf::MessageLite::ClassData*
HealthCheckResponse::GetClassData() const {
  PROTOBUF_CONSTINIT static const ::google::protobuf::MessageLite::
      ClassDataFull _data_ = {
          {
              nullptr,  // OnDemandRegisterArenaDtor
              PROTOBUF_FIELD_OFFSET(HealthCheckResponse, _impl_._cached_size_),
              false,
          },
          &HealthCheckResponse::MergeImpl,
          &HealthCheckResponse::kDescriptorMethods,
      };
  return &_data_;
}
PROTOBUF_NOINLINE void HealthCheckResponse::Clear() {
// @@protoc_insertion_point(message_clear_start:grpc.health.v1.HealthCheckResponse)
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  _impl_.status_ = 0;
  _internal_metadata_.Clear<::google::protobuf::UnknownFieldSet>();
}

const char* HealthCheckResponse::_InternalParse(
    const char* ptr, ::_pbi::ParseContext* ctx) {
  ptr = ::_pbi::TcParser::ParseLoop(this, ptr, ctx, &_table_.header);
  return ptr;
}


PROTOBUF_CONSTINIT PROTOBUF_ATTRIBUTE_INIT_PRIORITY1
const ::_pbi::TcParseTable<0, 1, 0, 0, 2> HealthCheckResponse::_table_ = {
  {
    0,  // no _has_bits_
    0, // no _extensions_
    1, 0,  // max_field_number, fast_idx_mask
    offsetof(decltype(_table_), field_lookup_table),
    4294967294,  // skipmap
    offsetof(decltype(_table_), field_entries),
    1,  // num_field_entries
    0,  // num_aux_entries
    offsetof(decltype(_table_), field_names),  // no aux_entries
    &_HealthCheckResponse_default_instance_._instance,
    ::_pbi::TcParser::GenericFallback,  // fallback
    #ifdef PROTOBUF_PREFETCH_PARSE_TABLE
    ::_pbi::TcParser::GetTable<::grpc::health::v1::HealthCheckResponse>(),  // to_prefetch
    #endif  // PROTOBUF_PREFETCH_PARSE_TABLE
  }, {{
    // .grpc.health.v1.HealthCheckResponse.ServingStatus status = 1;
    {::_pbi::TcParser::SingularVarintNoZag1<::uint32_t, offsetof(HealthCheckResponse, _impl_.status_), 63>(),
     {8, 63, 0, PROTOBUF_FIELD_OFFSET(HealthCheckResponse, _impl_.status_)}},
  }}, {{
    65535, 65535
  }}, {{
    // .grpc.health.v1.HealthCheckResponse.ServingStatus status = 1;
    {PROTOBUF_FIELD_OFFSET(HealthCheckResponse, _impl_.status_), 0, 0,
    (0 | ::_fl::kFcSingular | ::_fl::kOpenEnum)},
  }},
  // no aux_entries
  {{
  }},
};

::uint8_t* HealthCheckResponse::_InternalSerialize(
    ::uint8_t* target,
    ::google::protobuf::io::EpsCopyOutputStream* stream) const {
  // @@protoc_insertion_point(serialize_to_array_start:grpc.health.v1.HealthCheckResponse)
  ::uint32_t cached_has_bits = 0;
  (void)cached_has_bits;

  // .grpc.health.v1.HealthCheckResponse.ServingStatus status = 1;
  if (this->_internal_status() != 0) {
    target = stream->EnsureSpace(target);
    target = ::_pbi::WireFormatLite::WriteEnumToArray(
        1, this->_internal_status(), target);
  }

  if (PROTOBUF_PREDICT_FALSE(_internal_metadata_.have_unknown_fields())) {
    target =
        ::_pbi::WireFormat::InternalSerializeUnknownFieldsToArray(
            _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance), target, stream);
  }
  // @@protoc_insertion_point(serialize_to_array_end:grpc.health.v1.HealthCheckResponse)
  return target;
}

::size_t HealthCheckResponse::ByteSizeLong() const {
// @@protoc_insertion_point(message_byte_size_start:grpc.health.v1.HealthCheckResponse)
  ::size_t total_size = 0;

  ::uint32_t cached_has_bits = 0;
  // Prevent compiler warnings about cached_has_bits being unused
  (void) cached_has_bits;

  // .grpc.health.v1.HealthCheckResponse.ServingStatus status = 1;
  if (this->_internal_status() != 0) {
    total_size += 1 +
                  ::_pbi::WireFormatLite::EnumSize(this->_internal_status());
  }

  return MaybeComputeUnknownFieldsSize(total_size, &_impl_._cached_size_);
}


void HealthCheckResponse::MergeImpl(::google::protobuf::MessageLite& to_msg, const ::google::protobuf::MessageLite& from_msg) {
  auto* const _this = static_cast<HealthCheckResponse*>(&to_msg);
  auto& from = static_cast<const HealthCheckResponse&>(from_msg);
  // @@protoc_insertion_point(class_specific_merge_from_start:grpc.health.v1.HealthCheckResponse)
  ABSL_DCHECK_NE(&from, _this);
  ::uint32_t cached_has_bits = 0;
  (void) cached_has_bits;

  if (from._internal_status() != 0) {
    _this->_impl_.status_ = from._impl_.status_;
  }
  _this->_internal_metadata_.MergeFrom<::google::protobuf::UnknownFieldSet>(from._internal_metadata_);
}

void HealthCheckResponse::CopyFrom(const HealthCheckResponse& from) {
// @@protoc_insertion_point(class_specific_copy_from_start:grpc.health.v1.HealthCheckResponse)
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

PROTOBUF_NOINLINE bool HealthCheckResponse::IsInitialized() const {
  return true;
}

void HealthCheckResponse::InternalSwap(HealthCheckResponse* PROTOBUF_RESTRICT other) {
  using std::swap;
  _internal_metadata_.InternalSwap(&other->_internal_metadata_);
  swap(_impl_.status_, other->_impl_.status_);
}

::google::protobuf::Metadata HealthCheckResponse::GetMetadata() const {
  return ::_pbi::AssignDescriptors(&descriptor_table_health_2eproto_getter,
                                   &descriptor_table_health_2eproto_once,
                                   file_level_metadata_health_2eproto[1]);
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace v1
}  // namespace health
}  // namespace grpc
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
static ::std::false_type _static_init_ PROTOBUF_UNUSED =
    (::_pbi::AddDescriptors(&descriptor_table_health_2eproto),
     ::std::false_type{});
#include "google/protobuf/port_undef.inc"
