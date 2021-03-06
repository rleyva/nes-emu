#include "helpers.hh"

#include <array>
#include <bits/stdint-uintn.h>
#include <stdint.h>
#include <string_view>
#include <type_traits>

#pragma once

//
// Implements opcodes for 6502.
//

namespace cpu
{

// Naive memory implementation
using memory = std::array<uint8_t, 0xFFFF>;

// CPU status flags
class flags
{
public:
    // Bit indices.
    static constexpr size_t CARRY      = 0U;
    static constexpr size_t ZERO       = 1U;
    static constexpr size_t INTERRUPT  = 2U;
    static constexpr size_t DECIMAL    = 3U;
    static constexpr size_t BREAKPOINT = 4U;
    static constexpr size_t OVERFLOW   = 6U;
    static constexpr size_t NEGATIVE   = 7U;

    void reset() { std::fill(std::begin(flags_), std::end(flags_), false); }
    bool &at(const size_t i) { return flags_.at(i); }
    const bool &at(const size_t i) const { return flags_.at(i); }; 

private:
    std::array<bool, 8U> flags_{false};
};

// CPU state
struct state
{
    uint16_t pc = 0x0000;   // Program-counter.
    uint16_t sp = 0x0000;   // Stack-pointer.
    flags    status;        // Processor flags.

    // Registers
    uint8_t reg_a = 0x00;   // Accumulator
    uint8_t reg_x = 0x00;   // X-index
    uint8_t reg_y = 0x00;   // Y-index
};

namespace address
{
    //
    // This namespace contains functions which carry out all of the addressing modes
    // that the 6502 is capable of.
    //
    // This includes:
    //     - Implicit
    //     - Immediate
    //     - Zero-Page (along w/ its variants)
    //     - Relative
    //     - Absolute
    //     - Indirect
    //
    // These functions wrap around an `Operation`, and provide a mechanism for fetching
    // the required data for said operation, providing it to the operation, and updating
    // any CPU state before returning. This decouples the operations from the addressing.
    //

    // Implicit Addressing
    //    This addressing mode encompasses functions which do not need to take in
    //    values from memory, namely operations like CLC (Clear Carry Flag), RTS, etc.
    const auto implicit = [](const auto &op, const memory &_, state &s)
    {
         // TODO: Might just be best to remove this
        //helpers::assert_contains_call_operator<Operation>();

        op(s);
        ++s.pc;       
    };

    // Immediate Addressing
    //    Byte immediately after the opcode is used.
    const auto immediate = [](const auto &op, const memory &mem, state &s)
    {
        op(s, mem.at(++s.pc));
        ++s.pc;
    };

    // Zero Page - This should be a single function like the following:

    // Zero Page
    //    Uses byte immediately after instruction. This limits it to fetching
    //    from $0000 - $00FF.
    const auto zero_page = [](const auto &op, const memory &mem, state &s)
    {
        // TODO: Remove the ol' static assert
        const uint8_t address = mem.at(++s.pc);
        op(s, mem.at(address));
        ++s.pc;
    };

    // Zero Page, X
    //    Uses 8-bit address opperand immediately after instruction, along with the
    //    value currently stored the X register to generate the address that we will
    //    fetch our value from.
    const auto zero_page_x = [](const auto &op, const memory &mem, state &s)
    {
        const auto address = helpers::sum_with_wrap_around(mem.at(++s.pc), s.reg_x, 0xFF);
        op(s, mem.at(address));
        ++s.pc;
    };

    // Zero Page, Y
    //    Same as zero_page_x.
    const auto zero_page_y = [](const auto &op, const memory &mem, state &s)
    {
        const auto address = helpers::sum_with_wrap_around(mem.at(++s.pc), s.reg_y, 0xFF);
        op(s, mem.at(address));
        ++s.pc;
    };

    // Relative
    //    Used for branching operations - will use the the byte immediately after the
    //    opcode as the branch offset. The new branch will be the current PC + offset.
    const auto relative = [](const auto &op, const memory &mem, state &s)
    {
        // This will operate on a signed range; 127 bytes forward, and 128 bytes backwards. 
        // Values in memory are two's complement signed, but when we carry out the operations
        // here we need to convert this over to a signed integer value.
        
        // NOTE: What happens if one were to set the PC to something less than $0?
        //       Does the CPU expect a wraparound? (I think all programs are supposed to start
        //       past 0xFF, so this won't be a problem)
        // NOTE: Op will handle setting the PC based on the CPU flags. 
        const auto relative_offset = helpers::convert_from_twos_complement(mem.at(++s.pc));
        op(s, relative_offset);
    };

