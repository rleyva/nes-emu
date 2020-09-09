#include "utils.hh"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace utils
{
    // NOTE: Maybe we can move this over to a std::array<std::uint8_t, 0xFFFF>,
    //       since the amount of accessible memory is something that is always
    //       constant.
    std::vector<uint8_t> read_binary_blob(const std::string &file_path)
    {
        std::ifstream bs(file_path);
        
        if(!bs.is_open())
        {
            throw std::runtime_error("Failed to open file at given path!");
        }
    
        std::cout << "Successfully opened " << file_path << "!" << std::endl;
        return {std::istreambuf_iterator<char>(bs), std::istreambuf_iterator<char>()};
    }

} // namespace utils
