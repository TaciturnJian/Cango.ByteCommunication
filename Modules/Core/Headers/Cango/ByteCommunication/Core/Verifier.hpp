#pragma once

#include "ByteTypes.hpp"

namespace Cango :: inline ByteCommunication :: inline Core {
	struct Verifier {
		virtual bool Verify(CByteSpan span) noexcept = 0;
	};

	/// @brief @c Verifier 的概念，用于检查数据包是否完整，要求可默认初始化
	template <typename TObject>
	concept IsVerifier = std::default_initializable<TObject> && requires(TObject& object, CByteSpan span) {
		{ object.Verify(span) } -> std::convertible_to<bool>;
	};

	struct AllowAnythingVerifier final {
		// ReSharper disable once CppMemberFunctionMayBeStatic
		[[nodiscard]] constexpr bool Verify(CByteSpan) const noexcept { return true; }
	};

	struct TailZeroVerifier final {
		// ReSharper disable once CppMemberFunctionMayBeStatic
		[[nodiscard]] bool Verify(const CByteSpan span) const noexcept {
			return span.back() == 0;
		}
	};
}
