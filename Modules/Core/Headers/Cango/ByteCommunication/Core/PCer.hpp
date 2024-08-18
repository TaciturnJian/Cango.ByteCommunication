#pragma once

#include <Cango/CommonUtils/AsyncItemPool.hpp>
#include <Cango/TaskDesign/DeliveryTask.hpp>

#include "PPBuffer.hpp"
#include "RWer.hpp"

namespace Cango :: inline ByteCommunication :: inline Core {
	using ReaderProvider = ItemSource<ObjectOwner<Reader>>;
	template <
		typename TItemSource,
		typename TObjectOwner = typename TItemSource::ItemType,
		typename TObject = typename TObjectOwner::element_type>
	concept IsReaderProvider = IsItemSource<TItemSource> && IsReader<TObject>;

	using ReaderConsumer = ItemDestination<ObjectOwner<Reader>>;
	template <
		typename TItemDestination,
		typename TObjectOwner = typename TItemDestination::ItemType,
		typename TObject = typename TObjectOwner::element_type>
	concept IsReaderConsumer = IsItemDestination<TItemDestination> && IsReader<TObject>;

	using WriterProvider = ItemSource<ObjectOwner<Writer>>;
	template <
		typename TItemSource,
		typename TObjectOwner = typename TItemSource::ItemType,
		typename TObject = typename TObjectOwner::element_type>
	concept IsWriterProvider = IsItemSource<TItemSource> && IsWriter<TObject>;

	using WriterConsumer = ItemDestination<ObjectOwner<Writer>>;
	template <
		typename TItemDestination,
		typename TObjectOwner = typename TItemDestination::ItemType,
		typename TObject = typename TObjectOwner::element_type>
	concept IsWriterConsumer = IsItemDestination<TItemDestination> && IsWriter<TObject>;

	using RWerProvider = ItemSource<ObjectOwner<RWer>>;
	template <
		typename TItemSource,
		typename TObjectOwner = typename TItemSource::ItemType,
		typename TObject = typename TObjectOwner::element_type>
	concept IsRWerProvider = IsItemSource<TItemSource> && IsRWer<TObject>;

	template <IsReader TReader, std::default_initializable TMessage, IsVerifier TVerifier>
	class ReaderToMessageSourceAdapter final {
		ReaderBuffer<TMessage> Buffer{};
		PingPongSpan<TVerifier> Exchanger{Buffer};
		ObjectUser<TReader> Reader{};

		struct Configurations {
			struct ActorsType {
				ObjectUser<TReader>& Reader;
			} Actors;

			struct OptionsType {
				ByteType& HeadByte;
				TVerifier& Verifier;
			} Options;
		};

	public:
		using ReaderType = TReader;
		using ItemType = TMessage;
		using VerifierType = TVerifier;

		[[nodiscard]] Configurations Configure() noexcept {
			return {
				.Actors = {Reader},
				.Options = {
					Exchanger.HeadByte,
					Exchanger.Verifier
				}
			};
		}

		[[nodiscard]] bool IsFunctional() const noexcept { return ValidateAll(Reader); }

		[[nodiscard]] bool GetItem(TMessage& message) noexcept {
			return Reader->ReadBytes(Exchanger.PongSpan) == sizeof(TMessage)
				&& Exchanger.Examine(ByteSpan{reinterpret_cast<ByteType*>(&message), sizeof(TMessage)});
		}
	};

	template <
		IsReader TReader,
		IsVerifier TVerifier,
		IsItemDestination TMessageDestination,
		IsDeliveryTaskMonitor TTaskMonitor,
		std::default_initializable TMessage = typename TMessageDestination::ItemType>
	class DeliveryTaskAsReaderConsumer final {
		using AdapterType = ReaderToMessageSourceAdapter<TReader, TMessage, TVerifier>;

		ObjectOwner<AdapterType> AdapterOwner{};
		DeliveryTask<AdapterType, TMessageDestination, TTaskMonitor> Task{};

