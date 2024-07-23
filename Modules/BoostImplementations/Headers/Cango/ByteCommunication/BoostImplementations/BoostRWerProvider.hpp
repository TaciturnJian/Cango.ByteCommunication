#pragma once

#include <Cango/ByteCommunication/Core/PCer.hpp>

#include "BoostRWer.hpp"

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <typename TProvider, typename TRWer = typename TProvider::ItemType::element_type>
	concept IsSerialPortRWerProvider = IsRWerProvider<TProvider> && std::same_as<SerialPortRWer, TRWer>;

	class CangoSerialPortRWerProvider {
		Credential<boost::asio::io_context> IOContext{};
		ObjectUser<spdlog::logger> Logger{};
		ObjectUser<spdlog::logger> RWerLogger{};

		using baud_rate_type = boost::asio::serial_port::baud_rate;
		using flow_control_type = boost::asio::serial_port::flow_control;
		using parity_type = boost::asio::serial_port::parity;
		using stop_bits_type = boost::asio::serial_port::stop_bits;
		using character_size_type = boost::asio::serial_port::character_size;

		std::list<std::string> Ports{};
		baud_rate_type BaudRate{115200};
		flow_control_type FlowControl{flow_control_type::none};
		parity_type Parity{parity_type::none};
		stop_bits_type StopBits{stop_bits_type::one};
		character_size_type CharacterSize{};

		struct Configurations {
			struct ActorsType {
				Credential<boost::asio::io_context>& IOContext;
				ObjectUser<spdlog::logger>& Logger;
				ObjectUser<spdlog::logger>& RWerLogger;
			} Actors;

			struct OptionsType {
				std::list<std::string>& Ports;
				baud_rate_type& BaudRate;
				flow_control_type& FlowControl;
				parity_type& Parity;
				stop_bits_type& StopBits;
				character_size_type& CharacterSize;
			} Options;
		};

		[[nodiscard]] bool TryOpen(boost::asio::serial_port& device, const std::string& port) const noexcept {
			if (boost::system::error_code result{}; device.open(port, result).failed()) {
				if (Logger != nullptr) Logger->error("无法打开设备({}): {}", port, result.what());
				return false;
			}
			return true;
		}

		template <typename TOption>
		[[nodiscard]] bool TryApply(boost::asio::serial_port& device, const TOption& option) const noexcept {
			if (boost::system::error_code result{}; device.set_option(option, result).failed()) {
				if (Logger != nullptr) Logger->error("无法应用配置: {}", result.what());
				return false;
			}
			return true;
		}

		[[nodiscard]] bool TryApplyOptions(boost::asio::serial_port& device) const noexcept {
			return TryApply(device, BaudRate)
				&& TryApply(device, FlowControl)
				&& TryApply(device, Parity)
				&& TryApply(device, StopBits)
				&& TryApply(device, CharacterSize);
		}

	public:
		using ItemType = ObjectOwner<SerialPortRWer>;

		[[nodiscard]] Configurations Configure() noexcept {
			return Configurations{
				.Actors = {IOContext, Logger, RWerLogger},
				.Options = {Ports, BaudRate, FlowControl, Parity, StopBits, CharacterSize}
			};
		}

		[[nodiscard]] bool IsFunctional() const noexcept { return ValidateAll(IOContext); }

		[[nodiscard]] bool GetItem(ObjectOwner<SerialPortRWer>& sp) noexcept {
			auto [io_context_user, io_context] = Acquire(IOContext);

			for (const auto& port : Ports) {
				auto new_boost_sp = std::make_shared<boost::asio::serial_port>(io_context);
				if (!TryOpen(*new_boost_sp, port) || !TryApplyOptions(*new_boost_sp)) continue;
				sp = std::make_shared<SerialPortRWer>(std::move(new_boost_sp), RWerLogger);
				return true;
			}

			return false;
		}
	};

	template <typename TProvider, typename TRWer = typename TProvider::ItemType::element_type>
	concept IsTCPSocketRWerProvider = IsRWerProvider<TProvider> && std::same_as<TCPSocketRWer, TRWer>;

	class CangoTCPSocketRWerProvider {
		Credential<boost::asio::io_context> IOContext{};
		ObjectUser<spdlog::logger> Logger{};
		ObjectUser<spdlog::logger> RWerLogger{};

		boost::asio::ip::tcp::endpoint LocalEndpoint{};
		boost::asio::ip::tcp::endpoint RemoteEndpoint{};

		struct Configurations {
			struct ActorsType {
				Credential<boost::asio::io_context>& IOContext;
				ObjectUser<spdlog::logger>& Logger;
				ObjectUser<spdlog::logger>& RWerLogger;
			} Actors;

			struct OptionsType {
				boost::asio::ip::tcp::endpoint& LocalEndpoint;
				boost::asio::ip::tcp::endpoint& RemoteEndpoint;
			} Options;
		};

		[[nodiscard]] bool TryOpen(boost::asio::ip::tcp::socket& device) const {
			if (boost::system::error_code result{}; device.open(LocalEndpoint.protocol(), result).failed()) {
				if (Logger != nullptr) Logger->error("无法打开设备: {}", result.what());
				return false;
			}
			return true;
		}

		[[nodiscard]] bool TryBind(boost::asio::ip::tcp::socket& device) const noexcept {
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

		[[nodiscard]] bool TryConnect(boost::asio::ip::tcp::socket& device) const noexcept {
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

	public:
		using ItemType = ObjectOwner<TCPSocketRWer>;

		[[nodiscard]] Configurations Configure() noexcept {
			return Configurations{
				.Actors = {IOContext, Logger, RWerLogger},
				.Options = {LocalEndpoint, RemoteEndpoint}
			};
		}

		[[nodiscard]] bool IsFunctional() const noexcept { return ValidateAll(IOContext); }

		[[nodiscard]] bool GetItem(ObjectOwner<TCPSocketRWer>& socket) noexcept {
			auto [io_context_user, io_context] = Acquire(IOContext);
			auto new_socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
			if (!TryOpen(*new_socket) || !TryBind(*new_socket) || !TryConnect(*new_socket)) return false;
			socket = std::make_shared<TCPSocketRWer>(std::move(new_socket), RWerLogger);
			return true;
		}
	};

	template <typename TProvider, typename TRWer = typename TProvider::ItemType::element_type>
	concept IsUDPSocketRWerProvider = IsRWerProvider<TProvider> && std::same_as<UDPSocketRWer, TRWer>;

	class CangoUDPSocketRWerProvider {
		Credential<boost::asio::io_context> IOContext{};
		ObjectUser<spdlog::logger> Logger{};
		ObjectUser<spdlog::logger> RWerLogger{};

		boost::asio::ip::udp::endpoint LocalEndpoint{};
		boost::asio::ip::udp::endpoint RemoteEndpoint{};

		struct Configurations {
			struct ActorsType {
				Credential<boost::asio::io_context>& IOContext;
				ObjectUser<spdlog::logger>& Logger;
				ObjectUser<spdlog::logger>& RWerLogger;
			} Actors;

			struct OptionsType {
				boost::asio::ip::udp::endpoint& LocalEndpoint;
				boost::asio::ip::udp::endpoint& RemoteEndpoint;
			} Options;
		};

		[[nodiscard]] bool TryOpen(boost::asio::ip::udp::socket& device) const {
			if (boost::system::error_code result{}; device.open(LocalEndpoint.protocol(), result).failed()) {
				if (Logger != nullptr) Logger->error("无法打开设备: {}", result.what());
				return false;
			}
			return true;
		}

		[[nodiscard]] bool TryBind(boost::asio::ip::udp::socket& device) const noexcept {
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

		[[nodiscard]] bool TryConnect(boost::asio::ip::udp::socket& device) const noexcept {
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

	public:
		using ItemType = ObjectOwner<UDPSocketRWer>;

		[[nodiscard]] Configurations Configure() noexcept {
			return Configurations{
				.Actors = {IOContext, Logger, RWerLogger},
				.Options = {LocalEndpoint, RemoteEndpoint}
			};
		}

		[[nodiscard]] bool IsFunctional() const noexcept { return ValidateAll(IOContext); }

		[[nodiscard]] bool GetItem(ObjectOwner<UDPSocketRWer>& socket) noexcept {
			auto [io_context_user, io_context] = Acquire(IOContext);
			auto new_socket = std::make_shared<boost::asio::ip::udp::socket>(io_context);
			if (!TryOpen(*new_socket) || !TryBind(*new_socket) || !TryConnect(*new_socket)) return false;
			socket = std::make_shared<UDPSocketRWer>(std::move(new_socket), RWerLogger);
			return true;
		}
	};
}
