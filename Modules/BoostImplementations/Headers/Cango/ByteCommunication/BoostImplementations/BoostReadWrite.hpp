#pragma once

#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/ip/udp.hpp>
#include <Cango/ByteCommunication/Core/ByteTypes.hpp>

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <typename TBoostDevice>
	SizeType ReadBytes(
		TBoostDevice& device,
		const ByteSpan buffer,
		boost::system::error_code& result) noexcept {
		return boost::asio::read(device, boost::asio::buffer(buffer.data(), buffer.size()), result);
	}

	/// @brief 针对 udp socket 的写法，因为 udp 套接字不支持 @c boost::read ，所以特化 @c ReadBytes 方法
	template <>
	SizeType ReadBytes<boost::asio::ip::udp::socket>(
		boost::asio::ip::udp::socket& device,
		ByteSpan buffer,
		boost::system::error_code& result) noexcept;

	template <typename TBoostDevice>
	SizeType WriteBytes(
		TBoostDevice& device,
		const CByteSpan buffer,
		boost::system::error_code& result) noexcept {
		return boost::asio::write(device, boost::asio::buffer(buffer.data(), buffer.size()), result);
	}

	/// @brief 针对 udp socket 的写法，因为 udp 套接字不支持 @c boost::write ，所以特化 @c WriteBytes 方法
	template <>
	SizeType WriteBytes<boost::asio::ip::udp::socket>(
		boost::asio::ip::udp::socket& device,
		CByteSpan buffer,
		boost::system::error_code& result) noexcept;
}
