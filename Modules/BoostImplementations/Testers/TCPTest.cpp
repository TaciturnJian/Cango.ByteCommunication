#include <Cango/ByteCommunication/BoostImplementations.hpp>
#include <spdlog/spdlog.h>
#include <fmt/ostream.h>

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

		friend std::ostream& operator<<(std::ostream& stream, const MessageType& object) noexcept {
			for (const auto byte : object.Data) stream << static_cast<int>(byte) << ' ';
			return stream;
		}
	};
}

template<>
struct fmt::formatter<MessageType> : ostream_formatter {};

/* 测试脚本
(echo -n -e '!01234567\0'; sleep 2)|telnet 127.0.0.1 8989

*/

int main() {
	spdlog::set_level(spdlog::level::debug);

	const ObjectUser default_logger_user{spdlog::default_logger()};
	EasyBoostTCPSocketRWerCommunicationTaskCheatsheet<MessageType, MessageType> cheatsheet{};
	{
		{
			auto&& [actors, options] = cheatsheet.Provider->Configure();
			actors.ClientLogger = default_logger_user;
			actors.Logger = default_logger_user;
			options.LocalEndpoint = LocalEndpoint;
		}
		{
			auto&& [actors, options] = cheatsheet.Task.Configure();

			// 屏蔽异常处理
			const auto monitor = actors.WriterMonitor.lock();
			monitor->ExceptionHandler = []{};
			monitor->NormalHandler = []{};

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
			spdlog::info("Reader> {}", message);
			writer_pool.SetItem(message);
		}

		reader_monitor.Interrupt();
		writer_monitor.Interrupt();
		provider_monitor.Interrupt();
	};
	JoinThreads(threads);

	return 0;
}
