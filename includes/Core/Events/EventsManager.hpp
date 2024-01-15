#ifndef EVENTMANAGER_HPP
#define EVENTMANAGER_HPP

#include <any>
#include <cstddef>
#include <functional>
#include <typeindex>
#include <utility>
#include <vector>
#include "EventHandler.hpp"
#include "Exception.hpp"
#include <boost/container/flat_map.hpp>

namespace Engine::Event {
    DEFINE_EXCEPTION(EventManagerException);
    DEFINE_EXCEPTION_FROM(EventManagerExceptionNoHandler, EventManagerException);

    /**
     * @brief EventManager class is a singleton that manage all events
     *
     */
    class EventManager final
    {
        public:
            using func = std::function<void(EventManager &)>;
            using eventHandler = std::pair<std::any, func>;

        private:
            EventManager();

            boost::container::flat_map<std::type_index, eventHandler> _eventsHandler;

        public:
            //------------------- DESTRUCTOR-------------------//
            /**
             * @brief Destroy the Event Manager object
             *
             */
            ~EventManager();

            //-------------------OPERATORS-------------------//
            /**
             * @brief Copy assignment operator, delete because singleton.
             *
             * @param aOther The EventManager to copy.
             */
            EventManager(const EventManager &aOther) = delete;

            /**
             * @brief Move assignment operator, delete because singleton.
             *
             * @param aOther The EventManager to move.
             */
            EventManager(EventManager &&aOther) noexcept = delete;

            /**
             * @brief Copy assignment operator, delete because singleton.
             *
             * @param aOther The EventManager to copy.
             * @return EventManager& A reference to the EventManager.
             */
            EventManager &operator=(const EventManager &aOther) = delete;

            /**
             * @brief Move assignment operator, delete because singleton.
             *
             * @param aOther The EventManager to move.
             * @return EventManager& A reference to the EventManager.
             */
            EventManager &operator=(EventManager &&aOther) noexcept = delete;

            //-------------------METHODS-------------------//
            /**
             * @brief Get the Instance object (singleton)
             *
             * @return EventManager A reference to the EventManager.
             */
            static EventManager &getInstance();

            /**
             * @brief Push an event to the queue
             * @details Doesn't call the subscribers
             * @param aEvent The event to push.
             * @tparam Event The type of the event.
             */
            template<typename Event>
            void pushEvent(const Event &aEvent)
            {
                try {
                    auto &handler = getHandler<Event>();

                    handler.pushEvent(aEvent);
                } catch (const std::bad_any_cast &e) {
                    throw EventManagerExceptionNoHandler("Can't push event");
                }
            }

            /**
             * @brief Get all the events of a specific type
             * @tparam Event The type of the event.
             * @return std::vector<Event>& The list of events.
             */
            template<typename Event>
            std::vector<Event> &getEventsByType()
            {
                try {
                    auto &handler = getHandler<Event>();

                    return handler.getEvents();
                } catch (const std::bad_any_cast &e) {
                    throw EventManagerExceptionNoHandler("Can't get events");
                }
            }

            /**
             * @brief Clear all the events of the types that aren't in the list
             * @tparam EventList The list of events to keep.
             */
            template<typename... EventList>
            void keepEventsAndClear()
            {
                std::vector<std::type_index> eventIndexList = {std::type_index(typeid(EventList))...};

                for (auto &lbd : _eventsHandler) {
                    if (std::find(eventIndexList.begin(), eventIndexList.end(), lbd.first) == eventIndexList.end()) {
                        lbd.second.second(*this);
                    }
                }
            }

            /**
             * @brief Remove an event from the queue
             * @param aIndex The index of the event to remove.
             * @tparam Event The type of the event.
             *
             */
            template<typename Event>
            void removeEvent(const std::size_t aIndex)
            {
                auto eventIndex = std::type_index(typeid(Event));

                if (_eventsHandler.find(eventIndex) == _eventsHandler.end()) {
                    return;
                }
                try {
                    auto &handler = getHandler<Event>();

                    handler.removeEvent(aIndex);
                } catch (const std::bad_any_cast &e) {
                    throw EventManagerExceptionNoHandler("Can't remove event");
                }
            }

            /**
             * @brief Remove an event from the queue
             * @param aIndexes The indexes of the event to remove.
             * @tparam Event The type of the event.
             *
             */
            template<typename Event>
            void removeEvent(std::vector<size_t> aIndexes)
            {
                auto eventIndex = std::type_index(typeid(Event));

                if (_eventsHandler.find(eventIndex) == _eventsHandler.end()) {
                    return;
                }
                try {
                    auto &handler = getHandler<Event>();

                    for (size_t i = 0; i < aIndexes.size(); i++) {
                        size_t idx = aIndexes[i] - i;

                        handler.removeEvent(idx);
                    }
                } catch (const std::bad_any_cast &e) {
                    throw EventManagerExceptionNoHandler("Can't remove event");
                }
            }

            template<typename Event>
            void initEventHandler()
            {
                auto eventTypeIndex = std::type_index(typeid(Event));

                if (_eventsHandler.find(eventTypeIndex) != _eventsHandler.end()) {
                    return;
                }
                _eventsHandler[eventTypeIndex] = std::make_pair(EventHandler<Event>(), [](EventManager &aEventManager) {
                    auto &handler = aEventManager.getHandler<Event>();

                    handler.clearEvents();
                });
            }

            template<typename... EventList>
            void initEventHandlers()
            {
                (initEventHandler<EventList>(), ...);
            }

        private:
            /**
             * @brief Get an Hander linked to an event
             *
             * @tparam Event The type of the event to get the handler
             * @return EventHandler<Event>& The handler of the event.
             */
            template<typename Event>
            EventHandler<Event> &getHandler()
            {
                auto eventTypeIndex = std::type_index(typeid(Event));

                try {
                    if (_eventsHandler.find(eventTypeIndex) == _eventsHandler.cend()) {
                        throw EventManagerExceptionNoHandler("There is no handler of this type");
                    }
                    auto &handler = _eventsHandler.at(eventTypeIndex);
                    auto &component = std::any_cast<EventHandler<Event> &>(handler);

                    return component;
                } catch (const std::bad_any_cast &e) {
                    throw EventManagerExceptionNoHandler("There is no handler of this type");
                }
            }
    };
} // namespace Engine::Event

#endif // !