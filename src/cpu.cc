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
  using indices = cpu::flags::indices;
  os << "[ N: " << f.at(indices::NEGATIVE)
     << " | O: " << f.at(indices::OVERFLOW)
     << " | B: " << f.at(indices::BREAKPOINT)
     << " | D: " << f.at(indices::DECIMAL)
     << " | I: " << f.at(indices::INTERRUPT) << " | Z: " << f.at(indices::ZERO)
     << " | C: " << f.at(indices::CARRY) << " ]";

  return os;
}
} // namespace cpu
