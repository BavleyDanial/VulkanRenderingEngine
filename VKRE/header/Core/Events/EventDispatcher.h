#pragma once

#include "Events.h"

#include <algorithm>
#include <vector>
#include <array>

namespace VKRE {

    static inline size_t GetUniqueEventID() {
        static size_t counter = 0;
        return counter++;
    }

    template<typename T>
    static inline size_t GetEventID() {
        static const size_t id = GetUniqueEventID();
        return id;
    }

    class EventDispatcher {
    public:
        template<typename T, typename ClassType>
        requires IsEvent<T>
        void RegisterListener(ClassType* instance, void(ClassType::*Method)(const T&)) {
            struct Router {
                static void Execute(void* inst, void* methodBuffer, const void* e) {
                    auto method = *reinterpret_cast<void(ClassType::**)(const T&)>(methodBuffer);
                    const T& event = *static_cast<const T*>(e);
                    (static_cast<ClassType*>(inst)->*method)(event);
                }
            };

            TypeErasedDelegate delegate;
            delegate.instance = instance;
            delegate.invoke = Router::Execute;

            memset(delegate.fnPtr, 0, sizeof(delegate.fnPtr));
            memcpy(delegate.fnPtr, &Method, sizeof(Method));

            mListeners[GetEventID<T>()].push_back(delegate);
        }

        template<typename T>
        requires IsEvent<T>
        void RegisterListener(void(*Function)(const T&)) {
            struct Router {
                static void Execute(void* functionBuffer, const void* e) {
                    auto function = *reinterpret_cast<void(**)(const T&)>(functionBuffer);
                    const T& event = *static_cast<const T*>(e);
                    function(event);
                }
            };

            TypeErasedDelegate delegate;
            delegate.instance = nullptr;
            delegate.invoke = Router::Execute;

            memset(delegate.fnPtr, 0, sizeof(delegate.fnPtr));
            memcpy(delegate.fnPtr, &Function, sizeof(Function));

            mListeners[GetEventID<T>()].push_back(delegate);
        }

        template<typename T>
        requires IsEvent<T>
        void BroadcastToListeners(const T& e) {
            const auto& listeners = mListeners[GetEventID<T>()];
            for (const auto& listener : listeners) {
                listener.invoke(listener.instance, const_cast<uint8_t*>(listener.fnPtr), &e);
            }
        }

    private:
        struct TypeErasedDelegate {
            using MemberFnPtr = void(TypeErasedDelegate::*)();
            using FreeFnPtr = void(*)();

            static constexpr size_t ptrSize = std::max(sizeof(MemberFnPtr), sizeof(FreeFnPtr));
            static constexpr size_t ptrAlign = std::max(alignof(MemberFnPtr), alignof(FreeFnPtr));

            void* instance;
            alignas(ptrAlign) uint8_t fnPtr[ptrSize];
            void (*invoke)(void* instance, void* methodBuffer, const void* e);
        };

        std::array<std::vector<TypeErasedDelegate>, EventTypesCount> mListeners;

    };

}
