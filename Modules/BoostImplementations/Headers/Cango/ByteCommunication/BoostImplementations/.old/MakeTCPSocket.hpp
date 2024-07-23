#pragma once

#include "TCPSocketMakeParameters.hpp"

namespace Cango:: inline Runtimes :: inline Networks {
	inline ObjectOwner<boost::asio::ip::tcp::socket> MakeTCPSocket(
		boost::asio::io_context& context,
		const TCPSocketMakeParameters& parameters) {
		ObjectOwner<boost::asio::ip::tcp::socket> socket{};

		bool local_succeeded = false;
		if (parameters.LocalEndpoints.empty()) {
			if (parameters.Protocols.empty()) return nullptr;

			for (const auto& protocol : parameters.Protocols) {
				socket = std::make_shared<boost::asio::ip::tcp::socket>(context);

				if (boost::system::error_code result{};
					socket->open(protocol, result).failed()) {
					spdlog::error("TCPSocket> failed to create socket: {}", result.what());
					continue;
				}

				local_succeeded = true;
				break;
			}
		}
		else {
			for (const auto& local_endpoint : parameters.LocalEndpoints) {
				socket = std::make_shared<boost::asio::ip::tcp::socket>(context);

				if (boost::system::error_code result{};
					socket->open(local_endpoint.protocol(), result).failed()) {
					spdlog::error("TCPSocket> failed to open socket: {}", result.what());
					continue;
				}


				if (boost::system::error_code result{};
					socket->bind(local_endpoint, result).failed()) {
					spdlog::error("TCPSocket> failed to bind socket: {}", result.what());
					continue;
				}

				local_succeeded = true;
				break;
			}
		}

		if (!local_succeeded) return nullptr;

		bool remote_succeeded = false;
		for (const auto& remote_endpoint : parameters.RemoteEndpoints) {
			if (boost::system::error_code result{}; socket->connect(remote_endpoint, result).failed()) {
				spdlog::error("TCPSocket> failed to connect socket: {}", result.what());
				continue;
			}
			remote_succeeded = true;
			break;
		}

		if (!remote_succeeded) return nullptr;

		return socket;
	}
}
