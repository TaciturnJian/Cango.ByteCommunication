#pragma once

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/udp.hpp>
#include <Cango/ByteCommunication/Core/ByteTypes.hpp>

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <typename TBoostDevice>
	SizeType ReadBytes(TBoostDevice& device, const ByteSpan buffer, boost::system::error_code& result) noexcept {
		return boost::asio::read(device, boost::asio::buffer(buffer.data(), buffer.size()), result);
	}

	template <>
	SizeType ReadBytes<boost::asio::ip::udp::socket>(
		boost::asio::ip::udp::socket& device,
		ByteSpan buffer,
		boost::system::error_code& result) noexcept;

	template <typename TBoostDevice>
	SizeType WriteBytes(TBoostDevice& device, const CByteSpan buffer, boost::system::error_code& result) noexcept {
		return boost::asio::write(device, boost::asio::buffer(buffer.data(), buffer.size()), result);
	}

	template <>
	SizeType WriteBytes<boost::asio::ip::udp::socket>(
		boost::asio::ip::udp::socket& device,
		CByteSpan buffer,
		boost::system::error_code& result) noexcept;
}
