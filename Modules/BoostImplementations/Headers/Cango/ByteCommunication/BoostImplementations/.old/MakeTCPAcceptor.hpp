#pragma once

#include <Cango/Constrained.hpp>

#include "TCPSocketMakeParameters.hpp"

namespace Cango:: inline Runtimes :: inline Networks {
	ObjectOwner<boost::asio::ip::tcp::acceptor> MakeTCPAcceptor(
		boost::asio::io_context& context,
		const boost::asio::ip::tcp::endpoint& endpoint) {
		auto acceptor = std::make_shared<tcp_t::acceptor>(context);
		if (error_code_t result{}; acceptor->open(endpoint.protocol(), result).failed())
		{
			spdlog::error("{}> failed to open acceptor: {}", TCPAcceptorWrapper::Name, result.what());
			return nullptr;
		}

		if (error_code_t result{}; acceptor->bind(endpoint, result).failed())
		{
			spdlog::error("{}> failed to bind acceptor: {}", TCPAcceptorWrapper::Name, result.what());
			return nullptr;
		}

		if (error_code_t result{};
			acceptor->listen(socket_base_t::max_listen_connections, result).failed())
		{
			spdlog::error("{}> failed to listen acceptor: {}", TCPAcceptorWrapper::Name, result.what());
			return nullptr;
		}

		return acceptor;
	}
}
