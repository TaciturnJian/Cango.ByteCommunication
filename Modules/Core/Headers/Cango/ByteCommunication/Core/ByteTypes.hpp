#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace Cango :: inline ByteCommunication :: inline Core {
	using SizeType = std::size_t;
	using ByteType = std::uint8_t;
	using ByteSpan = std::span<ByteType>;
	using CByteSpan = std::span<const ByteType>;

	template<SizeType TSize>
	using ByteArray = std::array<ByteType, TSize>;
}
