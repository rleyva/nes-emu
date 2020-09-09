#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../cpu.hh"

namespace testing
{
    // No-Op operation for testing addressing.
    const auto noop = [](cpu::state &s, const uint8_t val){};
}

namespace constants
{
    // Hold constants necessary for unit-testing.

    constexpr uint16_t DEFAULT_PROGRAM_COUNTER = 0x1234;
    constexpr uint16_t DEFAULT_STACK_POINTER   = 0x2345;
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

    state.pc     = constants::DEFAULT_PROGRAM_COUNTER;
    state.sp     = constants::DEFAULT_STACK_POINTER;
    state.reg_a  = constants::DEFAULT_REG_A;
    state.reg_x  = constants::DEFAULT_REG_X;
    state.reg_y  = constants::DEFAULT_REG_Y;
    state.status = cpu::flags{};

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

    cpu::address::immediate(testing::noop, memory, state);

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

    cpu::address::zero_page(testing::noop, memory, state);

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

    cpu::address::zero_page_x(testing::noop, memory, state);

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

    cpu::address::zero_page_y(testing::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 2U);
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Relative", "[addressing]")
{
    // TODO: Make this less stupid...

    SECTION("Positive Offset")
    {
        auto state  = create_mock_state();
        auto memory = create_mock_memory(0xFF);

        constexpr auto PC_OFFSET      = 0x14; // 20 (base10)
        constexpr auto OFFSET_ADDRESS = constants::DEFAULT_PROGRAM_COUNTER + 1U;
        constexpr auto EXPECTED_PC    = OFFSET_ADDRESS + PC_OFFSET;
        memory.at(OFFSET_ADDRESS) = PC_OFFSET;

        cpu::address::relative(testing::noop, memory, state);

        REQUIRE(state.pc == EXPECTED_PC);    
        REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
        REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
        REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
        REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);   
    }

    SECTION("Negative Offset")
    {
        auto state  = create_mock_state();
        auto memory = create_mock_memory(0xFF);

        constexpr auto PC_OFFSET      = 0xFF; // -128 (base10; two's complement)
        constexpr auto OFFSET_ADDRESS = constants::DEFAULT_PROGRAM_COUNTER + 1U;
        constexpr auto EXPECTED_PC    = OFFSET_ADDRESS + helpers::convert_from_twos_complement(PC_OFFSET);
        memory.at(OFFSET_ADDRESS) = PC_OFFSET;

        cpu::address::relative(testing::noop, memory, state);

        REQUIRE(state.pc == EXPECTED_PC);    
        REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
        REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
        REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
        REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
   }
}

TEST_CASE("Addressing: Absolute", "[addressing]")
{
    auto state  = create_mock_state();
    auto memory = create_mock_memory(0x11);

    cpu::address::absolute(testing::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 3U);    
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Absolute, X", "[addressing]")
{
    auto state  = create_mock_state();
    auto memory = create_mock_memory(0x11);

    cpu::address::absolute_x(testing::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 3U);    
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Absolute, Y", "[addressing]")
{
    auto state  = create_mock_state();
    auto memory = create_mock_memory(0x11);

    cpu::address::absolute_y(testing::noop, memory, state);

    REQUIRE(state.pc == constants::DEFAULT_PROGRAM_COUNTER + 3U);    
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Indirect", "[addressing]")
{
    auto state  = create_mock_state();
    auto memory = create_mock_memory(0xFF);

    constexpr auto PC_ADDRESS = helpers::create_two_byte_address(0xAB, 0xCD); // 0xABCD
    memory.at(constants::DEFAULT_PROGRAM_COUNTER + 1U) = 0xCD; // LSB
    memory.at(constants::DEFAULT_PROGRAM_COUNTER + 2U) = 0xAB; // MSB

    cpu::address::indirect(testing::noop, memory, state);

    REQUIRE(state.pc == PC_ADDRESS);    
    REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
    REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
    REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
    REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
}

TEST_CASE("Addressing: Indexed Indirect", "[addressing]")
{
    SECTION("without Wrap-Around")
    {
        auto state  = create_mock_state();
        auto memory = create_mock_memory(0xFF); 
        
        constexpr uint8_t PAGE_ADDRESS = 0x21;   // Page address that we want to jump to.
        constexpr uint16_t PC_ADDRESS  = 0x5432; // Address that we will set the PC-counter to.
        
        memory.at(constants::DEFAULT_PROGRAM_COUNTER + 1U)      = PAGE_ADDRESS; 
        memory.at(PAGE_ADDRESS + constants::DEFAULT_REG_X)      = 0x32;
        memory.at(PAGE_ADDRESS + constants::DEFAULT_REG_X + 1U) = 0x54;

        cpu::address::indexed_indirect(testing::noop, memory, state);

        REQUIRE(state.pc == PC_ADDRESS);    
        REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
        REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
        REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
        REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
    }

    SECTION("with Wrap-Around")
    {
        auto state  = create_mock_state();
        auto memory = create_mock_memory(0xFF); 
        
        constexpr uint8_t PAGE_ADDRESS = 0xFF;
        constexpr uint16_t PC_ADDRESS  = 0x5432; // Address that we will set the PC-counter to.
        constexpr auto WRAPPED_ADDRESS
            = helpers::sum_with_wrap_around(PAGE_ADDRESS, constants::DEFAULT_REG_X, 0xFF);
 
        memory.at(WRAPPED_ADDRESS)      = 0x32;
        memory.at(WRAPPED_ADDRESS + 1U) = 0x54;

        cpu::address::indexed_indirect(testing::noop, memory, state);

        REQUIRE(state.pc == PC_ADDRESS);    
        REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
        REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
        REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
        REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
    }
}

TEST_CASE("Addressing: Indirect Indexed", "[addressing]")
{
    SECTION("without Wrap-Around")
    {
        auto state  = create_mock_state();
        auto memory = create_mock_memory(0xFF); 
        
        constexpr uint16_t PC_ADDRESS  = 0x5456; // Address that we will set the PC-counter to.
        
        memory.at(constants::DEFAULT_PROGRAM_COUNTER + 1U) = 0x01; 
        memory.at(constants::DEFAULT_PROGRAM_COUNTER + 2U) = 0x54;

        cpu::address::indirect_indexed(testing::noop, memory, state);

        REQUIRE(state.pc == PC_ADDRESS);    
        REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
        REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
        REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
        REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
    }

    SECTION("with Wrap-Around")
    {
        auto state  = create_mock_state();
        auto memory = create_mock_memory(0xFF); 
        
        constexpr uint16_t PC_ADDRESS  = 0x5455; // Address that we will set the PC-counter to.
        
        memory.at(constants::DEFAULT_PROGRAM_COUNTER + 1U) = 0xFF; 
        memory.at(constants::DEFAULT_PROGRAM_COUNTER + 2U) = 0x54;

        cpu::address::indirect_indexed(testing::noop, memory, state);

        REQUIRE(state.pc == PC_ADDRESS);    
        REQUIRE(state.sp == constants::DEFAULT_STACK_POINTER);
        REQUIRE(state.reg_a == constants::DEFAULT_REG_A);
        REQUIRE(state.reg_x == constants::DEFAULT_REG_X);
        REQUIRE(state.reg_y == constants::DEFAULT_REG_Y);
    }
}
