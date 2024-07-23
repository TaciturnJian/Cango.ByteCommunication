#include <Cango/ByteCommunication/BoostImplementations.hpp>

#include <Cango/TaskDesign/ItemSource.hpp>
#include <Cango/TaskDesign/ItemDestination.hpp>

namespace Cango {
	struct TaskMonitor {
		virtual void HandleError() noexcept = 0;

		[[nodiscard]] virtual bool IsDone() const noexcept = 0;
		virtual void Interrupt() noexcept = 0;
		virtual void Reset() noexcept = 0;
	};

	template <typename TObject>
	concept IsTaskMonitor = requires(const TObject& obj) {
		{ obj.HandleError() };
		{ obj.IsDone() } -> std::convertible_to<bool>;
		{ obj.Interrupt() };
		{ obj.Reset() };
	};

	template <
		IsItemSource TItemSource,
		IsItemDestination TItemDestination,
		IsTaskMonitor TMonitor,
		std::default_initializable TItem = typename TItemSource::ItemType>
		requires std::same_as<TItem, typename TItemDestination::ItemType>
	class ItemDeliveryArchitect {
		Credential<TItemSource> ItemSource{};
		Credential<TItemDestination> ItemDestination{};
		Credential<TMonitor> Monitor{};
		std::chrono::milliseconds MinInterval{};

		struct ConfigurationsType {
			struct ActorsType {
				Credential<TItemSource>& ItemSource;
				Credential<TItemDestination>& ItemDestination;
				Credential<TMonitor>& Monitor;
			} Actors;

			struct OptionsType {
				std::chrono::milliseconds& MinInterval;
			} Options;
		};

	public:
		[[nodiscard]] ConfigurationsType Configure() noexcept {
			return {
				.Actors = {ItemSource, ItemDestination, Monitor},
				.Options = {MinInterval}
			};
		}

		[[nodiscard]] bool IsExecutable() const noexcept {
			return ValidateAll(ItemSource, ItemDestination, Monitor);
		}

		void Execute() noexcept {
			auto [source_user, source_object] = Acquire(ItemSource);
			auto [destination_user, destination_object] = Acquire(ItemDestination);
			auto [monitor_user, monitor_object] = Acquire(Monitor);

			while (!monitor_object.IsDone()) {
				std::this_thread::sleep_for(MinInterval);
				if (TItem item{}; source_object.GetItem()) {
					destination_object.SetItem(item);
					continue;
				}
				monitor_object.HandleError();
			}
		}
	};
}
