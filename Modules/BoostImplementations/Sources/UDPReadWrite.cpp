#include <Cango/ByteCommunication/BoostImplementations/UDPReadWrite.hpp>

namespace Cango :: inline ByteCommunication :: inline BoostImplementations {
	SizeType ReadBytes(
		boost::asio::ip::udp::socket& socket,
		ByteSpan buffer,
		boost::system::error_code& result) noexcept {
		SizeType bytes{0};
		try { bytes = socket.receive(boost::asio::buffer(buffer.data(), buffer.size())); }
		catch (const boost::system::error_code& error) { result = error; }
		return bytes;
	}

	SizeType WriteBytes(
		boost::asio::ip::udp::socket& socket,
		CByteSpan buffer,
		boost::system::error_code& result) noexcept {
		SizeType bytes{0};
		try { bytes = socket.send(boost::asio::buffer(buffer.data(), buffer.size())); }
		catch (const boost::system::error_code& error) { result = error; }
		return bytes;
	}
}
