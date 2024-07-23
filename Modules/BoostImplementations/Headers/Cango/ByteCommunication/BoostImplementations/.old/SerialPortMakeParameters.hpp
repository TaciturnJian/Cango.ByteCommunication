#pragma once

#include <boost/asio/serial_port.hpp>

namespace Cango:: inline Runtimes :: inline Networks {
	struct SerialPortMakeParameters {
		std::vector<std::string> DeviceNames{};
		boost::asio::serial_port::baud_rate BaudRate{115200};

		std::ostream& Format(std::ostream& stream) const {
			for (const auto& name : DeviceNames)
				stream << name << '|';
			return stream << '@' << BaudRate.value();
		}
	};
}
