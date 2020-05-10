#include <type_traits>
#include <utility>

namespace utils
{
    // Compile-time helper to ensure Operations that are passed to 
    // any one of the addressing modes contain a call operator.
    template<typename Operation>
    constexpr void assert_contains_call_operator()
    {
        // NOTE: This is not currently what we want.
        static_assert(std::is_function<Operation>::value, "Operation does not contain call operator!");
    }

} // namespace utils
