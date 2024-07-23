#pragma once

#include <Cango/ByteCommunication/ExtensionDevices.hpp>

#include "MakeSerialPort.hpp"
#include "SerialPortWrapper.hpp"

namespace Cango:: inline Runtimes :: inline Networks {
	struct SerialPortMaker final : Acceptor<SerialPortWrapper>, NamedModule {
		boost::asio::io_context IOContext{};

		static constexpr char Name[] = "SerialPortMaker";

		SerialPortMakeParameters Parameters{};

		[[nodiscard]] bool GetItem(SerialPortWrapper& item) noexcept override {
			item.Parameters.BaudRate = Parameters.BaudRate;
			item.Device = MakeSerialPort(IOContext, Parameters, item.Parameters.DeviceName);
			return item.Device != nullptr;
		}

		std::ostream& GetName(std::ostream& stream) const noexcept override {
			return stream << Name;
		}

		std::ostream& GetBrief(std::ostream& stream) const noexcept override {
			return Parameters.Format(stream);
		}
	};
}
