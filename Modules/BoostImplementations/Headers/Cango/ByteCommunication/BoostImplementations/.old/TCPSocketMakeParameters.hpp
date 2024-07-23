#pragma once

#include <vector>
#include <boost/asio/ip/tcp.hpp>

namespace Cango:: inline Runtimes :: inline Networks {
	struct TCPSocketMakeParameters {
		std::vector<boost::asio::ip::tcp::endpoint> LocalEndpoints{};
		std::vector<boost::asio::ip::tcp::endpoint> RemoteEndpoints{};

		std::vector<boost::asio::ip::tcp> Protocols{
			boost::asio::ip::tcp::v4(),
			boost::asio::ip::tcp::v6()
		};

		std::ostream& Format(std::ostream& stream) const {
			if (RemoteEndpoints.empty()) return stream;
			if (LocalEndpoints.empty()) {
				if (Protocols.empty()) return stream;

				const auto v4_type = boost::asio::ip::tcp::v4().type();
				for (const auto& protocol : Protocols) stream << (protocol.type() == v4_type ? "v4" : "v6") << '|';
			}
			else for (const auto& endpoint : LocalEndpoints) stream << endpoint << '|';
			stream << " -> ";
			for (const auto& endpoint : RemoteEndpoints)
				stream << endpoint << '|';
			return stream;
		}
	};
}
