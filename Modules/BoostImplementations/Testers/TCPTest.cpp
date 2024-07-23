#include <Cango/ByteCommunication/BoostImplementations.hpp>

using namespace Cango;

namespace {
	const boost::asio::ip::tcp::endpoint LocalEndpoint{
		boost::asio::ip::make_address("127.0.0.1"), 8989
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
	const auto provider = std::make_shared<BoostTCPSocketRWerProvider>();
	{
		auto&& config = provider->Configure();
		config.Actors.ClientIOContext = socket_io_context_owner;
		config.Actors.ClientLogger = default_logger_user;
		config.Actors.Logger = default_logger_user;
		config.Options.LocalEndpoint = LocalEndpoint;
	}

	EasyCommunicationTaskPoolsAndMonitors<MessageType, MessageType> utils{};

	EasyCommunicationTask<BoostTCPSocketRWerProvider, MessageType, MessageType> task{};
	{
		utils.Apply(task);
		auto&& config = task.Configure();
		config.Actors.Provider = provider;
		const auto& options = config.Options;
		options.ReaderMinInterval = std::chrono::milliseconds{1};
		options.WriterMinInterval = std::chrono::milliseconds{5};
	}

	if (!task.IsFunctional()) return 1;

	ThreadList threads{};
	threads << task;
	threads << [&utils] {
		auto& reader_pool = *utils.ReaderMessagePool;
		auto& writer_pool = *utils.WriterMessagePool;
		auto& reader_monitor = *utils.ReaderMonitor;
		auto& writer_monitor = *utils.WriterMonitor;
		auto& provider_monitor = *utils.ProviderMonitor;

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
