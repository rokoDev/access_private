#include <type_traits>
#include <utility>

#if __cplusplus == 201103L
namespace std {
  template <bool B, class T = void>
  using enable_if_t = typename enable_if<B, T>::type;
  template <class T>
  using remove_reference_t = typename remove_reference<T>::type;
} // namespace std
#endif

// Unnamed namespace is used to avoid duplicate symbols if the macros are used
// in several translation units. See test1.
namespace {
  namespace private_access_detail {

    // @tparam TagType, used to declare different "get" funciton overloads for
    // different members/statics
    template <typename PtrType, PtrType PtrValue, typename TagType>
    struct private_access {
      // Normal lookup cannot find in-class defined (inline) friend functions.
      constexpr friend PtrType get(TagType) { return PtrValue; }
    };

  } // namespace private_access_detail
} // namespace

// Used macro naming conventions:
// The "namespace" of this macro library is PRIVATE_ACCESS, i.e. all
// macro here has this prefix.
// All implementation macro, which are not meant to be used directly have the
// PRIVATE_ACCESS_DETAIL prefix.
// Some macros have the ABCD_IMPL form, which means they contain the
// implementation details for the specific ABCD macro.

#if defined(_MSC_VER) && !defined(__clang__)
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_PUSH __pragma(warning(push))
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_POP __pragma(warning(pop))
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING(warningNumber)                   \
  __pragma(warning(disable : warningNumber))

#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    \
  PRIVATE_ACCESS_DETAIL_DISABLE_WARNING(4100)
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FUNCTION            \
  PRIVATE_ACCESS_DETAIL_DISABLE_WARNING(4505)
  // other warnings you want to deactivate...

#elif defined(__GNUC__) || defined(__clang__)
#define PRIVATE_ACCESS_DETAIL_DO_PRAGMA(X) _Pragma(#X)
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_PUSH                             \
  PRIVATE_ACCESS_DETAIL_DO_PRAGMA(GCC diagnostic push)
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_POP                              \
  PRIVATE_ACCESS_DETAIL_DO_PRAGMA(GCC diagnostic pop)
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING(warningName)                     \
  PRIVATE_ACCESS_DETAIL_DO_PRAGMA(GCC diagnostic ignored warningName)

#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER    \
  PRIVATE_ACCESS_DETAIL_DISABLE_WARNING("-Wunused-parameter")
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FUNCTION            \
  PRIVATE_ACCESS_DETAIL_DISABLE_WARNING("-Wunused-function")
  // other warnings you want to deactivate...

#else
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_PUSH
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_POP
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#define PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FUNCTION
  // other warnings you want to deactivate...

#endif

#define PRIVATE_ACCESS_DETAIL_CONCATENATE_IMPL(x, y) x##y
#define PRIVATE_ACCESS_DETAIL_CONCATENATE(x, y)                                \
  PRIVATE_ACCESS_DETAIL_CONCATENATE_IMPL(x, y)

