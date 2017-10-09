#pragma once
#include <type_traits>
#include <utility>
#include <algorithm> 
#include "enable_if.hpp"

namespace std {
  struct nullopt_t {};
  struct in_place_t {
    explicit in_place_t() = default;
  };
  class bad_optional_access : public exception
  {
    public:
    virtual const char* what() const noexcept
    {
      return "Value not set";
    }
  };

  //inline constexpr std::in_place_t in_place{};
  //template <class T> struct in_place_type_t {
  //  explicit in_place_type_t() = default;
  //};
  //template <class T>
  //inline constexpr std::in_place_type_t<T> in_place_type{};
  //template <size_t I> struct in_place_index_t {
  //  explicit in_place_index_t() = default;
  //};
  //template <size_t I>
  //inline constexpr in_place_index_t<I> in_place_index{};
}

namespace detail {
  template<class T, bool>
  class optional;


  template<class T, bool>
  union storage_;

  template<class T>
  union storage_<T, true> 
  {
    constexpr storage_() : x() {}
    
    constexpr storage_(T const & t) : t_(t) {}
    constexpr storage_(T && t) : t_(std::move(t)) {}

    constexpr T & value() & {
      return t_;
    }
    constexpr T const & value() const & {
      return t_;
    }
    
    T t_;
    char x;
  };
  
  template<class T>
  union storage_<T, false>
  {
    constexpr storage_() : x() {}
    constexpr storage_(T const & t) : t_(t) {}
    constexpr storage_(T && t) : t_(std::move(t)) {}

    constexpr T & value() & {
      return t_;
    }

    constexpr T const & value() const & {
      return t_;
    }
    
    ~storage_() { }
    T t_;
    char x;
  };

  template<class T>
  using storage = storage_<T, std::is_trivially_destructible<T>::value>;
}

namespace std {
  template<class T>
  using optional = detail::optional<T, std::is_trivially_destructible<T>::value>;
}

namespace detail {
  template<class T>
  Enable_When<void, std::is_trivially_destructible<T>> destruct(T &) {} 
  template<class T>
  Enable_When<void, Not<std::is_trivially_destructible<T>>> destruct(T & t) { t.~T(); } 

  template<class T>
  class optional<T, true>
  {
    public:
    using value_type = T;
    
    ///Constructor
    //http://en.cppreference.com/w/cpp/utility/optional/optional
    constexpr optional() noexcept
      : initalized_(false)
    {
    }

    constexpr optional( std::nullopt_t ) noexcept
      : optional()
    {
    }

    /*! 2) Copy constructor: 
       If other contains a value, 
       initializes the contained value as if direct-initializing 
       (but not direct-list-initializing) 
       an object of type T with the expression *other. 
       If other does not contain a value, 
       constructs an object that does not contain a value. 
       This constructor is defined as deleted if 
       std::is_copy_constructible_v<T> is false. 
       It is a constexpr constructor if std::is_trivially_copy_constructible_v<T> is true.
    */
    template<class U = T,
       When<
         Is<std::is_copy_constructible<U>>
       > = Enable
    >
    constexpr optional(
       const optional & other
    );
    

    /*! 3) Move constructor: 
        If other contains a value, 
        initializes the contained value as if direct-initializing 
        (but not direct-list-initializing) 
        an object of type T with the expression std::move(*other) 
        and does not make other empty: 
        a moved-from optional still contains a value, 
        but the value itself is moved from. 
        If other does not contain a value, constructs an object that does 
        not contain a value. 
        This constructor does not participate in overload resolution unless 
        std::is_move_constructible_v<T> is true. 
        It is a constexpr constructor if std::is_trivially_move_constructible_v<T> is true.
        3) Throws any exception thrown by the constructor of T. 
          Has the following noexcept specification:  
          noexcept(std::is_nothrow_move_constructible<T>::value)
    */
    template<class U=T,
      When<
        Is<std::is_move_constructible<U>>
      > = Enable
    >
    constexpr optional( optional&& other ) noexcept(std::is_nothrow_move_constructible<T>::value);

