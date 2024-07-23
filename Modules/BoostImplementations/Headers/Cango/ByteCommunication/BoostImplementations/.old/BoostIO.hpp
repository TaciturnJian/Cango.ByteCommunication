#pragma once

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <Cango/Constrained/OwnershipObjects.hpp>
#include <Cango/ByteCommunication/ReadWriteObjects.hpp>
#include <spdlog/spdlog.h>

namespace Cango:: inline Runtimes :: inline Networks {
	/// @brief 封装 boost 中的一些设备，让 io 操作更加简单
	///	@tparam TBoostDevice boost 设备类型
	/// @note 2024/5/29 既用模板又继承是不是太抽象了？
	template <typename TBoostDevice>
	struct BoostIO : Reader, Writer {
		/// @brief 指向 boost asio 库提供的设备
		ObjectUser<TBoostDevice> Device{};

		/// @brief 获取名称，用于日志输出
		/// @note 为了不引入 NamedModule 而添加的虚函数，用来替代 ToString(*this) 的效果
		[[nodiscard]] virtual std::string GetName() const noexcept = 0;

		/// @brief 使用 boost 提供的函数读取字节
		///	@param buffer 提供给写入读取到的字节的缓冲区
		///	@return 读取到的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t ReadBytes(const std::span<std::uint8_t> buffer) noexcept final {
			boost::system::error_code result{};
			const auto bytes = boost::asio::read(
				*Device,
				boost::asio::buffer(buffer.data(), buffer.size()),
				result
			);
			if (result.failed())
				spdlog::warn(
					"{}> 读取字节失败({}/{}): {}",
					GetName(), bytes, buffer.size(), result.what()
				);

			return bytes;
		}

		/// @brief 使用 boost 提供的函数写入字节
		///	@param buffer 提供要写入的字节的缓冲区
		///	@return 写入的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t WriteBytes(const std::span<const std::uint8_t> buffer) noexcept final {
			boost::system::error_code result{};
			const auto bytes = boost::asio::write(
				*Device,
				boost::asio::buffer(buffer.data(), buffer.size()),
				result
			);
			if (result.failed())
				spdlog::warn(
					"{}> 写入字节失败({}/{}): {}",
					GetName(), bytes, buffer.size(), result.what());

			return bytes;
		}
	};
}
