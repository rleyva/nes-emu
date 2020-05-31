#include <iomanip>

#include "cpu.hh"

namespace cpu {

std::ostream &operator<<(std::ostream &os, const state &s) {
  os << "PC:    " << std::hex << "0x" << s.pc << " (" << std::dec << s.pc
     << ")\n"
     << "SP:    " << std::hex << "0x" << s.sp << " (" << std::dec << s.sp
     << ")\n"
     << "A-reg: " << std::hex << "0x" << static_cast<uint16_t>(s.reg_a) << " ("
     << std::dec << static_cast<uint16_t>(s.reg_a) << ")\n"
     << "X-reg: " << std::hex << "0x" << static_cast<uint16_t>(s.reg_x) << " ("
     << std::dec << static_cast<uint16_t>(s.reg_x) << ")\n"
     << "Y-reg: " << std::hex << "0x" << static_cast<uint16_t>(s.reg_y) << " ("
     << std::dec << static_cast<uint16_t>(s.reg_y) << ")\n"
     << "Flags: " << s.f;

  return os;
}

std::ostream &operator<<(std::ostream &os, const flags &f) {
  os << "[ N: " << f.at(flags::NEGATIVE)
     << " | O: " << f.at(flags::OVERFLOW)
     << " | B: " << f.at(flags::BREAKPOINT)
     << " | D: " << f.at(flags::DECIMAL)
     << " | I: " << f.at(flags::INTERRUPT) << " | Z: " << f.at(flags::ZERO)
     << " | C: " << f.at(flags::CARRY) << " ]";

  return os;
}
} // namespace cpu
