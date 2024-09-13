#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>

namespace Cango :: inline ByteCommunication :: inline Core {
	using SizeType = std::size_t;
	using ByteType = std::uint8_t;
	using ByteSpan = std::span<ByteType>;

	/// @brief 只读字节范围
	/// 格式化器见 @c std::formatter<Cango::CByteSpan>
	using CByteSpan = std::span<const ByteType>;

	template<SizeType TSize>
	using ByteArray = std::array<ByteType, TSize>;
}

/// @brief 实现利用字节类型的格式标准实现只读字符区间的格式化
template<>
struct std::formatter<Cango::CByteSpan> : std::formatter<Cango::ByteType> {
	auto format(const Cango::CByteSpan& span, std::format_context& ctx) const {
		using byte_formatter = std::formatter<Cango::ByteType>; // 此格式化器将使用当前上下文，包含用户传递的格式说明符

		if (span.empty()) return ctx.out();
		const auto count = span.size();
		if (count == 1) return byte_formatter::format(span.front(), ctx);

		for (const auto& element : span.subspan(0, count - 1)) {
			const auto it = byte_formatter::format(element, ctx);
			std::format_to(it, " ");
		}
		return byte_formatter::format(span.back(), ctx);
	}
};
