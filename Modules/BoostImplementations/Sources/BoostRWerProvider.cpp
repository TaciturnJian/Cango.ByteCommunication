#include <Cango/ByteCommunication/BoostImplementations/BoostRWerProvider.hpp>

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	bool CangoSerialPortRWerProvider::
	TryOpen(boost::asio::serial_port& device, const std::string& port) const noexcept {
		if (boost::system::error_code result{}; device.open(port, result).failed()) {
			if (Logger != nullptr) Logger->error("无法打开设备({}): {}", port, result.what());
			return false;
		}
		return true;
	}

	template <typename TOption>
	bool CangoSerialPortRWerProvider::
	TryApply(boost::asio::serial_port& device, const TOption& option) const noexcept {
		if (boost::system::error_code result{}; device.set_option(option, result).failed()) {
			if (Logger != nullptr) Logger->error("无法应用配置: {}", result.what());
			return false;
		}
		return true;
	}

	bool CangoSerialPortRWerProvider::TryApplyOptions(boost::asio::serial_port& device) const noexcept {
		return TryApply(device, BaudRate)
			&& TryApply(device, FlowControl)
			&& TryApply(device, Parity)
			&& TryApply(device, StopBits)
			&& TryApply(device, CharacterSize);
	}

	CangoSerialPortRWerProvider::Configurations CangoSerialPortRWerProvider::Configure() noexcept {
		return Configurations{
			.Actors = {IOContext, Logger, RWerLogger},
			.Options = {Ports, BaudRate, FlowControl, Parity, StopBits, CharacterSize}
		};
	}

	bool CangoSerialPortRWerProvider::IsFunctional() const noexcept { return ValidateAll(IOContext); }

	bool CangoSerialPortRWerProvider::GetItem(ObjectOwner<SerialPortRWer>& sp) noexcept {
		auto [io_context_user, io_context] = Acquire(IOContext);

		for (const auto& port : Ports) {
			auto new_boost_sp = std::make_shared<boost::asio::serial_port>(io_context);
			if (!TryOpen(*new_boost_sp, port) || !TryApplyOptions(*new_boost_sp)) continue;
			sp = std::make_shared<SerialPortRWer>(std::move(new_boost_sp), RWerLogger);
			return true;
		}

		return false;
	}

	bool CangoTCPSocketRWerProvider::TryOpen(boost::asio::ip::tcp::socket& device) const {
		if (boost::system::error_code result{}; device.open(LocalEndpoint.protocol(), result).failed()) {
			if (Logger != nullptr) Logger->error("无法打开设备: {}", result.what());
			return false;
		}
		return true;
	}

	bool CangoTCPSocketRWerProvider::TryBind(boost::asio::ip::tcp::socket& device) const noexcept {
		if (boost::system::error_code result{}; device.bind(LocalEndpoint, result).failed()) {
			if (Logger != nullptr)
				Logger->error(
					"无法绑定到地址({}:{}): {}",
					LocalEndpoint.address().to_string(),
					LocalEndpoint.port(),
					result.what()
				);
			return false;
		}
		return true;
	}

	bool CangoTCPSocketRWerProvider::TryConnect(boost::asio::ip::tcp::socket& device) const noexcept {
		if (boost::system::error_code result{}; device.connect(RemoteEndpoint, result).failed()) {
			if (Logger != nullptr)
				Logger->error(
					"无法连接到地址({}:{}): {}",
					RemoteEndpoint.address().to_string(),
					RemoteEndpoint.port(),
					result.what()
				);
			return false;
		}
		return true;
	}

	CangoTCPSocketRWerProvider::Configurations CangoTCPSocketRWerProvider::Configure() noexcept {
		return Configurations{
			.Actors = {IOContext, Logger, RWerLogger},
			.Options = {LocalEndpoint, RemoteEndpoint}
		};
	}

	bool CangoTCPSocketRWerProvider::IsFunctional() const noexcept { return ValidateAll(IOContext); }

	bool CangoTCPSocketRWerProvider::GetItem(ObjectOwner<TCPSocketRWer>& socket) noexcept {
		auto [io_context_user, io_context] = Acquire(IOContext);
		auto new_socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
		if (!TryOpen(*new_socket) || !TryBind(*new_socket) || !TryConnect(*new_socket)) return false;
		socket = std::make_shared<TCPSocketRWer>(std::move(new_socket), RWerLogger);
		return true;
	}

	bool BoostTCPSocketRWerProvider::RefreshAcceptor() noexcept {
		if (IsListening) return true;
		if (boost::system::error_code result{}; Acceptor.open(LocalEndpoint.protocol(), result).failed()) {
			if (Logger != nullptr) Logger->error("无法打开侦听器: {}", result.what());
			return false;
		}
		if (boost::system::error_code result{};
			Acceptor.bind(LocalEndpoint, result).failed()) {
			if (Logger != nullptr)
				Logger->error(
					"无法绑定到地址({}:{}): {}",
					LocalEndpoint.address().to_string(),
					LocalEndpoint.port(),
					result.what());
			return false;
		}
		if (boost::system::error_code result{};
			Acceptor.listen(boost::asio::socket_base::max_listen_connections, result).failed()) {
			if (Logger != nullptr) Logger->error("无法开始侦听: {}", result.what());
			return false;
		}
		IsListening = true;
		return true;
	}

	BoostTCPSocketRWerProvider::Configurations BoostTCPSocketRWerProvider::Configure() noexcept {
		return {
			.Actors = {IOContext, Logger, ClientLogger},
			.Options = {LocalEndpoint}
		};
	}

	bool BoostTCPSocketRWerProvider::IsFunctional() const noexcept { return ValidateAll(IOContext); }

	bool BoostTCPSocketRWerProvider::GetItem(ObjectOwner<TCPSocketRWer>& socket) {
		if (!RefreshAcceptor()) return false;

		auto [io_context_user, io_context] = Acquire(IOContext);
		auto new_socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);

		if (boost::system::error_code result; Acceptor.accept(*new_socket, result).failed()) {
			if (Logger != nullptr) Logger->error("无法接受连接: {}", result.what());
			return false;
		}
		socket = std::make_shared<TCPSocketRWer>(std::move(new_socket), ClientLogger);
		return true;
	}

	bool CangoUDPSocketRWerProvider::TryOpen(boost::asio::ip::udp::socket& device) const noexcept {
		if (boost::system::error_code result{}; device.open(LocalEndpoint.protocol(), result).failed()) {
			if (Logger != nullptr) Logger->error("无法打开设备: {}", result.what());
			return false;
		}
		return true;
	}

	bool CangoUDPSocketRWerProvider::TryBind(boost::asio::ip::udp::socket& device) const noexcept {
		if (boost::system::error_code result{}; device.bind(LocalEndpoint, result).failed()) {
			if (Logger != nullptr)
				Logger->error(
					"无法绑定到地址({}:{}): {}",
					LocalEndpoint.address().to_string(),
					LocalEndpoint.port(),
					result.what());
			return false;
		}
		return true;
	}

	bool CangoUDPSocketRWerProvider::TryConnect(boost::asio::ip::udp::socket& device) const noexcept {
		if (boost::system::error_code result{}; device.connect(RemoteEndpoint, result).failed()) {
			if (Logger != nullptr)
				Logger->error(
					"无法连接到地址({}:{}): {}",
					RemoteEndpoint.address().to_string(),
					RemoteEndpoint.port(),
					result.what());
			return false;
		}
		return true;
	}

	CangoUDPSocketRWerProvider::Configurations CangoUDPSocketRWerProvider::Configure() noexcept {
		return Configurations{
			.Actors = {IOContext, Logger, RWerLogger},
			.Options = {LocalEndpoint, RemoteEndpoint}
		};
	}

	bool CangoUDPSocketRWerProvider::IsFunctional() const noexcept { return ValidateAll(IOContext); }

	bool CangoUDPSocketRWerProvider::GetItem(ObjectOwner<UDPSocketRWer>& socket) noexcept {
		auto [io_context_user, io_context] = Acquire(IOContext);
		auto new_socket = std::make_shared<boost::asio::ip::udp::socket>(io_context);
		if (!TryOpen(*new_socket) || !TryBind(*new_socket) || !TryConnect(*new_socket)) return false;
		socket = std::make_shared<UDPSocketRWer>(std::move(new_socket), RWerLogger);
		return true;
	}
}