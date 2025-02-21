#include <Cango/ByteCommunication/BoostImplementations.hpp>
#include <spdlog/spdlog.h>
#include <Cango/TaskDesign/FormattableObject.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

using namespace Cango;
using namespace std::chrono_literals;

namespace{
    struct MessageType {
        std::uint8_t Head{'!'};
        std::array<std::uint8_t, 8> Data{};
        std::uint8_t Tail{0};

        std::ostream& Format(std::ostream& stream) const noexcept {
            for (const auto byte : Data) stream << static_cast<int>(byte) << ' ';
            return stream;
        }
    };


    using EasyServerTask = Cango::EasyCommunicationTask<
        Cango::BoostTCPSocketRWerProvider,
        MessageType,
        MessageType>;

    using ServerCheatsheet = Cango::EasyBoostTCPSocketRWerCommunicationTaskCheatsheet<
        MessageType,
        MessageType>;
};

int main(){
//  const auto defualt_logger_user = ObjectUser{spdlog::default_logger()};
//    const ObjectUser default_logger_user{spdlog::default_logger()};
    spdlog::set_level(spdlog::level::trace);
const auto default_logger = spdlog::default_logger();
    ServerCheatsheet cheatsheet{};
    {
        {
        auto&& config = cheatsheet.Provider->Configure();
        const auto actors = config.Actors;
        actors.ClientLogger = default_logger;
        actors.Logger = default_logger;

        const auto options = config.Options;
        options.LocalEndpoint = boost::asio::ip::tcp::endpoint{
            boost::asio::ip::address_v4::from_string("127.0.0.1"), 8989};
        }
        {
        auto&& config = cheatsheet.Task.Configure();
        const auto options = config.Options;
//        options.ProviderMinInterval = 3s;
        options.ReaderMinInterval = 10ms;
        options.WriterMinInterval = 1ms;
        }
        {
            const auto& utils = cheatsheet.Utils;
            const auto& reader_config = utils.ReaderMonitor;
            reader_config->Counter.MaxCount = 10;

            const auto& writer_config = utils.WriterMonitor;
            writer_config->NormalHandler = EasyDeliveryTaskMonitor::EmptyHandler;
            writer_config->ExceptionHandler = EasyDeliveryTaskMonitor::EmptyHandler;

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
        MessageType sendMessage{};
        {
            sendMessage.Data = {1, 2, 3, 4, 5, 6, 7, 8};
        }
        IntervalSleeper sleeper{std::chrono::milliseconds{100}};


        while (!reader_monitor.IsDone()) {
            if (reader_pool.GetItem(message)) {
                spdlog::info("接收到客户端消息: {}");
                message.Format(std::cout);

            }
            writer_pool.SetItem(sendMessage);
            sleeper.Sleep();
        }
    };

    JoinThreads(threads);

    return 0;

}