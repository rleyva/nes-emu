#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "../cpu.hh"

namespace {

using flags = cpu::flags;
using state = cpu::state;

cpu::state mock_state() {
    cpu::flags flags;
    flags.at(flags::indices::CARRY) = true;

    cpu::state state;
    state.f = flags;

    return state;
}

cpu::memory mock_memory() { cpu::memory memory; }

} // namespace

TEST_CASE("CPU State is created", "[addressing]") {
    auto state  = mock_state();
    auto memory = mock_memory();

    std::cout << "---- Initial State -----" << std::endl;
    std::cout << state << std::endl;

    cpu::address::implicit(cpu::op::adc_func, state);
    cpu::address::implicit(cpu::op::and_func, state);

    std::cout << "---- Final State ----" << std::endl;
    std::cout << state << std::endl;
}
