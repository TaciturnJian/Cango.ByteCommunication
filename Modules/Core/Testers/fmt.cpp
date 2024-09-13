#include <format>
#include <iostream>
#include <Cango/ByteCommunication/Core.hpp>

using MyMessage = Cango::TypedMessage<12>;

template<>
struct std::formatter<MyMessage> : MyMessage::formatter{};


int main() {
    Cango::TypedMessage<12> message{};
    // fill data with [0,11]
    for (std::size_t i = 0; i < 12; ++i) {
        message.Data[i] = static_cast<Cango::ByteType>(i);
    }

    std::cout << std::format("{:02X}\n", message);
    return 0;
}
