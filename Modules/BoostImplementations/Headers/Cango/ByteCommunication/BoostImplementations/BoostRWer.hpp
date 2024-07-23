#pragma once

#include <boost/asio/read.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <Cango/TaskDesign/ItemOwnership.hpp>
#include <spdlog/spdlog.h>

#include "UDPReadWrite.hpp"

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <typename TBoostDevice>
	struct BoostRWer {
		ObjectOwner<TBoostDevice> DeviceOwner{};
		ObjectUser<spdlog::logger> Logger{};

		/// @brief 使用 boost 提供的函数读取字节
		///	@param buffer 提供给写入读取到的字节的缓冲区
		///	@return 读取到的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t ReadBytes(const std::span<std::uint8_t> buffer) noexcept {
			boost::system::error_code result{};
			const auto bytes = boost::asio::read(
				*DeviceOwner,
				boost::asio::buffer(buffer.data(), buffer.size()),
				result
			);
			if (Logger != nullptr && result.failed())
				Logger->error("读取字节失败({}/{}): {}", bytes, buffer.size(), result.what());

			return bytes;
		}

		/// @brief 使用 boost 提供的函数写入字节
		///	@param buffer 提供要写入的字节的缓冲区
		///	@return 写入的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t WriteBytes(const std::span<const std::uint8_t> buffer) noexcept {
			boost::system::error_code result{};
			const auto bytes = boost::asio::write(
				*DeviceOwner,
				boost::asio::buffer(buffer.data(), buffer.size()),
				result
			);
			if (Logger != nullptr && result.failed())
				Logger->error("写入字节失败({}/{}): {}", bytes, buffer.size(), result.what());

			return bytes;
		}
	};

	using SerialPortRWer = BoostRWer<boost::asio::serial_port>;
	using TCPSocketRWer = BoostRWer<boost::asio::ip::tcp::socket>;

	struct UDPSocketRWer final {
		ObjectOwner<boost::asio::ip::udp::socket> DeviceOwner{};
		ObjectUser<spdlog::logger> Logger{};

		/// @brief 使用 boost 提供的函数读取字节
		///	@param buffer 提供给写入读取到的字节的缓冲区
		///	@return 读取到的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t ReadBytes(const std::span<std::uint8_t> buffer) const noexcept {
			boost::system::error_code result{};
			const auto bytes = Cango::ReadBytes(*DeviceOwner, buffer, result);
			if (Logger != nullptr && result.failed())
				Logger->error("读取字节失败({}/{}): {}", bytes, buffer.size(), result.what());

			return bytes;
		}

		/// @brief 使用 boost 提供的函数写入字节
		///	@param buffer 提供要写入的字节的缓冲区
		///	@return 写入的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t WriteBytes(const std::span<const std::uint8_t> buffer) const noexcept {
			boost::system::error_code result{};
			const auto bytes = Cango::WriteBytes(*DeviceOwner, buffer, result);
			if (Logger != nullptr && result.failed())
				Logger->error("写入字节失败({}/{}): {}", bytes, buffer.size(), result.what());

			return bytes;
		}
	};
}