#include <catch.hpp>
#include <optional.hpp>

struct Trival_Destructor
{
};

struct Non_Trival_Destructor
{
  ~Non_Trival_Destructor() {}
};

struct Non_trival_constructor_constexpr {
  constexpr Non_trival_constructor_constexpr() {}
};

template<class T>
using optional = std::optional<T>;

TEST_CASE("type traits", "[optional]") {
  SECTION("trival distructor") {
    static_assert(std::is_trivially_destructible<Trival_Destructor>::value, "Trival_Destructor fail");
    static_assert(std::is_trivially_destructible<optional<Trival_Destructor>>::value, "optional Trival_Destructor fail");
  }

  SECTION("non trival destructor") {
    static_assert(!std::is_trivially_destructible<Non_Trival_Destructor>::value, "Non_Trival_Destructor fail");
    static_assert(!std::is_trivially_destructible<optional<Non_Trival_Destructor>>::value, "optional Non_Trival_Destructor fail");
  }
}
  
TEST_CASE("member types", "[optional]") {
  static_assert(std::is_same<int, optional<int>::value_type>::value, "member type incorrect");
}

TEST_CASE("constexpr", "[optional]") {
  SECTION("defalt constructor") {
    auto constexpr x = optional<Non_trival_constructor_constexpr>();
    static_assert(!x.has_value(), "does not have value");
    static_assert(!static_cast<bool>(x), "does not have value");
  }
  SECTION("nollopt constructor") {
    auto constexpr x = optional<Non_trival_constructor_constexpr>(std::nullopt_t());
    static_assert(!x.has_value(), "does not have value");
    static_assert(!static_cast<bool>(x), "does not have value");
  }
  //TODO: check explicit behavior of #8 constructor
  SECTION("value") {
    auto constexpr x = optional<int>{1};
    static_assert(x.has_value(), "does not have value");
    static_assert(static_cast<bool>(x), "does not have value");
    static_assert(x.value() == 1, "Value incorrect");
  }
  SECTION("rvalue value") {
    static_assert(std::is_same<int&&, decltype(optional<int>().value())>::value, "rvalue value");
  }
  SECTION("operator*") {
    //TODO: another way to make this non-const?
    constexpr optional<int> x{1};
    static_assert(*const_cast<optional<int>&>(x) == 1, "Value incorrect");
    static_assert(std::is_same<int&, decltype(*const_cast<optional<int>&>(x))>::value, "type");
  }
  SECTION("const operator*") {
    auto const constexpr x = optional<int>{1};
    static_assert(*x == 1, "Value incorrect");
    static_assert(std::is_same<int const &, decltype(*x)>::value, "type");
  }
}

struct Tracked
{
  Tracked() 
  : value()
  {
    ++constructed__;
  }

  Tracked(int x) 
    : value(x)
  {
  }

  Tracked(Tracked && x)
    : value(x.value)
  {
    x.value = 0;
    ++moved__;
  }
  
  Tracked & operator=(Tracked && x)
  {
    value = x.value;
    x.value = 0;
    ++moved__;
    return *this;
  }
  
  ~Tracked() {
    ++destructed__;
  }
  static ssize_t destructed__;
  static ssize_t constructed__;
  static ssize_t moved__;

  static void reset()
  {
    destructed__ = 0U;
    constructed__ = 0U;
    moved__ = 0U;
  }

  int value;
};
ssize_t Tracked::destructed__(0);
ssize_t Tracked::constructed__(0);
ssize_t Tracked::moved__(0);

TEST_CASE("runtime", "[optional]") {
  SECTION("reset no value") {
    Tracked::reset();
    std::optional<Tracked> t;
    REQUIRE(Tracked::destructed__ == 0U);
    REQUIRE(Tracked::constructed__ == 0U);
    REQUIRE(!t.has_value());
    t.reset();
    REQUIRE(Tracked::destructed__ == 0U);
    REQUIRE(Tracked::constructed__ == 0U);
    REQUIRE(!t.has_value());
  }
  SECTION("reset value") {
    std::optional<Tracked> t{Tracked()};
    REQUIRE(Tracked::destructed__ == 1U);
    REQUIRE(Tracked::constructed__ == 1U);
    REQUIRE(t.has_value());
    t.reset();
    REQUIRE(Tracked::destructed__ == 2U);
    REQUIRE(Tracked::constructed__ == 1U);
    REQUIRE(!t.has_value());
  }
  SECTION("no value throws const") {
    auto const x = optional<int>();
    REQUIRE_THROWS_AS(x.value(), std::bad_optional_access);
    static_assert(std::is_same<int const &, decltype(x.value())>::value, "type");
  }
  SECTION("no value throws ") {
    auto x = optional<int>();
    REQUIRE_THROWS_AS(x.value(), std::bad_optional_access);
    static_assert(std::is_same<int &, decltype(x.value())>::value, "type");
  }
  SECTION("no value throws rvalue") {
    REQUIRE_THROWS_AS(optional<int>().value(), std::bad_optional_access);
    static_assert(std::is_same<int &&, decltype(optional<int>().value())>::value, "type");
  }
  SECTION("no value throws const rvalue") {
    REQUIRE_THROWS_AS(static_cast<const optional<int>&&>(optional<int>()).value(), std::bad_optional_access);
    static_assert(std::is_same<int const &&, decltype(static_cast<optional<int> const &&>(optional<int>()).value())>::value, "type");
  }
}

TEST_CASE("swap", "[optional]") {
  SECTION("neither set")
  {
    Tracked::reset();
    optional<Tracked> v1;
    optional<Tracked> v2;
    v1.swap(v2);
    REQUIRE(!v1.has_value());
    REQUIRE(!v2.has_value());
    REQUIRE(Tracked::constructed__ == 0U);
    REQUIRE(Tracked::destructed__ == 0U);
  }
  SECTION("both set")
  {
    optional<Tracked> v1{Tracked(1)};
    optional<Tracked> v2{Tracked(2)};
    Tracked::reset();
    v1.swap(v2);
    REQUIRE(v1.has_value());
    REQUIRE(v2.has_value());
    REQUIRE(v2.value().value == 1U);
    REQUIRE(v1.value().value == 2U);
    REQUIRE(Tracked::constructed__ == 0U);
    REQUIRE(Tracked::destructed__ == 1U);
    REQUIRE(Tracked::moved__ == 3U);
  }
  SECTION("lhs set")
  {
    optional<Tracked> v1{Tracked(1)};
    optional<Tracked> v2;
    Tracked::reset();
    v1.swap(v2);
    REQUIRE(!v1.has_value() == true);
    REQUIRE(v2.has_value());
    REQUIRE(v2.value().value == 1U);
    REQUIRE(Tracked::constructed__ == 0U);
    REQUIRE(Tracked::destructed__ == 1U);
    REQUIRE(Tracked::moved__ == 1U);
  }
  SECTION("rhs set")
  {
    optional<Tracked> v2{Tracked(1)};
    optional<Tracked> v1;
    Tracked::reset();
    v1.swap(v2);
    REQUIRE(v1.has_value());
    REQUIRE(!v2.has_value());
    REQUIRE(v1.value().value == 1U);
    REQUIRE(Tracked::constructed__ == 0U);
    REQUIRE(Tracked::destructed__ == 1U);
    REQUIRE(Tracked::moved__ == 1U);
  }
}