// @param PtrTypeKind E.g if we have "class A", then it can be "A::*" in case of
// members, or it can be "*" in case of statics.
#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name,           \
                                             PtrTypeKind)                      \
  namespace {                                                                  \
    namespace private_access_detail {                                          \
      /* Tag type, used to declare different get funcitons for different       \
       * members                                                               \
       */                                                                      \
      struct Tag {};                                                           \
      /* We can build the PtrType only with two aliases */                     \
      /* E.g. using PtrType = int(int) *; would be illformed */                \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(Alias_, Tag) = Type;             \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(PtrType_, Tag) =                 \
          PRIVATE_ACCESS_DETAIL_CONCATENATE(Alias_, Tag) PtrTypeKind;          \
      /* Explicit instantiation */                                             \
      template struct private_access<PRIVATE_ACCESS_DETAIL_CONCATENATE(        \
                                         Alias_, Tag)(PtrTypeKind),            \
                                     &Class::Name, Tag>;                       \
      /* Declare the friend function, now it is visible in namespace scope.    \
       * Note,                                                                 \
       * we could declare it inside the Tag type too, in that case ADL would   \
       * find                                                                  \
       * the declaration. By choosing to declare it here, the Tag type remains \
       * a                                                                     \
       * simple tag type, it has no other responsibilities. */                 \
      constexpr PRIVATE_ACCESS_DETAIL_CONCATENATE(PtrType_, Tag) get(Tag);     \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FIELD(Tag, Class, Type, Name)     \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)       \
  namespace {                                                                  \
    namespace access_private {                                                 \
      PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_PUSH                               \
      PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_UNREFERENCED_FUNCTION              \
      constexpr Type &Name(Class &&t) {                                        \
        return t.*get(private_access_detail::Tag{});                           \
      }                                                                        \
      constexpr Type &Name(Class &t) {                                         \
        return t.*get(private_access_detail::Tag{});                           \
      }                                                                        \
      /* The following usings are here to avoid duplicate const qualifier      \
       * warnings                                                              \
       */                                                                      \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(X, Tag) = Type;                  \
      using PRIVATE_ACCESS_DETAIL_CONCATENATE(Y, Tag) =                        \
          const PRIVATE_ACCESS_DETAIL_CONCATENATE(X, Tag);                     \
      constexpr PRIVATE_ACCESS_DETAIL_CONCATENATE(Y, Tag) &                    \
          Name(const Class &t) {                                               \
        return t.*get(private_access_detail::Tag{});                           \
      }                                                                        \
      PRIVATE_ACCESS_DETAIL_DISABLE_WARNING_POP                                \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FUN(Tag, Class, Type, Name)       \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, Class::*)       \
  namespace {                                                                  \
    namespace call_private {                                                   \
      /* We do perfect forwarding, but we want to restrict the overload set    \
       * only for objects which have the type Class. */                        \
      template <typename Obj,                                                  \
                std::enable_if_t<std::is_same<std::remove_reference_t<Obj>,    \
                                              Class>::value> * = nullptr,      \
                typename... Args>                                              \
      constexpr auto Name(Obj &&o, Args &&...args) -> decltype((               \
          static_cast<Obj &&>(o).*                                             \
          get(private_access_detail::Tag{}))(static_cast<Args &&>(args)...)) { \
        return (static_cast<Obj &&>(o).*get(private_access_detail::Tag{}))(    \
            static_cast<Args &&>(args)...);                                    \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FIELD(Tag, Class, Type,    \
                                                          Name)                \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, *)              \
  namespace {                                                                  \
    namespace access_private_static {                                          \
      namespace Class {                                                        \
        constexpr Type &Name() { return *get(private_access_detail::Tag{}); }  \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FUN(Tag, Class, Type,      \
                                                        Name)                  \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE(Tag, Class, Type, Name, *)              \
  namespace {                                                                  \
    namespace call_private_static {                                            \
      namespace Class {                                                        \
        template <typename... Args>                                            \
        constexpr auto Name(Args &&...args) -> decltype(get(                   \
            private_access_detail::Tag{})(static_cast<Args &&>(args)...)) {    \
          return get(private_access_detail::Tag{})(                            \
              static_cast<Args &&>(args)...);                                  \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }

#define PRIVATE_ACCESS_DETAIL_UNIQUE_TAG                                       \
  PRIVATE_ACCESS_DETAIL_CONCATENATE(PrivateAccessTag, __COUNTER__)

#define ACCESS_PRIVATE_FIELD(Class, Type, Name)                                \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FIELD(PRIVATE_ACCESS_DETAIL_UNIQUE_TAG, \
                                             Class, Type, Name)

#define ACCESS_PRIVATE_FUN(Class, Type, Name)                                  \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_FUN(PRIVATE_ACCESS_DETAIL_UNIQUE_TAG,   \
                                           Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FIELD(Class, Type, Name)                         \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FIELD(                           \
      PRIVATE_ACCESS_DETAIL_UNIQUE_TAG, Class, Type, Name)

#define ACCESS_PRIVATE_STATIC_FUN(Class, Type, Name)                           \
  PRIVATE_ACCESS_DETAIL_ACCESS_PRIVATE_STATIC_FUN(                             \
      PRIVATE_ACCESS_DETAIL_UNIQUE_TAG, Class, Type, Name)