    // Absolute
    //    Specifies memory location using the two bytes immediately following the opcode
    //    to generate the new address. The 6502 is little-endian therefore the LSB will
    //    be the first value loaded, followed by the MSB.
    //    NOTE: What happens when we load 0xFFFF?
    
    // Used to generate absolute addresses (doi....) 
    const auto generate_absolute_address = [](const uint8_t register_value,
                                              const memory &mem,
                                              state &       s)
    {
       const uint8_t LSB      = mem.at(++s.pc);
       const uint8_t MSB      = mem.at(++s.pc);
       const uint16_t address = helpers::create_two_byte_address(MSB, LSB) + register_value;

       return address;
    };

    const auto absolute = [](const auto &op, const memory &mem, state &s)
    {
        const uint16_t address = generate_absolute_address(0U, mem, s);
        op(s, mem.at(address));
        ++s.pc;
    };

    // Absolute, X
    //    Uses the values stored in the next two opcodes, along with the values stored
    //    in the X-register to generate the new address.
    const auto absolute_x = [](const auto &op, const memory &mem, state &s)
    {
        const uint8_t address = generate_absolute_address(s.reg_x, mem, s);
        op(s, mem.at(address));
        ++s.pc;
    };

    // Absolute, Y
    //    See above.
    const auto absolute_y = [](const auto &op, const memory &mem, state &s)
    {
        const uint8_t address = generate_absolute_address(s.reg_y, mem, s);
        op(s, mem.at(address));
        ++s.pc;
    };

    // Indirect
    //    The next two bytes immediately following the opcode are used to set the PC.
    //    NOTE: This addressing mode should only be used in conjunction with the JMP instr.
    const auto indirect = [](const auto &op, const memory &mem, state &s)
    {
        const uint8_t LSB = mem.at(++s.pc);
        const uint8_t MSB = mem.at(++s.pc);
        
        s.pc = helpers::create_two_byte_address(MSB, LSB);
    };

    // Indexed Indirect
    //
    const auto indexed_indirect = [](const auto &op, const memory &mem, state &s)
    {
        // NOTE: See https://stackoverflow.com/questions/46262435/indirect-y-indexed-addressing-mode-in-mos-6502
        //       to clear any confusion.
        const auto page_address = helpers::sum_with_wrap_around(mem.at(++s.pc), s.reg_x, 0xFF);
        const auto LSB          = mem.at(page_address);
        const auto MSB          = mem.at(page_address + 1U);

        s.pc = helpers::create_two_byte_address(MSB, LSB); 
    };

    // Indirect indexed
    //
    const auto indirect_indexed = [](const auto &op, const memory &mem, state &s)
    {
        // NOTE: See https://stackoverflow.com/questions/46262435/indirect-y-indexed-addressing-mode-in-mos-6502
        //       to clear any confusion.
        const uint8_t LSB = helpers::sum_with_wrap_around(mem.at(++s.pc), s.reg_y, 0xFF);
        const uint8_t MSB = mem.at(++s.pc); 

        s.pc = helpers::create_two_byte_address(MSB, LSB); 
    };

} // namespace address

namespace op
{
    // How should this be structured? It seems that we can make these a pair of op-code
    // to number of cycles since this is the only place in the arch where we are fully aware
    // of what it is that we are running with.
    //
    enum class codes : uint8_t
    {
        //
        // ADC instructions (Add with carry)
        //
        ADC_IMMEDIATE   = 0x69,
        ADC_ZERO_PAGE   = 0x65,
        ADC_ZERO_PAGE_X = 0x75,
        ADC_ABSOLUTE    = 0x6D,
        ADC_ABSOLUTE_X  = 0x7D,
        ADC_ABSOLUTE_Y  = 0x79,
        ADC_INDIRECT_X  = 0x61,
        ADC_INDIRECT_Y  = 0x71,

        //
        // AND instructions (Bit-wise and with accumulator)
        //
        AND_IMMEDIATE   = 0x29,
        AND_ZERO_PAGE   = 0x25,
        AND_ZERO_PAGE_X = 0x35,
        AND_ABSOLUTE    = 0x2D,
        AND_ABSOLUTE_X  = 0x3D,
        AND_ABSOLUTE_Y  = 0x39,
        AND_INDIRECT_X  = 0x21,
        AND_INDIRECT_Y  = 0x31,

