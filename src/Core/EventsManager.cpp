#include "Events/EventsManager.hpp"

//-------------------CONSTRUCTORS / DESTRUCTOR-------------------//
Engine::Event::EventManager::EventManager() = default;

Engine::Event::EventManager::~EventManager() = default;

//-------------------PUBLIC METHODS-------------------//

Engine::Event::EventManager &Engine::Event::EventManager::getInstance()
{
    static EventManager instance;

    return instance;
}
