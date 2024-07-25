#include <Cango/ByteCommunication/BoostImplementations/BoostReadWrite.hpp>

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	template <>
	SizeType ReadBytes<boost::asio::ip::udp::socket>(
		boost::asio::ip::udp::socket& device,
		ByteSpan buffer,
		boost::system::error_code& result) noexcept {
		SizeType bytes{0};
		try { bytes = device.receive(boost::asio::buffer(buffer.data(), buffer.size())); }
		catch (const boost::system::error_code& error) { result = error; }
		return bytes;
	}

	template <>
	SizeType WriteBytes<boost::asio::ip::udp::socket>(
		boost::asio::ip::udp::socket& device,
		const CByteSpan buffer,
		boost::system::error_code& result) noexcept {
		SizeType bytes{0};
		try { bytes = device.send(boost::asio::buffer(buffer.data(), buffer.size())); }
		catch (const boost::system::error_code& error) { result = error; }
		return bytes;
	}
}
