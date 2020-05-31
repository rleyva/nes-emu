#include "utils.hh"

#include <array>
#include <functional>
#include <stdint.h>
#include <iostream>
#include <type_traits>

#pragma once

//
// Implements opcodes for 6502.
//

namespace cpu
{

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
    const bool &at(const size_t i) const { return flags_.at(i); }

private:
    std::array<bool, 8U> flags_{false}; 
};
 
struct state
{
    uint16_t pc = 0x0000;   // Program-counter.
    uint16_t sp = 0x0000;   // Stack-pointer.
    flags    f;             // Processor flags.

    // Registers
    uint8_t reg_a = 0x00;   // Accumulator
    uint8_t reg_x = 0x00;   // X-index
    uint8_t reg_y = 0x00;   // Y-index  
};

using memory = std::array<uint8_t, 0xFFFF>;

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
    template <typename Operation>
    void implicit(const Operation &op, state &s)
    {
        // TODO: Might just be best to remove this
        utils::assert_contains_call_operator<Operation>();

        const uint8_t immediate_value = 20;
        op(s, immediate_value);
        ++s.pc;        
    }

    // Immediate Addressing
    //    Byte immediately after the opcode is used.  
    template <typename Operation>
    void immediate(const Operation &op, const memory &mem, state &s)
    {   
        ++s.pc;
        op(s, mem.at(s.pc));
        ++s.pc;
    }
    
    // Zero Page
    //    Uses byte immediately after instruction. This limits it to fetching
    //    from $0000 - $00FF.
    template <typename Operation>
    void zero_page(const Operation &op, const memory &mem, state &s)
    {
        // TODO: Remove the ol' static assert
        const auto &address = mem.at(++s.pc);
        if(address > 0xFF)
        { 
            throw std::runtime_error("Fetched page address greater than 0xFF");
        }
         
        op(s, mem.at(address));
        ++s.pc;
    }
    
    // Zero Page, X
    //    Uses 8-bit address opperand immediately after instruction, along with the
    //    value currently stored the X register to generate the address that we will
    //    fetch our value from.
    template <typename Operation> 
    void zero_page_x(const Operation &op, const memory &mem, state &s)
    {
        // TODO: Revisit this, the cast to uint8_t will make it so that the address
        //       that we're operating on is between 0x0000 - 0x00FF.
        const auto address = static_cast<uint8_t>(mem.at(++s.pc) + s.reg_x);
        if(address > 0xFF)
        {
            throw std::runtime_error("Fetched page address greater than 0xFF");
        }
    
        op(s, mem.at(address));
        ++s.pc;
    }

    // Zero Page, Y
    //    Same as zero_page_x.
    template <typename Operation> 
    void zero_page_y(const Operation &op, const memory &mem, state &s)
    {
        const auto address = static_cast<uint8_t>(mem.at(++s.pc) + s.reg_y);
        if(address > 0xFF)
        {
            throw std::runtime_error("Fetched page address greater than 0xFF");
        }
    
        op(s, mem.at(address));
        ++s.pc;
    }
    
    // Relative
    //    Used for branching operations - will use the the byte immediately after the
    //    opcode as the branch offset. The new branch will be the current PC + offset.
    template <typename Operation>
    void relative(const Operation &op, const memory &mem, state &s)
    {
        // TODO: How are negative values stored?
        const int8_t relative_offset = mem.at(++s.pc); 
        s.pc += relative_offset;
    }
    
    // Absolute
    //    Specifies memory location using the two bytes immediately following the opcode
    //    to generate the new address. The 6502 is little-endian therefore the LSB will 
    //    be the first value loaded, followed by the MSB.
    template <typename Operation>
    void absolute(const Operation &op, const memory &mem, state &s)
    {
        ++s.pc;
        const uint8_t LSB = mem.at(s.pc); 
        
        ++s.pc;
        const uint8_t MSB = mem.at(s.pc);  

        const uint16_t address =  (static_cast<uint8_t>(MSB) << 8U) | LSB;        
        
        op(s, mem.at(address));
        ++s.pc;
    }
    
    // Absolute, X
    //    Uses the values stored in the next two opcodes, along with the values stored
    //    in the X-register to generate the new address.
    /*
    void absolute_x()
    {
    }
   
    // Absolute, Y
    //    See above.
    void absolute_y()
    {
    }
   
    // Indirect
    //    The next two bytes immediately following the opcode are used to set the PC.
    void indirect()
    {
    }

    // Indexed Indirect
    //     
    void indexed_indirect()
    {
    }

    void indirect_indexed()
    {
    }
    */

} // namespace address

namespace op
{
    enum class codes : uint8_t {
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

    // Carry functions.
    inline void clear_carry(flags &flags) { flags.at(flags::CARRY) = false; }
    inline void set_cary(flags &flags) { flags.at(flags::CARRY) = true; }

    // Interrupt functions.
    inline void clear_interrupt(flags &flags) { flags.at(flags::INTERRUPT) = false; }
    inline void set_interrupt(flags &flags) { flags.at(flags::INTERRUPT) = true; } 

    // Overflow functions.
    inline void clear_overflow(flags &flags) { flags.at(flags::OVERFLOW) = false; }
   
    // Decimal functions.
    inline void clear_decimal(flags &flags) { flags.at(flags::DECIMAL) = false; }  
    inline void set_decimal(flags &flags) { flags.at(flags::DECIMAL) = true; }

    //
    // Operations 
    //

    // ADC
    const auto adc_func = [](state &state, const uint8_t m)
    {
        // TODO: Revist this uint16_t nonsense.
        auto &carry_flag         = state.f.at(flags::CARRY);
        const uint16_t result    = state.reg_a + m + (carry_flag ? 1U : 0U);
        const bool     carry_set = result > 255U; 
  
        // Set CPU flags.
        state.f.reset();
        state.f.at(flags::CARRY)    = carry_set;  
        state.f.at(flags::ZERO)     = (result == 0);
        state.f.at(flags::OVERFLOW) = true; // TODO: Revisit
        state.f.at(flags::NEGATIVE) = (result & (1 << 7U));
    
        // Set value in accumulator.
        state.reg_a = static_cast<uint8_t>(result);    
    };

    // AND 
    const auto and_func = [](state &state, const uint8_t m) 
    {
        const uint8_t result = state.reg_a & m;

        state.f.reset();
        state.f.at(flags::ZERO)     = (result == 0);
        state.f.at(flags::NEGATIVE) = (result & (1 << 7U));       
        
        // Save value in accumulator. 
        state.reg_a = result;
    };

    // ASL
    const auto asl = [](state &state)
    {
        // Is there a way where we can ensure we never attempt to call this with
        // an unsupported addressing mode? 
        const bool    carry_result = (state.reg_a >= 128U);
        const uint8_t result       = (1U << state.reg_a);
        
        // Set flags.
        state.f.reset();
        state.f.at(flags::CARRY)    = carry_result;
        state.f.at(flags::ZERO)     = (result == 0);
        state.f.at(flags::NEGATIVE) = (result & (1 << 7U));

        // Save value in accumulator.
        state.reg_a = result;
    };

    // 

} // namspace op

// Debug ostream operators.
std::ostream& operator << (std::ostream &os, const state &s);
std::ostream& operator << (std::ostream &os, const flags &f);

} // namespace cpu