        //
        // ASL (Arithmetic shift left) instructions.
        //
        ASL_ACCUMULATOR = 0x0A,
        ASL_ZERO_PAGE   = 0x06,
        ASL_ZERO_PAGE_X = 0x16,
        ASL_ABSOLUTE    = 0x0E,
        ASL_ABSOLUTE_X  = 0x1E,

        //
        // BIT (Test BITs)
        //
        BIT_ZERO_PAGE = 0x24,
        BIT_ABSOLUTE  = 0x2C,

        //
        // Branch instructions
        //
        BPL = 0x10, // Branch on plus
        BMI = 0x30, // Branch on minus
        BVC = 0x50, // Branch on Overflow clear
        BVS = 0x70, // Branch on Overflow set
        BCC = 0x90, // Branch on Carry clear
        BCS = 0xB0, // Branch on Carry set
        BNE = 0xD0, // Branch on not-equal
        BEQ = 0xF0, // Branch on equal

        //
        // BRK (Break)
        //
        BRK = 0x00,

        //
        // CMP (Compare accumulator)
        //
        CMP_IMMEDIATE   = 0xC9,
        CMP_ZERO_PAGE   = 0xC5,
        CMP_ZERO_PAGE_X = 0xD5,
        CMP_ABSOLUTE    = 0xCD,
        CMP_ABSOLUTE_X  = 0xDD,
        CMP_ABSOLUTE_Y  = 0xD9,
        CMP_INDIRECT_X  = 0xC1,
        CMP_INDIRECT_Y  = 0xD1,

        //
        // CPX (Compare X-register)
        //
        CPX_IMMEDIATE = 0xE0,
        CPX_ZERO_PAGE = 0xE4,
        CPX_ABSOLUTE  = 0xEC,

        //
        // CPY (Compare Y-register)
        //
        CPY_IMMEDIATE = 0xC0,
        CPY_ZERO_PAGE = 0xC4,
        CPY_ABSOLUTE  = 0xCC,

        //
        // DEC (Decrement memory)
        //
        DEC_ZERO_PAGE   = 0xC6,
        DEC_ZERO_PAGE_X = 0xD6,
        DEC_ABSOLUTE    = 0xCE,
        DEC_ABSOLUTE_X  = 0xDE,

        //
        // EOR (bitwise exclusive-OR)
        //
        EOR_IMMEDIATE   = 0x49,
        EOR_ZERO_PAGE   = 0x45,
        EOR_ZERO_PAGE_X = 0x55,
        EOR_ABSOLUTE    = 0x4D,
        EOR_ABSOLUTE_X  = 0x5D,
        EOR_ABSOLUTE_Y  = 0x59,
        EOR_INDIRECT_X  = 0x41,
        EOR_INDIRECT_Y  = 0x51,

        //
        // Flag instructions.
        //
        CLC = 0x18, // Clear carry
        SEC = 0x38, // Set carry
        CLI = 0x58, // Clear interrupt
        SEI = 0x78, // Set interrupt
        CLV = 0xB8, // Clear overflow
        CLD = 0xD8, // Clear decimal (DECIMAL)
        SED = 0xF8, // Set decimal (DECIMAL)

        //
        // INC (Increment memory)
        //
        INC_ZERO_PAGE   = 0xE6,
        INC_ZERO_PAGE_X = 0xF6,
        INC_ABSOLUTE    = 0xEE,
        INC_ABSOLUTE_X  = 0xFE,

        //
        // JMP ()
        //
        JMP_ABSOLUTE = 0x4C,
        JMP_INDIRECT = 0x6C,

        //
        // JSR (Jump to Subroutine)
        //
        JSR_ABSOLUTE = 0x20,

        //
        // LDA (Load Accumulator)
        //
        LDA_IMMEDIATE   = 0xA9,
        LDA_ZERO_PAGE   = 0xA5,
        LDA_ZERO_PAGE_X = 0xB5,
        LDA_ABSOLUTE    = 0xAD,
        LDA_ABSOLUTE_X  = 0xBD,
        LDA_ABSOLUTE_Y  = 0xB9,
        LDA_INDIRECT_X  = 0xA1,
        LDA_INDIRECT_Y  = 0xB1,