		struct Configurations {
			struct ActorsType {
				Credential<TMessageDestination>& MessageDestination;
				Credential<TTaskMonitor>& Monitor;
			} Actors;

			struct OptionsType {
				ByteType& HeadByte;
				TVerifier& Verifier;
				std::chrono::milliseconds& MinInterval;
			} Options;
		};

	public:
		using ItemType = ObjectUser<TReader>;

		DeliveryTaskAsReaderConsumer() noexcept { AdapterOwner.Authorize(Task.Configure().Actors.ItemSource); }

		[[nodiscard]] Configurations Configure() noexcept {
			auto&& adapter = AdapterOwner->Configure();
			auto&& delivery = Task.Configure();
			return {
				.Actors = {
					delivery.Actors.ItemDestination,
					delivery.Actors.Monitor
				},
				.Options = {
					adapter.Options.HeadByte,
					adapter.Options.Verifier,
					delivery.Options.MinInterval
				}
			};
		}

		[[nodiscard]] bool IsFunctional() noexcept {
			const auto actors = Task.Configure().Actors;
			return Validate(actors.ItemDestination, actors.Monitor);
		}

		void SetItem(const ObjectUser<TReader>& reader) noexcept {
			AdapterOwner->Configure().Actors.Reader = reader;
			Task.Configure().Actors.Monitor.AcquireUser()->Reset();
			Task.Execute();
		}
	};

	template <IsWriter TWriter, std::default_initializable TMessage>
	class WriterToMessageDestinationAdapter final {
		ObjectUser<TWriter> Writer{};

		struct Configurations {
			struct ActorsType {
				ObjectUser<TWriter>& Writer;
			} Actors;
		};

	public:
		[[nodiscard]] Configurations Configure() noexcept { return {.Actors = {Writer}}; }

		[[nodiscard]] bool IsFunctional() const noexcept { return ValidateAll(Writer); }
		
		using ItemType = TMessage;
		void SetItem(const TMessage& message) noexcept {
			(void)Writer->WriteBytes(CByteSpan{reinterpret_cast<const ByteType*>(&message), sizeof(TMessage)});
		}
	};

	template <
		IsWriter TWriter,
		IsItemSource TMessageSource,
		IsDeliveryTaskMonitor TTaskMonitor,
		std::default_initializable TMessage = typename TMessageSource::ItemType>
	class DeliveryTaskAsWriterConsumer final {
		using AdapterType = WriterToMessageDestinationAdapter<TWriter, TMessage>;

		ObjectOwner<AdapterType> Transformer{};
		DeliveryTask<TMessageSource, AdapterType, TTaskMonitor> Task{};

		struct Configurations {
			struct ActorsType {
				Credential<TMessageSource>& MessageSource;
				Credential<TTaskMonitor>& Monitor;
			} Actors;

			struct OptionsType {
				std::chrono::milliseconds& MinInterval;
			} Options;
		};

	public:
		
		DeliveryTaskAsWriterConsumer() noexcept { Transformer.Authorize(Task.Configure().Actors.ItemDestination); }

		[[nodiscard]] Configurations Configure() noexcept {
			auto&& delivery = Task.Configure();
			return {
				.Actors = {
					delivery.Actors.ItemSource,
					delivery.Actors.Monitor
				},
				.Options = {
					delivery.Options.MinInterval
				}
			};
		}

		bool IsFunctional() noexcept {
			auto&& config = Task.Configure();
			return ValidateAll(config.Actors.ItemSource, config.Actors.Monitor);
		}

		using ItemType = ObjectUser<TWriter>;
		void SetItem(const ObjectUser<TWriter>& writer) noexcept {
			Transformer->Configure().Actors.Writer = writer;
			Task.Configure().Actors.Monitor.AcquireUser()->Reset();
			Task.Execute();
		}
	};

