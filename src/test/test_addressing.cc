#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../cpu.hh"

namespace utils
{
    // No-Op operation for testing addressing.
    const auto noop = [](cpu::state &s, const uint8_t val){};
}

namespace constants
{
    // Hold constants necessary for unit-testing.

    constexpr uint16_t DEFAULT_PROGRAM_COUNTER = 0x1111;
    constexpr uint16_t DEFAULT_STACK_POINTER   = 0x2222;
    constexpr uint8_t  DEFAULT_REG_A           = 0x33;
    constexpr uint8_t  DEFAULT_REG_X           = 0x44;
    constexpr uint8_t  DEFAULT_REG_Y           = 0x55;

} // namespace constants

namespace
{

using flags = cpu::flags;
using state = cpu::state;

cpu::state create_mock_state()
{
    cpu::state state;
    
    state.pc    = constants::DEFAULT_PROGRAM_COUNTER;
    state.sp    = constants::DEFAULT_STACK_POINTER;
    state.reg_a = constants::DEFAULT_REG_A;
    state.reg_x = constants::DEFAULT_REG_X;
    state.reg_y = constants::DEFAULT_REG_Y;

    state.f = cpu::flags{};
    
    return state;
}

cpu::memory create_mock_memory(const uint8_t fill_value)
{ 
    cpu::memory memory;
    std::fill(std::begin(memory), std::end(memory), fill_value);

    return memory;
}

} // namespace

TEST_CASE("Addressing: Implicit", "[addressing]")
{ 
    // NOTE: No need for this addressing mode.
    REQUIRE(true);
}

TEST_CASE("Addressing: Immediate", "[addressing]")
{ 
    const auto memory = create_mock_memory(0x55);
    auto state        = create_mock_state();
   
    cpu::address::immediate(utils::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 2U);
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Zero-Page", "[addressing]")
{ 
    const auto memory = create_mock_memory(0xFF);
    auto state        = create_mock_state();
   
    cpu::address::zero_page(utils::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 2U);
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Zero-Page, X", "[addressing]")
{ 
    const auto memory = create_mock_memory(0xFF);
    auto state        = create_mock_state();
   
    cpu::address::zero_page_x(utils::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 2U);
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Zero-Page, Y", "[addressing]")
{
    const auto memory = create_mock_memory(0xFF);
    auto state        = create_mock_state();
   
    cpu::address::zero_page_y(utils::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 2U);
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Relative", "[addressing]")
{

}

TEST_CASE("Addressing: Absolute", "[addressing]")
{

}

TEST_CASE("Addressing: Absolute, X", "[addressing]")
{

}

TEST_CASE("Addressing: Absolute, Y", "[addressing]")
{

}

TEST_CASE("Addressing: Indirect", "[addressing]")
{

}

TEST_CASE("Addressing: Indexed Indirect", "[addressing]")
{
    
}
