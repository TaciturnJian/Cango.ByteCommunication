#pragma once

#include "ByteTypes.hpp"

namespace Cango:: inline ByteCommunication :: inline Core {
	/// @brief @c Reader 的概念
	template <typename TObject>
	concept IsReader = requires(TObject& object, ByteSpan buffer) {
		{ object.ReadBytes(buffer) } -> std::same_as<SizeType>;
	};

	/// @brief @c Writer 的概念
	template <typename TObject>
	concept IsWriter = requires(TObject& object, CByteSpan buffer) {
		{ object.WriteBytes(buffer) } -> std::same_as<SizeType>;
	};

	/// @brief @c RWer 的概念
	template <typename TObject>
	concept IsRWer = IsReader<TObject> && IsWriter<TObject>;

	/// @brief 运行时确定的字节读取器
	struct RuntimeReader {
		/// @brief 读取字节，直到无剩余内容或者提供的缓冲区已满
		///	@return 实际读取的字节数，如果小于缓冲区长度，则可以认为遇到了一些错误，如 EOF 或者读取器不再可用
		[[nodiscard]] virtual SizeType ReadBytes(ByteSpan buffer) = 0;

		virtual ~RuntimeReader() = default;
	};

	/// @brief 运行时确定的字节写入器
	struct RuntimeWriter {
		/// @brief 写入字节，直到写入任意字节错误或者提供的缓冲区已经全部写入
		///	@return 实际写入的字节数，如果小于缓冲区长度，则可以认为遇到了一些错误，如写入器不再可用
		[[nodiscard]] virtual SizeType WriteBytes(CByteSpan buffer) = 0;

		virtual ~RuntimeWriter() = default;
	};

	/// @brief Reader and Writer 代表支持输入输出的设备
	struct RuntimeRWer : RuntimeReader, RuntimeWriter {
		~RuntimeRWer() override = default;
	};
}
