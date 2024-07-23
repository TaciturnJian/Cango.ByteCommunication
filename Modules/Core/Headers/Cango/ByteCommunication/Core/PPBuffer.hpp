#pragma once

#include <algorithm>
#include <stdexcept>

#include "Verifier.hpp"

namespace Cango :: inline ByteCommunication {
	/// @brief 基于 PingPong 思想的运行时双数据包大小的内存缓冲器。
	///		输入一个数据包的字节数，经过检验后输出数据包。
	///		当一个数据包大小的数据到来时，将把所有字节加入缓冲区，然后在缓冲区中检查是否有完整的数据包，如果有则输出数据包。
	///		如果没有则储存本次数据包等待下次数据包到来。
	///	@note
	///		初始化时需要提供 数据包的头字节 和 有效的、长度为两倍数据包大小的、连续内存 （将在构造时检查）
	template <IsVerifier TVerifier>
	class PingPongSpan final {
		[[nodiscard]] bool Verify(CByteSpan span) noexcept { return span.front() == HeadByte && Verifier.Verify(span); }
		void CopyPongToPing() noexcept { std::ranges::copy(PongSpan, PingSpan.begin()); }

		/// @brief 优化函数 @c FillPingWith0 ，避免重复清空 PingSpan
		bool IsLastMessageFoundInPongSpan{false};

		void FillPingWith0() noexcept {
			if (IsLastMessageFoundInPongSpan) return; // 上一次已经清空了 Ping，这次就不用了
			IsLastMessageFoundInPongSpan = true;
			std::ranges::fill(PingSpan, 0);
		}

		[[nodiscard]] bool FindMessageSpan(CByteSpan& span) noexcept {
			const auto head_index = std::ranges::find(PingSpan, HeadByte);
			if (head_index == PingSpan.end()) return false;
			span = FullSpan.subspan(head_index - PingSpan.begin(), PingSpan.size());
			return true;
		}

	public:
		using VerifierType = TVerifier;

		ByteType HeadByte{'!'};
		ByteSpan PingSpan{};
		ByteSpan PongSpan{};
		ByteSpan FullSpan{};
		TVerifier Verifier{};

		/// @brief 给定头字节和完整的内存区间，构造 PingPongExchanger
		///	@param fullSpan size 至少大于 2，否则会抛出 std::runtime_error
		/// @exception std::runtime_error 当给定的参数不符合要求时抛出异常
		explicit PingPongSpan(const ByteSpan fullSpan) :
			PingSpan(fullSpan.data(), fullSpan.size() / 2),
			PongSpan(fullSpan.data() + PingSpan.size(), PingSpan.size()),
			FullSpan(fullSpan) {
			if (PingSpan.empty()) throw std::runtime_error("sizeof fullSpan must be at least 2");
			std::ranges::fill(PingSpan, 0);
		}

		/// @brief 假设已经写入所有数据到读取缓冲区(即 Pong 缓冲区)，现在执行交换流程，并检查是否可以输出
		///	@param destination 用于输出数据的位置，大小必须大于或等于 GetReaderSpan().size()
		[[nodiscard]] bool Examine(ByteSpan destination) noexcept {
			// 检查 destination 的空间
			if (destination.size() < PingSpan.size()) return false;

			// 先验证读取缓冲区是否有完整的数据包，以提前返回，若提前返回
			if (Verify(PongSpan)) {
				std::ranges::copy(PongSpan, destination.begin());
				FillPingWith0();
				return true;
			}

			IsLastMessageFoundInPongSpan = false;
			CByteSpan message_span{};
			const bool successful = FindMessageSpan(message_span) && Verify(message_span);
			if (successful) std::ranges::copy(message_span, destination.begin());
			CopyPongToPing();
			return successful;
		}
	};

	template <typename TMessage>
	using ReaderBuffer = ByteArray<sizeof(TMessage) * 2>;
}
