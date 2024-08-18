#pragma once

#include <Cango/ByteCommunication/Core/PCer.hpp>

#include "BoostRWer.hpp"

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <
		IsRWerProvider TProvider,
		std::default_initializable TReaderMessage,
		std::default_initializable TWriterMessage>
	struct EasyRWerCommunicationTaskCheatsheet {
		EasyCommunicationTask<TProvider, TReaderMessage, TWriterMessage> Task{};
		EasyCommunicationTaskPoolsAndMonitors<TReaderMessage, TWriterMessage> Utils{};
		Owner<boost::asio::io_context> IOContext{};
		Owner<TProvider> Provider{};

		EasyRWerCommunicationTaskCheatsheet() {
			auto&& provider_config = Provider->Configure();
			provider_config.Actors.IOContext = IOContext;

			Utils.Apply(Task);
			auto&& task_config = Task.Configure();
			{
				const auto actors = task_config.Actors;
				actors.Provider = Provider;
			}
		}
	};

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

		[[nodiscard]] bool TryOpen(boost::asio::serial_port& device, const std::string& port) const noexcept;

		template <typename TOption>
		[[nodiscard]] bool TryApply(boost::asio::serial_port& device, const TOption& option) const noexcept;

		[[nodiscard]] bool TryApplyOptions(boost::asio::serial_port& device) const noexcept;

	public:
		using ItemType = Owner<SerialPortRWer>;

		[[nodiscard]] Configurations Configure() noexcept;

		[[nodiscard]] bool IsFunctional() const noexcept;

		[[nodiscard]] bool GetItem(Owner<SerialPortRWer>& sp) const noexcept;
	};

	template <std::default_initializable TReaderMessage, std::default_initializable TWriterMessage>
	using EasyCangoSerialPortRWerCommunicationTaskCheatsheet = EasyRWerCommunicationTaskCheatsheet<
		CangoSerialPortRWerProvider, TReaderMessage, TWriterMessage>;

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

		[[nodiscard]] bool TryOpen(boost::asio::ip::tcp::socket& device) const;

		[[nodiscard]] bool TryBind(boost::asio::ip::tcp::socket& device) const noexcept;

		[[nodiscard]] bool TryConnect(boost::asio::ip::tcp::socket& device) const noexcept;

	public:
		using ItemType = Owner<TCPSocketRWer>;

		[[nodiscard]] Configurations Configure() noexcept;

		[[nodiscard]] bool IsFunctional() const noexcept;

		[[nodiscard]] bool GetItem(Owner<TCPSocketRWer>& socket) noexcept;
	};

	template <std::default_initializable TReaderMessage, std::default_initializable TWriterMessage>
	using EasyCangoTCPSocketRWerCommunicationTaskCheatsheet = EasyRWerCommunicationTaskCheatsheet<
		CangoTCPSocketRWerProvider, TReaderMessage, TWriterMessage>;

	class BoostTCPSocketRWerProvider {
		bool IsListening{false};
		boost::asio::io_context MyIOContext{};
		boost::asio::ip::tcp::acceptor Acceptor{MyIOContext};
		boost::asio::ip::tcp::endpoint LocalEndpoint{};

		Credential<boost::asio::io_context> IOContext{};
		ObjectUser<spdlog::logger> Logger{};
		ObjectUser<spdlog::logger> ClientLogger{};

		struct Configurations {
			struct ActorsType {
				Credential<boost::asio::io_context>& IOContext;
				ObjectUser<spdlog::logger>& Logger;
				ObjectUser<spdlog::logger>& ClientLogger;
			} Actors;

			struct OptionsType {
				boost::asio::ip::tcp::endpoint& LocalEndpoint;
			} Options;
		};

		[[nodiscard]] bool RefreshAcceptor() noexcept;

	public:
		using ItemType = Owner<TCPSocketRWer>;

		[[nodiscard]] Configurations Configure() noexcept;

		[[nodiscard]] bool IsFunctional() const noexcept;

		[[nodiscard]] bool GetItem(Owner<TCPSocketRWer>& socket);
	};

	template <std::default_initializable TReaderMessage, std::default_initializable TWriterMessage>
	using EasyBoostTCPSocketRWerCommunicationTaskCheatsheet = EasyRWerCommunicationTaskCheatsheet<
		BoostTCPSocketRWerProvider, TReaderMessage, TWriterMessage>;

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

		[[nodiscard]] bool TryOpen(boost::asio::ip::udp::socket& device) const noexcept;

		[[nodiscard]] bool TryBind(boost::asio::ip::udp::socket& device) const noexcept;

		[[nodiscard]] bool TryConnect(boost::asio::ip::udp::socket& device) const noexcept;

	public:
		using ItemType = Owner<UDPSocketRWer>;

		[[nodiscard]] Configurations Configure() noexcept;

		[[nodiscard]] bool IsFunctional() const noexcept;

		[[nodiscard]] bool GetItem(Owner<UDPSocketRWer>& socket) noexcept;
	};

	template <std::default_initializable TReaderMessage, std::default_initializable TWriterMessage>
	using EasyCangoUDPSocketRWerCommunicationTaskCheatsheet = EasyRWerCommunicationTaskCheatsheet<
		CangoUDPSocketRWerProvider, TReaderMessage, TWriterMessage>;
}
