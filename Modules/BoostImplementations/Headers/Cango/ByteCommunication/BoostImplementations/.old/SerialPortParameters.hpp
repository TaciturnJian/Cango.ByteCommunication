#pragma once

#include <ostream>
#include <boost/asio/serial_port.hpp>

namespace Cango:: inline Runtimes :: inline Networks {
	struct SerialPortParameters {
		std::string DeviceName{};
		boost::asio::serial_port::baud_rate BaudRate{115200};

		std::ostream& Format(std::ostream& stream) const {
			return stream << DeviceName << '@' << BaudRate.value();
		}
	};
}
