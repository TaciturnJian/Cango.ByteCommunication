#pragma once

#include <Cango/Constrained.hpp>

#include <boost/asio/ip/tcp.hpp>

namespace LangYa:: inline Network :: inline TCPSocketImplementation {

	class TCPSocketMaker final :
		public SourceNode<TCPSocketWrapper>,
		public NamedModule {
		boost::asio::io_context IOContext{};

	public:
		static constexpr char Name[] = "TCPSocketMaker";

		TCPSocketMakeParameters Parameters{};

		std::ostream& GetName(std::ostream& stream) const noexcept override;

		std::ostream& GetBrief(std::ostream& stream) const noexcept override;

		[[nodiscard]] bool GetItem(TCPSocketWrapper& item) noexcept override;
	};

	struct TCPAcceptorWrapper final : SourceNode<TCPSocketWrapper>, NamedModule {
		static std::ostream& GetLocalEndpoint(std::ostream& stream, const boost::asio::ip::tcp::acceptor& acceptor);

		static constexpr char Name[] = "TCPAcceptor";

		Dependency<boost::asio::ip::tcp::acceptor> Device{};

		std::ostream& GetName(std::ostream& stream) const noexcept override;

		std::ostream& GetBrief(std::ostream& stream) const noexcept override;

		[[nodiscard]] bool GetItem(TCPSocketWrapper& item) noexcept override;
	};

}
