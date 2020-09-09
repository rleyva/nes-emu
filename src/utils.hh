#include <algorithm>
#include <array>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace utils
{
    // Funtion will attempt to open file containing 
    std::vector<uint8_t> read_binary_blob(const std::string &file_path);

    // Function will take a text file containing assembly instructions
    // and generate machine code.
    //void generate_assembly(const);

    // Function will take a generated machine code, and convert it to
    // human-readable assembly.
    //void generate_disassembly(const auto path, const bool header=true);

    // Function will... TBD.
    //void pretty_print_hex();

    // Constexpr map
    template<typename Key, typename Value, std::size_t Size>
    struct Map {
        std::array<std::pair<Key, Value>, Size> data;

        constexpr Value at(const Key &key) const
        {
            const auto itr = std::find_if(std::begin(data), std::end(data),
                    [&key](const auto &v)
                    {
                        return v.first == key;
                    });

            if(itr != std::end(data))
            {
                return itr->second;
            }
            else
            {
                throw std::range_error("Key not found!");
            }
        }
    };
}
// namespace utils
