#pragma once

#include <Cango/Constrained/FormattableObjects.hpp>

#include "SerialPortParameters.hpp"

namespace Cango:: inline Runtimes :: inline Networks {
	/// @brief 串口封装，使用更简单
	///	只提供了串口的读、写、打开、关闭基本操作和一些便于日志的函数
	struct SerialPortWrapper final : BoostIO<boost::asio::serial_port>, NamedModule {
		static constexpr char Name[] = "串口";

		SerialPortParameters Parameters{};

		std::ostream& GetName(std::ostream& stream) const noexcept override { return stream << Name; }

		std::ostream& GetBrief(std::ostream& stream) const noexcept override { return Parameters.Format(stream); }

		[[nodiscard]] std::string GetName() const noexcept override { return Cango::Format(*this); }
	};
}