	template <
		IsRWer TRWer,
		IsVerifier TReaderMessageVerifier,
		IsItemDestination TReaderMessageDestination,
		IsItemSource TWriterMessageSource,
		IsDeliveryTaskMonitor TReaderMonitor,
		IsDeliveryTaskMonitor TWriterMonitor>
	class DeliveryTaskAsRWerConsumer final {
		using ReaderConsumerType = DeliveryTaskAsReaderConsumer<
			TRWer, TReaderMessageVerifier, TReaderMessageDestination, TReaderMonitor>;
		using WriterConsumerType = DeliveryTaskAsWriterConsumer<TRWer, TWriterMessageSource, TWriterMonitor>;

		ReaderConsumerType ReaderConsumer{};
		WriterConsumerType WriterConsumer{};

		struct Configurations {
			struct ActorsType {
				Credential<TReaderMessageDestination>& ReaderMessageDestination;
				Credential<TWriterMessageSource>& WriterMessageSource;
				Credential<TReaderMonitor>& ReadingMonitor;
				Credential<TWriterMonitor>& WritingMonitor;
			} Actors;

			struct OptionsType {
				ByteType& HeadByte;
				TReaderMessageVerifier& ReaderMessageVerifier;
				std::chrono::milliseconds& ReaderMinInterval;
				std::chrono::milliseconds& WriterMinInterval;
			} Options;
		};

	public:
		DeliveryTaskAsRWerConsumer() = default;

		[[nodiscard]] Configurations Configure() noexcept {
			auto&& reader = ReaderConsumer.Configure();
			auto&& writer = WriterConsumer.Configure();
			return {
				.Actors = {
					reader.Actors.MessageDestination,
					writer.Actors.MessageSource,
					reader.Actors.Monitor,
					writer.Actors.Monitor
				},
				.Options = {
					reader.Options.HeadByte,
					reader.Options.Verifier,
					reader.Options.MinInterval,
					writer.Options.MinInterval
				}
			};
		}

		[[nodiscard]] bool IsFunctional() noexcept {
			return ReaderConsumer.IsFunctional() && WriterConsumer.IsFunctional();
		}

		using ItemType = ObjectOwner<TRWer>;
		void SetItem(const ObjectOwner<TRWer>& rw) noexcept {
			const auto rw_user = rw.AcquireUser();
			std::thread reader_thread{
				[this, rw_user] {
					ObjectUser<TWriterMonitor> writer_monitor_user;
					if (!WriterConsumer.Configure().Actors.Monitor.Acquire(writer_monitor_user))
						return;
					auto& writer_monitor = *writer_monitor_user;
					ReaderConsumer.SetItem(rw_user);
					writer_monitor.Interrupt();
				}
			};
			std::thread writer_thread{
				[this, rw_user] {
					ObjectUser<TReaderMonitor> reader_monitor_user;
					if (!ReaderConsumer.Configure().Actors.Monitor.Acquire(reader_monitor_user)) return;
					auto& reader_monitor = *reader_monitor_user;
					WriterConsumer.SetItem(rw_user);
					reader_monitor.Interrupt();
				}
			};
			reader_thread.join();
			writer_thread.join();
		}
	};

	/// @brief 单设备获取、通信任务
	template <
		IsRWerProvider TProvider,
		IsDeliveryTaskMonitor TProviderMonitor,
		IsVerifier TReaderMessageVerifier,
		IsItemDestination TReaderMessageDestination,
		IsItemSource TWriterMessageSource,
		IsDeliveryTaskMonitor TReaderMonitor,
		IsDeliveryTaskMonitor TWriterMonitor>
	class CommunicationTask {
		using RWerConsumerType = DeliveryTaskAsRWerConsumer<
			typename TProvider::ItemType::element_type,
			TReaderMessageVerifier,
			TReaderMessageDestination,
			TWriterMessageSource,
			TReaderMonitor,
			TWriterMonitor>;
		using ProviderTaskType = DeliveryTask<TProvider, RWerConsumerType, TProviderMonitor>;

		ProviderTaskType ProviderTask{};
		ObjectOwner<RWerConsumerType> RWerConsumer{};