    /*! Converting copy constructor: 
      If other doesn't contain a value, constructs an optional object that 
      does not contain a value. Otherwise, constructs an optional object that 
      contains a value, initialized as if direct-initializing 
      (but not direct-list-initializing) an object 
      of type T with the expression *other. 
      
      This constructor does not participate in overload resolution unless the following conditions are met: 
      std::is_constructible_v<T, const U&> is true.
      T is not constructible or convertible from any expression of 
      type (possibly const) std::optional<U>, 
      i.e., the following 8 type traits are all false:
      
      std::is_constructible_v<T, std::optional<U>&>
      std::is_constructible_v<T, const std::optional<U>&>
      std::is_constructible_v<T, std::optional<U>&&>
      std::is_constructible_v<T, const std::optional<U>&&>
      std::is_convertible_v<std::optional<U>&, T>
      std::is_convertible_v<const std::optional<U>&, T>
      std::is_convertible_v<std::optional<U>&&, T>
      std::is_convertible_v<const std::optional<U>&&, T>

      This constructor is explicit if and only if std::is_convertible_v<const U&, T> is false.
    */
    template < class U,
      When<
        std::is_constructible<T, const U&>,
        std::is_constructible<T, std::optional<U>&>,
        std::is_constructible<T, const std::optional<U>&>,
        std::is_constructible<T, std::optional<U>&&>,
        std::is_constructible<T, const std::optional<U>&&>,
        std::is_convertible<std::optional<U>&, T>,
        std::is_convertible<const std::optional<U>&, T>,
        std::is_convertible<std::optional<U>&&, T>,
        std::is_convertible<const std::optional<U>&&, T>,
        Not<std::is_convertible<const U&, T>>
      > = Enable
    >
    explicit optional( const std::optional<U>& other );
    
    template < class U,
      When<
        std::is_constructible<T, const U&>,
        std::is_constructible<T, std::optional<U>&>,
        std::is_constructible<T, const std::optional<U>&>,
        std::is_constructible<T, std::optional<U>&&>,
        std::is_constructible<T, const std::optional<U>&&>,
        std::is_convertible<std::optional<U>&, T>,
        std::is_convertible<const std::optional<U>&, T>,
        std::is_convertible<std::optional<U>&&, T>,
        std::is_convertible<const std::optional<U>&&, T>,
        std::is_convertible<const U&, T>
      > = Enable
    >
    optional( const std::optional<U>& other );

    /*! 5) Converting move constructor: 
      If other doesn't contain a value, constructs an optional object that 
      does not contain a value. Otherwise, constructs an optional object that 
      contains a value, initialized as if direct-initializing 
      (but not direct-list-initializing) an object of type T with the 
      expression std::move(*other). 
      This constructor does not participate in overload resolution unless the following conditions are met: 
      std::is_constructible_v<T, U&&> is true.
      T is not constructible or convertible from any expression of type (possibly const) std::optional<U>, i.e., the following 8 type traits are all false:
      std::is_constructible_v<T, std::optional<U>&>
      std::is_constructible_v<T, const std::optional<U>&>
      std::is_constructible_v<T, std::optional<U>&&>
      std::is_constructible_v<T, const std::optional<U>&&>
      std::is_convertible_v<std::optional<U>&, T>
      std::is_convertible_v<const std::optional<U>&, T>
      std::is_convertible_v<std::optional<U>&&, T>
      std::is_convertible_v<const std::optional<U>&&, T>
   This constructor is explicit if and only if std::is_convertible_v<U&&, T> is false.*/

    template < class U,
    When<
      std::is_constructible<T, U&&>,
      Not<std::is_constructible<T, std::optional<U>&>>,
      Not<std::is_constructible<T, const std::optional<U>&>>,
      Not<std::is_constructible<T, std::optional<U>&&>>,
      Not<std::is_constructible<T, const std::optional<U>&&>>,
      Not<std::is_convertible<std::optional<U>&, T>>,
      Not<std::is_convertible<const std::optional<U>&, T>>,
      Not<std::is_convertible<std::optional<U>&&, T>>,
      Not<std::is_convertible<const std::optional<U>&&, T>>,
      Not<std::is_convertible<U&&, T>>
    > = Enable>
    explicit optional( std::optional<U>&& other );

    /*! 6) 
    Constructs an optional object that contains a value, 
    initialized as if direct-initializing (but not direct-list-initializing) 
    an object of type T from the arguments std::forward<Args>(args).... 
    If the selected constructor of T is a constexpr constructor, 
    this constructor is a constexpr constructor. 
    The function does not participate in the overload resolution unless 
    std::is_constructible_v<T, Args...> is true*/
    //template< class... Args > 
    //constexpr explicit optional( std::in_place_t, Args&&... args );
   
    /*!
      8) Constructs an optional object that contains a value, 
      initialized as if direct-initializing (but not direct-list-initializing) 
      an object of type T (where T = value_type) with the expression 
      std::forward<U>(value). 
      If the selected constructor of T is a constexpr constructor, 
      this constructor is a constexpr constructor. 
      This constructor does not participate in overload resolution unless 
      std::is_constructible_v<T, U&&> is true and 
      std::decay_t<U> is neither std::in_place_t nor std::optional<T>. 
      This constructor is explicit if and only if std::is_convertible_v<U&&, T> is false.
    */
    //template< class U, class... Args >
    //constexpr explicit optional( std::in_place_t,
    //                         std::initializer_list<U> ilist, 
    //                         Args&&... args );

