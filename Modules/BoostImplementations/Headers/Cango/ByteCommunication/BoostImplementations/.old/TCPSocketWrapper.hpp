#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <Cango/Constrained/FormattableObjects/Format.hpp>

#include "BoostIO.hpp"

namespace Cango:: inline ByteCommunication :: inline BoostImplementations {
	/// @brief 串口封装，使用更简单
	///	只提供了串口的读、写、打开、关闭基本操作和一些便于日志的函数
	struct TCPSocketWrapper final : BoostIO<boost::asio::ip::tcp::socket>, NamedModule {
		static std::ostream& GetLocalEndpoint(
			std::ostream& stream,
			const boost::asio::ip::tcp::socket& socket) noexcept {
			boost::system::error_code result{};
			const auto local_endpoint = socket.local_endpoint(result);
			if (!result.failed()) return stream << local_endpoint;
			spdlog::warn("{}> failed to get local endpoint: {}", Name, result.what());
			return stream << "local";
		}

		static std::ostream& GetRemoteEndpoint(
			std::ostream& stream,
			const boost::asio::ip::tcp::socket& socket) noexcept {
			boost::system::error_code result{};
			const auto remote_endpoint = socket.remote_endpoint(result);
			if (!result.failed()) return stream << remote_endpoint;
			spdlog::warn("{}> failed to get remote endpoint: {}", Name, result.what());
			return stream << "remote";
		}

		static std::ostream& DefaultFormat(std::ostream& stream, const boost::asio::ip::tcp::socket& socket) {
			GetLocalEndpoint(stream, socket);
			stream << " -> ";
			return GetRemoteEndpoint(stream, socket);
		}

		static constexpr char Name[] = "TCPSocket";

		std::ostream& GetName(std::ostream& stream) const noexcept override { return stream << Name; }

		std::ostream& GetBrief(std::ostream& stream) const noexcept override {
			if (!Device) return stream << "nullptr";
			return DefaultFormat(stream, *Device);
		}

		[[nodiscard]] std::string GetName() const noexcept override { return Cango::Format(*this); }
	};
}