        //
        // LDX (Load X-register)
        //
        LDX_IMMEDIATE   = 0xA2,
        LDX_ZERO_PAGE   = 0xA4,
        LDX_ZERO_PAGE_Y = 0xB4,
        LDX_ABSOLUTE    = 0xAE,
        LDX_ABSOLUTE_Y  = 0xBE,

        //
        // LDY (Load Y-register)
        //
        LDY_IMMEDIATE   = 0xA0,
        LDY_ZERO_PAGE   = 0xA4,
        LDY_ZERO_PAGE_X = 0xB4,
        LDY_ABSOLUTE    = 0xAC,
        LDY_ABSOLUTE_X  = 0xBC,

       //
       // LSR (Logical shift-right)
       //
       LSR_ACCUMULATOR = 0x4A,
       LSR_ZERO_PAGE   = 0x46,
       LSR_ZERO_PAGE_X = 0x56,
       LSR_ABSOLUTE    = 0x4E,
       LSR_ABSOLUTE_X  = 0x5E,

       //
       // NOP (No Operation)
       //
       NOP = 0xEA,

       //
       // ORA (bitwise OR with Accumulator)
       //
       ORA_IMMEDIATE   = 0x09,
       ORA_ZERO_PAGE   = 0x05,
       ORA_ZERO_PAGE_X = 0x15,
       ORA_ABSOLUTE    = 0x0D,
       ORA_ABSOLUTE_X  = 0x1D,
       ORA_ABSOLUTE_Y  = 0x19,
       ORA_INDIRECT_X  = 0x01,
       ORA_INDIRECT_Y  = 0x11,

        //
        // Register Instructions
        //
        REG_TAX = 0xAA, // Transfer A to X
        REG_TXA = 0x8A, // Transfer X to A
        REG_DEX = 0xCA, // Decrement X
        REG_INX = 0xE8, // Increment X
        REG_TAY = 0xA8, // Transfer A to Y
        REG_TYA = 0x98, // Transfer Y to A
        REG_DEY = 0x88, // Decrement Y
        REG_INY = 0xC8, // Increment Y

        //
        // ROL (Rotate Left)
        //
        ROL_ACCUMULATOR = 0x2A,
        ROL_ZERO_PAGE   = 0x26,
        ROL_ZERO_PAGE_x = 0x36,
        ROL_ABSOLUTE    = 0x2E,
        ROL_ABSOLUTE_X  = 0x3E,

        //
        // ROR (Rotate Right)
        //
        ROR_ACCUMULATOR = 0x6A,
        ROR_ZERO_PAGE   = 0x66,
        ROR_ZERO_PAGE_X = 0x76,
        ROR_ABSOLUTE    = 0x6E,
        ROR_ABSOLUTE_X  = 0x7E,

        //
        // RTI (Return from Interrupt)
        //
        RTI = 0x40,

        //
        // RTS (Return from Subroutine)
        //
        RTS = 0x60,

        //
        // SBC (Subtract with Carry)
        //
        SBC_IMMEDIATE   = 0xE9,
        SBC_ZERO_PAGE   = 0xE5,
        SBC_ZERO_PAGE_X = 0xF5,
        SBC_ABSOLUTE    = 0xED,
        SBC_ABSOLUTE_X  = 0xFD,
        SBC_ABSOLUTE_Y  = 0xF9,
        SBC_INDIRECT_X  = 0xE1,
        SBC_INDIRECT_Y  = 0xF1,

        //
        // STA (Store Accumulator)
        //
        STA_ZERO_PAGE   = 0x85,
        STA_ZERO_PAGE_X = 0x95,
        STA_ABSOLUTE    = 0x8D,
        STA_ABSOLUTE_X  = 0x9D,
        STA_ABSOLUTE_Y  = 0x99,
        STA_INDIRECT_X  = 0x81,
        STA_INDIRECT_Y  = 0x91,

        //
        // Stack Instructions
        //
        TXS = 0x9A, // Transfer X to stack-pointer
        TSX = 0xBA, // Transfer stack-pointer to X
        PHA = 0x48, // Push to Accumulator
        PLA = 0x68, // Pull Accumulator
        PHP = 0x08, // Push Processor status
        PLP = 0x28, // Pull Processor status

        //
        // STX (Store X-register)
        //
        STX_ZERO_PAGE   = 0x86,
        STX_ZERO_PAGE_Y = 0x96,
        STX_ABSOLUTE    = 0x8E,

        //
        // STY (Store Y-register)
        //
        STY_ZERO_PAGE   = 0x84,
        STY_ZERO_PAGE_X = 0x94,
        STY_ABSOLUTE    = 0x8C
    };

