#include <Cango/ByteCommunication/BoostImplementations.hpp>
#include <spdlog/spdlog.h>

using namespace Cango;
using namespace std::chrono_literals;

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
	const auto default_logger_user = spdlog::default_logger();
	EasyBoostTCPSocketRWerCommunicationTaskCheatsheet<MessageType, MessageType> cheatsheet{};
	{
		{
			auto&& config = cheatsheet.Provider->Configure();
			const auto actors = config.Actors;
			actors.ClientLogger = default_logger_user;
			actors.Logger = default_logger_user;

			const auto options = config.Options;
			options.LocalEndpoint = LocalEndpoint;
		}
		{
			auto&& config = cheatsheet.Task.Configure();
			const auto options = config.Options;
			options.ReaderMinInterval = 1ms;
			options.WriterMinInterval = 5ms;
		}
	}

	auto& task = cheatsheet.Task;
	auto& utils = cheatsheet.Utils;

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
