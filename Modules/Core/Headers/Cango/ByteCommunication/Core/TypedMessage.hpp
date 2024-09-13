#pragma once

#include <ranges>
#include <format>
#include <concepts>

#include "ByteTypes.hpp"

namespace Cango :: inline ByteCommunication :: inline Core {
	/// @brief 为 TypedMessage 实现额外功能，包括数据复制、数据转换、格式化等
	template<typename TMessage>
	class FunctionsForTypedMessage {
	public:
		/// @brief 将数据从 Data 复制到目标内存
		///	@param object 指向目标的内存，必须是 TDataSize 大小
		template <typename T>
		requires std::is_trivially_copyable_v<T>
		void CopyDataTo(T* object) const noexcept {
			static_assert(sizeof(T) == TMessage::DataSize, "inconsistent size of data and object");
			std::ranges::copy(reinterpret_cast<const TMessage*>(this)->Data, reinterpret_cast<ByteType*>(object));
		}

		/// @brief 将内部数据视作目标类型的引用
		///	@tparam T 目标类型，其大小必须和 DataSize 相同
		template <typename T>
		requires std::is_trivially_assignable_v<T, const T>
		[[nodiscard]] T& GetDataAs() noexcept {
			static_assert(sizeof(T) == TMessage::DataSize, "inconsistent size of data and object");
			return *reinterpret_cast<T*>(reinterpret_cast<const TMessage*>(this)->Data.data());
		}

		/// @brief 将内部数据视作目标类型的常量引用
		///	@tparam T 目标类型，其大小必须和 DataSize 相同
		template <typename T>
		requires std::is_trivially_assignable_v<T, const T>
		[[nodiscard]] const T& GetDataAs() const noexcept {
			static_assert(sizeof(T) == TMessage::DataSize, "inconsistent size of data and object");
			return *reinterpret_cast<const T*>(reinterpret_cast<const TMessage*>(this)->Data.data());
		}

		/// @brief 构造 std::span，用于将数据从其他位置读取到此空间
		[[nodiscard]] ByteSpan ToSpan() noexcept { return {reinterpret_cast<ByteType*>(this), TMessage::FullSize}; }

		/// @brief 构造 std::span，用于将数据从此空间写入其他位置
		[[nodiscard]] CByteSpan ToSpan() const noexcept { return {reinterpret_cast<const ByteType*>(this), TMessage::FullSize}; }

		/// @brief 预定义的格式化器，将 @c TypedMessage 转化为字节区间格式化
		struct formatter : std::formatter<CByteSpan> {
			auto format(const TMessage& message, std::format_context& ctx) const {
				return std::formatter<CByteSpan>::format(message.ToSpan(), ctx);
			}
		};
	};

#pragma pack(push, 1)
	/// @brief 一种用来通信的结构，含头尾标记用于检验，还有一个额外字节指示该数据包的种类。
	///	@details
	///		包含头(Head)、数据类型(Type)、数据区(Data)、尾(Tail)
	///		提供了一些有关 Data 的转换函数，和转换对象为 std::span 的工具函数
	///		要让自定义类可格式化，需要特化 @c std::formatter ，可以继承此类下的 @c formatter 简化工作
	///
	///	@tparam TDataSize 数据区长度，此结构的总长度为数据区长度 + 3
	template <std::size_t TDataSize>
	struct TypedMessage final : FunctionsForTypedMessage<TypedMessage<TDataSize>> {
		using DataType = ByteArray<TDataSize>;
		static constexpr SizeType DataSize = TDataSize;
		static constexpr SizeType FullSize = TDataSize + sizeof(ByteType) * 3;

		/// @brief 头字节，用于标识数据包的开始
		ByteType Head{'!'};

		/// @brief 标识消息种类
		ByteType Type{0};

		/// @brief 数据段，用于存放消息数据
		DataType Data{};

		/// @brief 尾字节，用于验证数据包
		ByteType Tail{0};
	};
#pragma pack(pop)

}
