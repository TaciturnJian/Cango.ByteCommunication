#pragma once

#include <boost/asio/ip/udp.hpp>
#include <Cango/ByteCommunication/Core/ByteTypes.hpp>

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	SizeType ReadBytes(
		boost::asio::ip::udp::socket& socket,
		ByteSpan buffer,
		boost::system::error_code& result) noexcept;

	SizeType WriteBytes(
		boost::asio::ip::udp::socket& socket,
		CByteSpan buffer,
		boost::system::error_code& result) noexcept;
}
