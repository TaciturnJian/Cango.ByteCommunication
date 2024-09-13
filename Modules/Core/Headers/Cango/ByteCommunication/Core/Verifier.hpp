#pragma once

#include <concepts>

#include "ByteTypes.hpp"

namespace Cango :: inline ByteCommunication :: inline Core {
	/// @brief @c Verifier 的概念，用于检查数据包是否完整，要求可默认初始化
	template <typename TObject>
	concept IsVerifier = std::default_initializable<TObject> && requires(TObject& object, CByteSpan span) {
		{ object.Verify(span) } -> std::convertible_to<bool>;
	};

	/// @brief 返回常量的 @c Verifier
	template<bool TResult>
	class ConstexprResultVerifier final {
	public:
		// ReSharper disable once CppMemberFunctionMayBeStatic
		[[nodiscard]] constexpr bool Verify(const CByteSpan) const noexcept { return TResult; }
	};

	using AllowAnythingVerifier = ConstexprResultVerifier<true>;
	using DenyAnythingVerifier = ConstexprResultVerifier<false>;

	/// @brief 检验数据包尾部是否为目标字节
	template<ByteType TTailByte>
	class TailByteVerifier final {
	public:
		// ReSharper disable once CppMemberFunctionMayBeStatic
		[[nodiscard]] bool Verify(const CByteSpan span) const noexcept { // NOLINT(*-convert-member-functions-to-static)
			if (span.empty()) return false;
			return span.back() == TTailByte;
		}
	};

	using TailZeroVerifier = TailByteVerifier<0>;

	/// @brief 在运行时确定的 @c Verifier
	class RuntimeVerifier {
	public:
		[[nodiscard]] virtual bool Verify(CByteSpan span) noexcept = 0;

		virtual ~RuntimeVerifier() = default;
	};

	/// @brief 将编译时确定的 @c Verifier 包装为运行时确定的 @c Verifier
	/// @details 注意这里 @c TVerifier 要求可默认构造
	template<IsVerifier TVerifier>
	class WrapVerifierToRuntime final : public RuntimeVerifier, public TVerifier {
	public:
		[[nodiscard]] bool Verify(CByteSpan span) noexcept override { return TVerifier::Verify(span); }
	};
}
