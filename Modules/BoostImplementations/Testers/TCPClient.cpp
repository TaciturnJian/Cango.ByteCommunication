#include <Cango/ByteCommunication/BoostImplementations.hpp>
#include <spdlog/spdlog.h>
#include <Cango/TaskDesign/FormattableObject.hpp>
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

    using EasyClientTask = Cango::EasyCommunicationTask<
        Cango::CangoTCPSocketRWerProvider,
        MessageType,
        MessageType>;


    // struct ClientCheatsheet {
    //     EasyClientTask Task{};
    //     Cango::EasyCommunicationTaskPoolsAndMonitors<MessageType, MessageType> Utils{};
    //     Cango::Owner<Cango::CangoTCPSocketRWerProvider> Provider{};
    //     Cango::Owner<boost::asio::io_context> IOContext{};
    //
    //     ClientCheatsheet() noexcept {
    //         auto&& provider_config = Provider->Configure();
    //         provider_config.Actors.IOContext = IOContext;
    //
    //         Utils.Apply(Task);
    //         auto&& task_config = Task.Configure();
    //         {
    //             const auto actors = task_config.Actors;
    //             actors.Provider = Provider;
    //         }
    //     }
    // };
    using ClientCheatSheet = EasyCangoTCPSocketRWerCommunicationTaskCheatsheet<
        MessageType,
        MessageType>;

};

int main(){

    const auto default_logger = spdlog::default_logger();
    spdlog::set_level(spdlog::level::trace);
    ClientCheatSheet cheatsheet{};
    {
        {
            auto&& config = cheatsheet.Provider->Configure();
            const auto actors = config.Actors;
            actors.RWerLogger = default_logger;
            actors.Logger = default_logger;

            const auto options = config.Options;
            options.LocalEndpoint = boost::asio::ip::tcp::endpoint{
                boost::asio::ip::address_v4::from_string("127.0.0.1"), 8992};
            options.RemoteEndpoint = boost::asio::ip::tcp::endpoint{
                boost::asio::ip::address_v4::from_string("127.0.0.1"), 8989};
        }
        {
            auto&& config = cheatsheet.Task.Configure();
            const auto options = config.Options;
            options.ReaderMinInterval = 1ms;
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
    threads << [&utils]
    {
        auto& reader_pool = *utils.ReaderMessagePool;
        auto& writer_pool = *utils.WriterMessagePool;
        auto& reader_monitor = *utils.ReaderMonitor;
        auto& writer_monitor = *utils.WriterMonitor;
        auto& provider_monitor = *utils.ProviderMonitor;

        MessageType message{};
        {
            message.Data.fill(1);
        }
        IntervalSleeper sleeper{std::chrono::milliseconds{50}};

        // 客户端持续往设备写入消息，同时读取客户端的消息并打印出来
        writer_pool.SetItem(message);

        MessageType messageRecivedFromServer{};
        while (!reader_monitor.IsDone()) {
            writer_pool.SetItem(message);
            sleeper.Sleep();
        }

    };

    JoinThreads(threads);
}