		struct Configurations {
			struct ActorsType {
				Credential<TProvider>& Provider;
				Credential<TReaderMessageDestination>& ReaderMessageDestination;
				Credential<TWriterMessageSource>& WriterMessageSource;
				Credential<TProviderMonitor>& ProviderMonitor;
				Credential<TReaderMonitor>& ReaderMonitor;
				Credential<TWriterMonitor>& WriterMonitor;
			} Actors;

			struct OptionsType {
				ByteType& HeadByte;
				TReaderMessageVerifier& ReaderMessageVerifier;
				std::chrono::milliseconds& ReaderMinInterval;
				std::chrono::milliseconds& WriterMinInterval;
				std::chrono::milliseconds& ProviderMinInterval;
			} Options;
		};

	public:
		using ProviderType = TProvider;
		using ProviderTaskMonitorType = TProviderMonitor;
		using ReaderMessageVerifierType = TReaderMessageVerifier;
		using ReaderMessageDestinationType = TReaderMessageDestination;
		using WriterMessageSourceType = TWriterMessageSource;
		using ReaderTaskMonitorType = TReaderMonitor;
		using WriterTaskMonitorType = TWriterMonitor;

		CommunicationTask() noexcept {
			auto&& provider = ProviderTask.Configure();
			RWerConsumer.Authorize(provider.Actors.ItemDestination);
		}

		Configurations Configure() noexcept {
			auto&& provider = ProviderTask.Configure();
			auto&& consumer = RWerConsumer->Configure();
			return {
				.Actors = {
					provider.Actors.ItemSource,
					consumer.Actors.ReaderMessageDestination,
					consumer.Actors.WriterMessageSource,
					provider.Actors.Monitor,
					consumer.Actors.ReadingMonitor,
					consumer.Actors.WritingMonitor
				},
				.Options = {
					consumer.Options.HeadByte,
					consumer.Options.ReaderMessageVerifier,
					consumer.Options.ReaderMinInterval,
					consumer.Options.WriterMinInterval,
					provider.Options.MinInterval
				}
			};
		}

		[[nodiscard]] bool IsFunctional() noexcept {
			return ProviderTask.IsFunctional() && RWerConsumer->IsFunctional();
		}

		void Execute() noexcept { ProviderTask.Execute(); }
	};

	template <
		IsRWerProvider TProvider,
		std::default_initializable TReaderMessage,
		std::default_initializable TWriterMessage>
	using EasyCommunicationTask = CommunicationTask<
		TProvider,
		EasyDeliveryTaskMonitor,
		TailZeroVerifier,
		AsyncItemPool<TReaderMessage>,
		AsyncItemPool<TWriterMessage>,
		EasyDeliveryTaskMonitor,
		EasyDeliveryTaskMonitor>;

	template <std::default_initializable TReaderMessage, std::default_initializable TWriterMessage>
	struct EasyCommunicationTaskPoolsAndMonitors {
		ObjectOwner<AsyncItemPool<TReaderMessage>> ReaderMessagePool{};
		ObjectOwner<AsyncItemPool<TWriterMessage>> WriterMessagePool{};
		ObjectOwner<EasyDeliveryTaskMonitor> ProviderMonitor{};
		ObjectOwner<EasyDeliveryTaskMonitor> ReaderMonitor{};
		ObjectOwner<EasyDeliveryTaskMonitor> WriterMonitor{};

		template <IsRWerProvider TProvider>
		void Apply(EasyCommunicationTask<TProvider, TReaderMessage, TWriterMessage>& task) noexcept {
			auto&& config = task.Configure();
			const auto actors = config.Actors;
			ReaderMessagePool.Authorize(actors.ReaderMessageDestination);
			WriterMessagePool.Authorize(actors.WriterMessageSource);
			ProviderMonitor.Authorize(actors.ProviderMonitor);
			ReaderMonitor.Authorize(actors.ReaderMonitor);
			WriterMonitor.Authorize(actors.WriterMonitor);
		}
	};
}
