#include <stdint.h>
#include <utility>

namespace helpers
{
    // Types 
    struct carry_result
    {
        uint8_t sum;
        bool    carry;
    };

    // Functions

    // Compile-time helper to ensure Operations that are passed to 
    // any one of the addressing modes contain a call operator.
    template<typename Operation>
    constexpr void assert_contains_call_operator()
    {
        // NOTE: This is not currently what we want.
        //static_assert(std::is_function<Operation>::value, "Operation does not contain call operator!");
    }

    constexpr int8_t convert_from_twos_complement(const uint8_t twos_comp_value)
    {
        if((twos_comp_value & 0x80) == 0x1) // Check sign-bit to see if value is negative.
        {
            // Check if signed
            constexpr std::size_t NUM_BITS = 8U;
            const uint8_t mask             = 2^(NUM_BITS - 1U);
            return -(twos_comp_value & mask) + (twos_comp_value & ~mask);
        }
        else
        {
            return twos_comp_value;
        }
    }

    /// Function takes a signed value and returns an unsigned byte 
    constexpr uint8_t convert_to_twos_complement(const uint8_t signed_value)
    {
        // Unsure of this - refer to the following doc: 
        // https://stackoverflow.com/questions/25754082/how-to-take-twos-complement-of-a-byte-in-c
        return -(unsigned int)signed_value;
    }

    // Helper function that will 
    carry_result sum_with_carry(const uint8_t val_a, const uint8_t val_b)
    {
        uint16_t sum = val_a + val_b;
        return {std::move(static_cast<uint8_t>(sum)), sum > 0xFF};
    }

    // Helper function to compute a wrap-around address
    constexpr uint8_t sum_with_wrap_around(const uint8_t val_a,
                                                  const uint8_t val_b,
                                                  const uint8_t wrap_around_val)
    {
        // NOTE: When we wrap-around, do we end up setting the overflow flag?
        return (val_a + val_b) % wrap_around_val;
    }

    // Helper function used to combine two bytes into a single 16-bit representation.
    constexpr uint16_t create_two_byte_address(const uint8_t MSB, const uint8_t LSB)
    {
        return (static_cast<uint8_t>(MSB) << 8U) | LSB;
    }
} // namespace helpers
