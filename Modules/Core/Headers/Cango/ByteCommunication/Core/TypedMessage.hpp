#pragma once

#include <ranges>
#include <ostream>
#include <iomanip>
#include <format>

#include "ByteTypes.hpp"

namespace Cango :: inline ByteCommunication :: inline Core {
	template<typename TMessage>
	struct BasicTypedMessage {
		/// @brief 将数据从 Data 复制到目标内存
		///	@param object 指向目标的内存，必须是 TDataSize 大小
		template <typename T>
		void CopyDataTo(T* object) const noexcept {
			static_assert(sizeof(T) == TMessage::DataSize, "inconsistent size of data and object");
			std::ranges::copy(reinterpret_cast<const TMessage*>(this)->Data, reinterpret_cast<ByteType*>(object));
		}

		/// @brief 将内部数据视作目标类型的引用
		///	@tparam T 目标类型，其大小必须和 DataSize 相同
		template <typename T>
		[[nodiscard]] T& GetDataAs() noexcept {
			static_assert(sizeof(T) == TMessage::DataSize, "inconsistent size of data and object");
			return *reinterpret_cast<T*>(reinterpret_cast<const TMessage*>(this)->Data.data());
		}

		/// @brief 将内部数据视作目标类型的常量引用
		///	@tparam T 目标类型，其大小必须和 DataSize 相同
		template <typename T>
		[[nodiscard]] const T& GetDataAs() const noexcept {
			static_assert(sizeof(T) == TMessage::DataSize, "inconsistent size of data and object");
			return *reinterpret_cast<const T*>(reinterpret_cast<const TMessage*>(this)->Data.data());
		}

		/// @brief 构造 std::span，用于将数据从其他位置读取到此空间
		[[nodiscard]] ByteSpan ToSpan() noexcept { return {reinterpret_cast<ByteType*>(this), TMessage::FullSize}; }

		/// @brief 构造 std::span，用于将数据从此空间写入其他位置
		[[nodiscard]] CByteSpan ToSpan() const noexcept { return {reinterpret_cast<const ByteType*>(this), TMessage::FullSize}; }

		// 格式为十六进制，右对齐，不足两位的补零
		static std::ostream& FormatByte(std::ostream& stream, ByteType byte) noexcept {
			return stream << std::format("{:02X}", byte);
		}

		std::ostream& Format(std::ostream& stream) const noexcept {
			const CByteSpan span = ToSpan();
			for (const auto byte : span.subspan(0, span.size() - 1))
				FormatByte(stream, byte) << ' ';
			return FormatByte(stream, span.back());
		}

	};

#pragma pack(push, 1)
	/// @brief 一种用来通信的结构，包含头(Head)、数据类型(Type)、数据区(Data)、尾(Tail)
	///		提供了一些有关 Data 的转换函数，和转换对象为 std::span 的工具函数
	///	@tparam TDataSize 数据区长度，此结构的总长度为数据区长度 + 3
	template <std::size_t TDataSize>
	struct TypedMessage final : BasicTypedMessage<TypedMessage<TDataSize>>{
		static constexpr SizeType DataSize = TDataSize;
		static constexpr SizeType FullSize = TDataSize + sizeof(ByteType) * 3;

		/// @brief 头字节，用于标识数据包的开始
		ByteType Head{'!'};

		/// @brief 标识消息种类
		ByteType Type{0};

		/// @brief 数据段，用于存放消息数据
		ByteArray<TDataSize> Data{};

		/// @brief 尾字节，用于验证数据包
		ByteType Tail{0};
	};
#pragma pack(pop)
}
