#pragma once

#include "SerialPortMakeParameters.hpp"

namespace Cango:: inline ByteCommunication :: inline BoostImplementations {
	inline ObjectOwner<boost::asio::serial_port> MakeSerialPort(
		boost::asio::io_context& context,
		const SerialPortMakeParameters& parameters,
		std::string& name) noexcept {
		if (parameters.DeviceNames.empty()) return nullptr;

		auto serial_port = std::make_shared<boost::asio::serial_port>(context);
		bool open_succeeded = false;
		for (const auto& device : parameters.DeviceNames) {
			if (boost::system::error_code result{};
				serial_port->open(device, result).failed()) {
				spdlog::error("SerialPort> failed to open device: {}", result.what());
				continue;
			}
			name = device;
			open_succeeded = true;
			break;
		}

		if (!open_succeeded) return nullptr;

		const auto brief = fmt::format("SerialPort({})", name);

		boost::system::error_code result{};
		using baud_rate = boost::asio::serial_port_base::baud_rate;
		if (serial_port->set_option(baud_rate(parameters.BaudRate), result).failed()) {
			spdlog::error("{}> cannot set baud rate: {}", brief, result.what());
			return nullptr;
		}

		using flow_control = boost::asio::serial_port_base::flow_control;
		if (serial_port->set_option(flow_control(flow_control::none), result).failed()) {
			spdlog::error("{}> cannot set flow control: {}", brief, result.what());
			return nullptr;
		}

		using parity = boost::asio::serial_port_base::parity;
		if (serial_port->set_option(parity(parity::none), result).failed()) {
			spdlog::error("{}> cannot set parity: {}", brief, result.what());
			return nullptr;
		}

		using stop_bits = boost::asio::serial_port_base::stop_bits;
		if (serial_port->set_option(stop_bits(stop_bits::one), result).failed()) {
			spdlog::error("{}> cannot set stop bits: {}", brief, result.what());
			return nullptr;
		}

		using character_size = boost::asio::serial_port_base::character_size;
		if (serial_port->set_option(character_size(8), result).failed()) {
			spdlog::error("{}> cannot set character size: {}", brief, result.what());
			return nullptr;
		}

		return serial_port;
	}
}
