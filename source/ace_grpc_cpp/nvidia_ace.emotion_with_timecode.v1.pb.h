// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: nvidia_ace.emotion_with_timecode.v1.proto
// Protobuf C++ Version: 5.26.0

#ifndef GOOGLE_PROTOBUF_INCLUDED_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto_2epb_2eh

#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "google/protobuf/port_def.inc"
#if PROTOBUF_VERSION != 5026000
#error "Protobuf C++ gencode is built with an incompatible version of"
#error "Protobuf C++ headers/runtime. See"
#error "https://protobuf.dev/support/cross-version-runtime-guarantee/#cpp"
#endif
#include "google/protobuf/port_undef.inc"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/arena.h"
#include "google/protobuf/arenastring.h"
#include "google/protobuf/generated_message_tctable_decl.h"
#include "google/protobuf/generated_message_util.h"
#include "google/protobuf/metadata_lite.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/message.h"
#include "google/protobuf/repeated_field.h"  // IWYU pragma: export
#include "google/protobuf/extension_set.h"  // IWYU pragma: export
#include "google/protobuf/map.h"  // IWYU pragma: export
#include "google/protobuf/map_entry.h"
#include "google/protobuf/map_field_inl.h"
#include "google/protobuf/unknown_field_set.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"

#define PROTOBUF_INTERNAL_EXPORT_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto {
  static const ::uint32_t offsets[];
};
extern const ::google::protobuf::internal::DescriptorTable
    descriptor_table_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto;
