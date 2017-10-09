namespace detail {
  enum class enabler {};
}
// This works around a clang issue
// http://flamingdangerzone.com/cxx11/2012/06/01/almost-static-if.html
constexpr detail::enabler Enable = {};

template<class First, class ... T>
struct all
  : std::conditional<
      First::value,
      typename all<T...>::type,
      std::false_type
    >::type
{
};

template<class First>
struct all<First>
  : std::conditional<
      First::value,
      std::true_type,
      std::false_type
    >::type
{
};

template <typename ... Condition>
using When = typename std::enable_if<all<Condition...>::value, detail::enabler>::type;

template <typename Ret, typename ... Condition>
using Enable_When = typename std::enable_if<all<Condition...>::value, Ret>::type;

template<class T>
using Not = typename std::conditional<!T::value, std::true_type, std::false_type>::type;

template <typename ... Condition>
using When_Not = typename std::enable_if<all<Not<Condition>...>::value, detail::enabler>::type;

template<class T>
using Is = typename std::conditional<T::value, std::true_type, std::false_type>::type;