    /*!Constructs an optional object that contains a value, initialized as if 
       direct-initializing (but not direct-list-initializing) an object of type 
       T (where T = value_type) with the expression std::forward<U>(value). 
       If the selected constructor of T is a constexpr constructor, 
       this constructor is a constexpr constructor. 
       This constructor does not participate in overload resolution unless 
       std::is_constructible_v<T, U&&> is true and 
       std::decay_t<U> is neither std::in_place_t nor std::optional<T>. 
       This constructor is explicit if and only if std::is_convertible_v<U&&, T> is false.*/
    template < class U = value_type, 
      When<
        Is<std::is_constructible<T, U&&>>,
        Not<std::is_same<std::decay_t<U>, std::in_place_t>>,
        Not<std::is_same<std::decay_t<U>, std::optional<T>>>,
        Not<std::is_constructible<T, U&&>>
      > = Enable
    >
    explicit constexpr optional( U && value )
      : initalized_(true)
      , value_(std::forward<U>(value))
    {
    }
    
    template < class U = value_type, 
      When<
        Is<std::is_constructible<T, U&&>>,
        Not<std::is_same<std::decay_t<U>, std::in_place_t>>,
        Not<std::is_same<std::decay_t<U>, std::optional<T>>>,
        Is<std::is_constructible<T, U&&>>
      > = Enable
    >
    optional( U && value )
      : initalized_(true)
      , value_(std::forward<U>(value))
    {
    }
      

   ///Observers
   //http://en.cppreference.com/w/cpp/utility/optional/operator*
#if 0
   constexpr const T* operator->() const
   {
   }

   constexpr T* operator->()
   {
   }
#endif

   constexpr const T& operator*() const&
   {
     return value_.value();
   }

   constexpr T& operator*() &
   {
     return value_.value();
   }

#if 0
   constexpr const T&& operator*() const&&
   {
   }

   constexpr T&& operator*() &&
   {
   }
#endif

  //http://en.cppreference.com/w/cpp/utility/optional/operator_bool
  constexpr explicit operator bool() const noexcept
  {
    return has_value();
  }

  constexpr bool has_value() const noexcept
  {
    return initalized_;
  }

  //http://en.cppreference.com/w/cpp/utility/optional/value
  constexpr T & value() & 
  {
    check();
    return value_.value();
  }

  constexpr const T & value() const &
  {
    check();
    return value_.value();
  }

  constexpr T&& value() &&
  {
    check();
    return std::move(value_.value());
  }

  constexpr const T&& value() const &&
  {
    check();
    return std::move(value_.value());
  }

#if 0
  //http://en.cppreference.com/w/cpp/utility/optional/value_or
  template< class U > 
  constexpr T value_or( U&& default_value ) const&;
  template< class U > 
  constexpr T value_or( U&& default_value ) &&;
#endif

  ///Modifiers
  //http://en.cppreference.com/w/cpp/utility/optional/swap

  /*!Swaps the contents with those of other.
     If neither *this nor other contain a value, the function has no effect.
     
     If only one of *this and other contains a value 
     (let's call this object in and the other un), 
     the contained value of un is direct-initialized from std::move(*in), 
     followed by destruction of the contained value of in as if by in->T::~T(). 
     After this call, in does not contain a value; un contains a value.
 
     If both *this and other contain values, 
     the contained values are exchanged by calling 
     using std::swap; 
     swap(**this, *other). 
     T lvalues must satisfy Swappable.
  */
  void swap( optional& rhs) 
    noexcept(
      std::is_nothrow_move_constructible<T>::value && 
      //std::is_nothrow_swappable<T>::value
      std::__is_nothrow_swappable<T>::value
    )
  {
    if (!has_value() && !rhs.has_value())
    {
      return;
    }
    else if (has_value() && rhs.has_value())
    {
      using std::swap;
      swap(**this, *rhs);
    }
    else if (rhs.has_value())
    {
      **this = std::move(*rhs);
      initalized_ = true;
      destruct(rhs.value());
      rhs.initalized_ = false;
    }
    else
    {
      *rhs = std::move(**this);
      rhs.initalized_ = true;
      destruct(value());
      initalized_ = false;
    }
  }

  //http://en.cppreference.com/w/cpp/utility/optional/reset
  void reset() noexcept
  {
    if (has_value())
    {
      destruct(value());
      initalized_ = false;
    }
  }

#if 0
  //http://en.cppreference.com/w/cpp/utility/optional/emplace
  template< class... Args > 
  T& emplace( Args&&... args );
  template< class U, class... Args > 
  T& emplace( std::initializer_list<U> ilist, Args&&... args );
#endif
    private:
    constexpr void check() const {
      if (!has_value())
      {
        throw std::bad_optional_access();
      }
    }

    bool initalized_;
    storage<T> value_;
  };

  template<class T>
  class optional<T, false> : public optional<T, true>
  {
    public:
    using optional<T, true>::optional;

    ~optional()
    {
    }
  };
}

namespace std {
  template<class T>
  using optional = detail::optional<T, std::is_trivially_destructible<T>::value>;
}
