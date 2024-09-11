#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/serial_port.hpp>
#include <Cango/CommonUtils/ObjectOwnership.hpp>
#include <spdlog/logger.h>

#include "BoostReadWrite.hpp"

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <typename TBoostDevice>
	struct BoostRWer final {
		Owner<TBoostDevice> DeviceOwner;
		ObjectUser<spdlog::logger> Logger;

		BoostRWer(Owner<TBoostDevice>& deviceOwner, const ObjectUser<spdlog::logger>& logger) :
			DeviceOwner(std::move(deviceOwner)),
			Logger(logger) {
		}

		/// @brief 使用 boost 提供的函数读取字节
		///	@param buffer 提供给写入读取到的字节的缓冲区
		///	@return 读取到的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t ReadBytes(const ByteSpan buffer) noexcept {
			boost::system::error_code result{};
			const auto bytes = Cango::ReadBytes(*DeviceOwner, buffer, result);
			if (result.failed() && Logger) {
				Logger->error("读取字节失败({}/{}): {}", bytes, buffer.size(), result.what());
			}

			return bytes;
		}

		/// @brief 使用 boost 提供的函数写入字节
		///	@param buffer 提供要写入的字节的缓冲区
		///	@return 写入的字节数
		///	@warning 此函数不检查 Device 是否指向正确对象，如果 Device 为 nullptr，将会引起段错误
		[[nodiscard]] std::size_t WriteBytes(const CByteSpan buffer) noexcept {
			boost::system::error_code result{};
			const auto bytes = Cango::WriteBytes(*DeviceOwner, buffer, result);
			if (result.failed() && Logger) // 因为 result.failed() 是 constexpr ，所以放在前面
				Logger->error("写入字节失败({}/{}): {}", bytes, buffer.size(), result.what());
			return bytes;
		}
	};

	using SerialPortRWer = BoostRWer<boost::asio::serial_port>;
	using TCPSocketRWer = BoostRWer<boost::asio::ip::tcp::socket>;
	using UDPSocketRWer = BoostRWer<boost::asio::ip::udp::socket>;
}