namespace nvidia_ace {
namespace emotion_with_timecode {
namespace v1 {
class EmotionWithTimeCode;
struct EmotionWithTimeCodeDefaultTypeInternal;
extern EmotionWithTimeCodeDefaultTypeInternal _EmotionWithTimeCode_default_instance_;
class EmotionWithTimeCode_EmotionEntry_DoNotUse;
struct EmotionWithTimeCode_EmotionEntry_DoNotUseDefaultTypeInternal;
extern EmotionWithTimeCode_EmotionEntry_DoNotUseDefaultTypeInternal _EmotionWithTimeCode_EmotionEntry_DoNotUse_default_instance_;
}  // namespace v1
}  // namespace emotion_with_timecode
}  // namespace nvidia_ace
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace nvidia_ace {
namespace emotion_with_timecode {
namespace v1 {

// ===================================================================


// -------------------------------------------------------------------

class EmotionWithTimeCode_EmotionEntry_DoNotUse final
    : public ::google::protobuf::internal::MapEntry<
          EmotionWithTimeCode_EmotionEntry_DoNotUse, std::string, float,
          ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
          ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT> {
 public:
  using SuperType = ::google::protobuf::internal::MapEntry<
      EmotionWithTimeCode_EmotionEntry_DoNotUse, std::string, float,
      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
      ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>;
  EmotionWithTimeCode_EmotionEntry_DoNotUse();
  template <typename = void>
  explicit PROTOBUF_CONSTEXPR EmotionWithTimeCode_EmotionEntry_DoNotUse(
      ::google::protobuf::internal::ConstantInitialized);
  explicit EmotionWithTimeCode_EmotionEntry_DoNotUse(::google::protobuf::Arena* arena);
  static const EmotionWithTimeCode_EmotionEntry_DoNotUse* internal_default_instance() {
    return reinterpret_cast<const EmotionWithTimeCode_EmotionEntry_DoNotUse*>(
        &_EmotionWithTimeCode_EmotionEntry_DoNotUse_default_instance_);
  }
  static bool ValidateKey(std::string* s) {
    return ::google::protobuf::internal::WireFormatLite::VerifyUtf8String(s->data(), static_cast<int>(s->size()), ::google::protobuf::internal::WireFormatLite::PARSE, "nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode.EmotionEntry.key");
 }
  static bool ValidateValue(void*) { return true; }
  ::google::protobuf::Metadata GetMetadata() const final;
  friend struct ::TableStruct_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto;
};
// -------------------------------------------------------------------

class EmotionWithTimeCode final : public ::google::protobuf::Message
/* @@protoc_insertion_point(class_definition:nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode) */ {
 public:
  inline EmotionWithTimeCode() : EmotionWithTimeCode(nullptr) {}
  ~EmotionWithTimeCode() override;
  template <typename = void>
  explicit PROTOBUF_CONSTEXPR EmotionWithTimeCode(
      ::google::protobuf::internal::ConstantInitialized);

  inline EmotionWithTimeCode(const EmotionWithTimeCode& from) : EmotionWithTimeCode(nullptr, from) {}
  inline EmotionWithTimeCode(EmotionWithTimeCode&& from) noexcept
      : EmotionWithTimeCode(nullptr, std::move(from)) {}
  inline EmotionWithTimeCode& operator=(const EmotionWithTimeCode& from) {
    CopyFrom(from);
    return *this;
  }
  inline EmotionWithTimeCode& operator=(EmotionWithTimeCode&& from) noexcept {
    if (this == &from) return *this;
    if (GetArena() == from.GetArena()
#ifdef PROTOBUF_FORCE_COPY_IN_MOVE
        && GetArena() != nullptr
#endif  // !PROTOBUF_FORCE_COPY_IN_MOVE
    ) {
      InternalSwap(&from);
    } else {
      CopyFrom(from);
    }
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.unknown_fields<::google::protobuf::UnknownFieldSet>(::google::protobuf::UnknownFieldSet::default_instance);
  }
  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields()
      ABSL_ATTRIBUTE_LIFETIME_BOUND {
    return _internal_metadata_.mutable_unknown_fields<::google::protobuf::UnknownFieldSet>();
  }

  static const ::google::protobuf::Descriptor* descriptor() {
    return GetDescriptor();
  }
  static const ::google::protobuf::Descriptor* GetDescriptor() {
    return default_instance().GetMetadata().descriptor;
  }
  static const ::google::protobuf::Reflection* GetReflection() {
    return default_instance().GetMetadata().reflection;
  }
  static const EmotionWithTimeCode& default_instance() {
    return *internal_default_instance();
  }
  static inline const EmotionWithTimeCode* internal_default_instance() {
    return reinterpret_cast<const EmotionWithTimeCode*>(
        &_EmotionWithTimeCode_default_instance_);
  }
  static constexpr int kIndexInFileMessages = 1;
  friend void swap(EmotionWithTimeCode& a, EmotionWithTimeCode& b) { a.Swap(&b); }
  inline void Swap(EmotionWithTimeCode* other) {
    if (other == this) return;
#ifdef PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() != nullptr && GetArena() == other->GetArena()) {
#else   // PROTOBUF_FORCE_COPY_IN_SWAP
    if (GetArena() == other->GetArena()) {
#endif  // !PROTOBUF_FORCE_COPY_IN_SWAP
      InternalSwap(other);
    } else {
      ::google::protobuf::internal::GenericSwap(this, other);
    }
  }
  void UnsafeArenaSwap(EmotionWithTimeCode* other) {
    if (other == this) return;
    ABSL_DCHECK(GetArena() == other->GetArena());
    InternalSwap(other);
  }

  // implements Message ----------------------------------------------

  EmotionWithTimeCode* New(::google::protobuf::Arena* arena = nullptr) const final {
    return ::google::protobuf::Message::DefaultConstruct<EmotionWithTimeCode>(arena);
  }
  using ::google::protobuf::Message::CopyFrom;
  void CopyFrom(const EmotionWithTimeCode& from);
  using ::google::protobuf::Message::MergeFrom;
  void MergeFrom(const EmotionWithTimeCode& from) { EmotionWithTimeCode::MergeImpl(*this, from); }

  private:
  static void MergeImpl(
      ::google::protobuf::MessageLite& to_msg,
      const ::google::protobuf::MessageLite& from_msg);

  public:
  ABSL_ATTRIBUTE_REINITIALIZES void Clear() final;
  bool IsInitialized() const final;

  ::size_t ByteSizeLong() const final;
  const char* _InternalParse(const char* ptr, ::google::protobuf::internal::ParseContext* ctx) final;
  ::uint8_t* _InternalSerialize(
      ::uint8_t* target,
      ::google::protobuf::io::EpsCopyOutputStream* stream) const final;
  int GetCachedSize() const { return _impl_._cached_size_.Get(); }

  private:
  void SharedCtor(::google::protobuf::Arena* arena);
  void SharedDtor();
  void InternalSwap(EmotionWithTimeCode* other);
 private:
  friend class ::google::protobuf::internal::AnyMetadata;
  static ::absl::string_view FullMessageName() { return "nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode"; }

 protected:
  explicit EmotionWithTimeCode(::google::protobuf::Arena* arena);
  EmotionWithTimeCode(::google::protobuf::Arena* arena, const EmotionWithTimeCode& from);
  EmotionWithTimeCode(::google::protobuf::Arena* arena, EmotionWithTimeCode&& from) noexcept
      : EmotionWithTimeCode(arena) {
    *this = ::std::move(from);
  }
  const ::google::protobuf::MessageLite::ClassData* GetClassData()
      const final;

 public:
  ::google::protobuf::Metadata GetMetadata() const final;
  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------
  enum : int {
    kEmotionFieldNumber = 2,
    kTimeCodeFieldNumber = 1,
  };
  // map<string, float> emotion = 2;
  int emotion_size() const;
  private:
  int _internal_emotion_size() const;

  public:
  void clear_emotion() ;
  const ::google::protobuf::Map<std::string, float>& emotion() const;
  ::google::protobuf::Map<std::string, float>* mutable_emotion();

  private:
  const ::google::protobuf::Map<std::string, float>& _internal_emotion() const;
  ::google::protobuf::Map<std::string, float>* _internal_mutable_emotion();

  public:
  // double time_code = 1;
  void clear_time_code() ;
  double time_code() const;
  void set_time_code(double value);

  private:
  double _internal_time_code() const;
  void _internal_set_time_code(double value);

  public:
  // @@protoc_insertion_point(class_scope:nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode)
 private:
  class _Internal;
  friend class ::google::protobuf::internal::TcParser;
  static const ::google::protobuf::internal::TcParseTable<
      0, 2, 1,
      71, 2>
      _table_;
  friend class ::google::protobuf::MessageLite;
  friend class ::google::protobuf::Arena;
  template <typename T>
  friend class ::google::protobuf::Arena::InternalHelper;
  using InternalArenaConstructable_ = void;
  using DestructorSkippable_ = void;
  struct Impl_ {
    inline explicit constexpr Impl_(
        ::google::protobuf::internal::ConstantInitialized) noexcept;
    inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                          ::google::protobuf::Arena* arena);
    inline explicit Impl_(::google::protobuf::internal::InternalVisibility visibility,
                          ::google::protobuf::Arena* arena, const Impl_& from);
    ::google::protobuf::internal::MapField<EmotionWithTimeCode_EmotionEntry_DoNotUse, std::string, float,
                      ::google::protobuf::internal::WireFormatLite::TYPE_STRING,
                      ::google::protobuf::internal::WireFormatLite::TYPE_FLOAT>
        emotion_;
    double time_code_;
    mutable ::google::protobuf::internal::CachedSize _cached_size_;
    PROTOBUF_TSAN_DECLARE_MEMBER
  };
  union { Impl_ _impl_; };
  friend struct ::TableStruct_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto;
};

// ===================================================================




// ===================================================================


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
// -------------------------------------------------------------------

// -------------------------------------------------------------------

// EmotionWithTimeCode

// double time_code = 1;
inline void EmotionWithTimeCode::clear_time_code() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.time_code_ = 0;
}
inline double EmotionWithTimeCode::time_code() const {
  // @@protoc_insertion_point(field_get:nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode.time_code)
  return _internal_time_code();
}
inline void EmotionWithTimeCode::set_time_code(double value) {
  _internal_set_time_code(value);
  // @@protoc_insertion_point(field_set:nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode.time_code)
}
inline double EmotionWithTimeCode::_internal_time_code() const {
  PROTOBUF_TSAN_READ(&_impl_._tsan_detect_race);
  return _impl_.time_code_;
}
inline void EmotionWithTimeCode::_internal_set_time_code(double value) {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.time_code_ = value;
}

// map<string, float> emotion = 2;
inline int EmotionWithTimeCode::_internal_emotion_size() const {
  return _internal_emotion().size();
}
inline int EmotionWithTimeCode::emotion_size() const {
  return _internal_emotion_size();
}
inline void EmotionWithTimeCode::clear_emotion() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  _impl_.emotion_.Clear();
}
inline const ::google::protobuf::Map<std::string, float>& EmotionWithTimeCode::_internal_emotion() const {
  PROTOBUF_TSAN_READ(&_impl_._tsan_detect_race);
  return _impl_.emotion_.GetMap();
}
inline const ::google::protobuf::Map<std::string, float>& EmotionWithTimeCode::emotion() const ABSL_ATTRIBUTE_LIFETIME_BOUND {
  // @@protoc_insertion_point(field_map:nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode.emotion)
  return _internal_emotion();
}
inline ::google::protobuf::Map<std::string, float>* EmotionWithTimeCode::_internal_mutable_emotion() {
  PROTOBUF_TSAN_WRITE(&_impl_._tsan_detect_race);
  return _impl_.emotion_.MutableMap();
}
inline ::google::protobuf::Map<std::string, float>* EmotionWithTimeCode::mutable_emotion() ABSL_ATTRIBUTE_LIFETIME_BOUND {
  // @@protoc_insertion_point(field_mutable_map:nvidia_ace.emotion_with_timecode.v1.EmotionWithTimeCode.emotion)
  return _internal_mutable_emotion();
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)
}  // namespace v1
}  // namespace emotion_with_timecode
}  // namespace nvidia_ace


// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_nvidia_5face_2eemotion_5fwith_5ftimecode_2ev1_2eproto_2epb_2eh