    // Overflow flag computation.
    constexpr bool compute_overflow_flag(const uint8_t a, const uint8_t b)
    {
        // Reference: http://www.6502.org/tutorials/vflag.html
        return (65280 + a) - b < 65152 || (65280 + a) - b > 65407;
    }

    //
    // Operations
    //

    // ADC
    auto adc_op = [](state &state, const uint8_t value)
    {
        // NOTE: What happens if there's overflow w/ the carry addition?
        const auto carry_value     = state.status.at(flags::CARRY) ? 1U : 0U;
        const auto [result, carry] = helpers::sum_with_carry(state.reg_a, value + carry_value);

        // Set CPU flags.
        state.status.reset();
        state.status.at(flags::CARRY)    = carry;
        state.status.at(flags::ZERO)     = (result == 0);
        state.status.at(flags::OVERFLOW) = op::compute_overflow_flag(state.reg_a, value + carry_value);
        state.status.at(flags::NEGATIVE) = (result & (1 << 7U));

        // Set value in accumulator.
        state.reg_a = result;
    };

    // AND
    const auto and_op = [](state &state, const uint8_t value)
    {
        const uint8_t result = state.reg_a & value;

        state.status.reset();
        state.status.at(flags::ZERO)     = (result == 0);
        state.status.at(flags::NEGATIVE) = (result & (1 << 7U));

        // Save value in accumulator.
        state.reg_a = result;
    };

    // ASL
    const auto asl_op = [](state &state)
    {
        // Is there a way where we can ensure we never attempt to call this with
        // an unsupported addressing mode?
        const bool    carry_result = (state.reg_a >= 128U);
        const uint8_t result       = (1U << state.reg_a);

        // Set flags.
        state.status.reset();
        state.status.at(flags::CARRY)    = carry_result;
        state.status.at(flags::ZERO)     = (result == 0);
        state.status.at(flags::NEGATIVE) = (result & (1 << 7U));

        // Save value in accumulator.
        state.reg_a = result;
    };

    // BCC (branch on carry clear)
    const auto bcc_op = [](state &state, const int8_t relative_offset)
    {
        !state.status.at(flags::CARRY) ? state.pc += relative_offset : state.pc += 1U;
        
        // NOTE: Do these instructions require us to reset the status reg?
    };

    // BCS (branch on carry set)
    const auto bcs_op = [](state &state, const int8_t relative_offset)
    {
        state.status.at(flags::CARRY) ? state.pc += relative_offset : state.pc += 1U;
    };

    // BEQ (branch on equal)
    const auto beq_op = [](state &state, const int8_t relative_offset)
    {
        state.status.at(flags::ZERO) ? state.pc += relative_offset : state.pc += 1U;
    };

    // BCS (branch on carry set)
    const auto bit_op = [](state &state, const uint8_t value)
    {
        const bool masking_result = state.reg_a & value;
        
        state.status.reset();
        state.status.at(flags::NEGATIVE) = (masking_result & (1 << 7)); // Set to value of bit 7
        state.status.at(flags::OVERFLOW) = (masking_result & (1 << 6)); // Set to value of bit 6
        state.status.at(flags::ZERO)     = (masking_result == 0x00);
    };

} // namespace op

// Struct containing everything needed to define a 6502 instruction. 
template<typename AddressingFunc, typename OperationFunc>
class instruction
{
public:
    instruction(const std::string_view &name,
                const uint8_t           num_clk_cycles,
                const AddressingFunc    addressing_function,
                const OperationFunc     operation_function)
        : name(name),
          num_clk_cycles(num_clk_cycles),
          addr_func_(addressing_function),
          op_func_(operation_function)
    {
    }

    // Readable name for the instruction that this struct represents.
    std::string_view name;
    
    // Number of cycles that the operation will take to complete.
    uint8_t num_clk_cycles;

    // 
    void operator()(const memory &mem, state &s) const
    {
        // TODO: Handle num_clk_cycles here.
        addr_func_(op_func_, mem, s);
    }

    // TODO: Add << operator

private:
    // TODO: Add some template bits that will ensure we don't mix up an addressing
    //       function and an operation function.
    AddressingFunc addr_func_;
    
    OperationFunc op_func_;
};

// Debug ostream operators.
std::ostream& operator << (std::ostream &os, const state &s);
std::ostream& operator << (std::ostream &os, const flags &f);

} // namespace cpu
