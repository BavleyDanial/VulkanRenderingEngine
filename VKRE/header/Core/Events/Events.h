#pragma once

#include <format>
#include <type_traits>
#include <variant>
#include <string>

namespace VKRE {

    struct WindowResizeEvent {
        uint32_t Width, Height = 0;

        std::string ToString() const {
            return std::format("WindowResizeEvent: {}x{}", Width, Height);
        }
    };

    using EventData = std::variant<WindowResizeEvent>;
    constexpr size_t EventTypesCount = std::variant_size_v<EventData>;

    namespace EventsDetails {
        template<typename T, typename Variant>
        struct IsVariantAlternative;

        template<typename T, typename... Events>
        struct IsVariantAlternative<T, std::variant<Events...>>
            :std::bool_constant<(std::same_as<T, Events> || ...)> {};
    }

    template<typename T>
    concept IsEvent = EventsDetails::IsVariantAlternative<T, EventData>::value;

    class Event {
    public:
        bool Handled = false;

    public:
        Event(EventData data)
            :mData(data) {}

        std::string ToSting() const {
            return std::visit([](const auto& eventImpl) { return eventImpl.ToString(); }, mData);
        }

        template<typename T>
        requires IsEvent<T>
        bool IsType() const {
            return std::holds_alternative<T>(mData);
        }

        template<typename T>
        requires IsEvent<T>
        const T& GetData() const {
            return std::get<T>(mData);
        }

    private:
        EventData mData;
    };

}
