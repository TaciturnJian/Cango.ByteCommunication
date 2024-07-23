#include <Cango/ByteCommunication/BoostImplementations.hpp>
#include <Cango/ByteCommunication/Core.hpp>
#include <Cango/CommonUtils/AsyncItemPool.hpp>
#include <iostream>

using namespace Cango;

namespace {
	const boost::asio::ip::udp::endpoint LocalEndpoint{
		boost::asio::ip::make_address("127.0.0.1"), 8989
	};
	const boost::asio::ip::udp::endpoint RemoteEndpoint{
		boost::asio::ip::make_address("127.0.0.1"), 8990
	};

	struct MessageType {
		std::uint8_t Head{'!'};
		std::array<std::uint8_t, 8> Data{};
		std::uint8_t Tail{0};

		std::ostream& Format(std::ostream& stream) const noexcept {
			for (const auto byte : Data) stream << static_cast<int>(byte) << ' ';
			return stream;
		}
	};
}

int main() {
	const auto socket_io_context_owner = std::make_shared<boost::asio::io_context>();
	const auto default_logger_user = spdlog::default_logger();
	const auto provider = std::make_shared<CangoUDPSocketRWerProvider>();
	{
		auto&& config = provider->Configure();
		config.Actors.IOContext = socket_io_context_owner;
		config.Actors.RWerLogger = default_logger_user;
		config.Actors.Logger = default_logger_user;
		config.Options.LocalEndpoint = LocalEndpoint;
		config.Options.RemoteEndpoint = RemoteEndpoint;
	}

	EasyCommunicationTaskPoolsAndMonitors<MessageType, MessageType> pools_and_monitors{};

	EasyCommunicationTask<CangoUDPSocketRWerProvider, MessageType, MessageType> task{};
	{
		pools_and_monitors.Apply(task);
		auto&& config = task.Configure();
		config.Actors.Provider = provider;
		auto& options = config.Options;
		options.ReaderMinInterval = std::chrono::milliseconds{1};
		options.WriterMinInterval = std::chrono::milliseconds{5};
	}

	if (!task.IsFunctional()) return 1;

	ThreadList threads{};
	threads << task;
	threads << [&pools_and_monitors] {
		auto& reader_pool = *pools_and_monitors.ReaderMessagePool;
		auto& writer_pool = *pools_and_monitors.WriterMessagePool;
		auto& reader_monitor = *pools_and_monitors.ReaderMonitor;
		auto& writer_monitor = *pools_and_monitors.WriterMonitor;
		auto& provider_monitor = *pools_and_monitors.ProviderMonitor;

		MessageType message{};
		IntervalSleeper sleeper{std::chrono::milliseconds{100}};

		for (int i = 0; i < 10; i++) {
			while (!reader_pool.GetItem(message)) 
				sleeper.Sleep();
			spdlog::info("Reader> {}", Format(message));
			writer_pool.SetItem(message);
		}

		reader_monitor.Interrupt();
		writer_monitor.Interrupt();
		provider_monitor.Interrupt();
	};
	JoinThreads(threads);

	return 0;
}
