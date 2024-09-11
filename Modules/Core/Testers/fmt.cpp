#include <format>
#include <iostream>
#include <Cango/ByteCommunication/Core.hpp>

int main() {
    std::array<Cango::ByteType, 100> data{};
    for (std::size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<Cango::ByteType>(i);
    }
    std::cout << std::format("{:02X}\n", Cango::CByteSpan{data});
    return 0;
}
