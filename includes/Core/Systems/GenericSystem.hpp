#ifndef GENERICSYSTEM_HPP_
#define GENERICSYSTEM_HPP_

#include <functional>
#include <utility>
#include "Core/Clock.hpp"
#include "Core/World.hpp"
#include "System.hpp"

namespace Engine::Core {
    template<typename Func, typename... Components>
    class GenericSystem : public System
    {
        public:
            GenericSystem(Core::World &world, Func updateFunc)
                : _world(world),
                  _updateFunc(updateFunc)
            {}

            void update() override
            {
                double deltaTime = _clock.getElapsedTime();
                _world.get().template query<Components...>().forEach(deltaTime, _updateFunc);
            }

        private:
            std::reference_wrapper<Core::World> _world;
            Func _updateFunc;
            Clock _clock;
    };

    template<typename... Components, typename Func>
    std::pair<std::string, std::unique_ptr<System>> createSystem(World &aWorld, const std::string &aName,
                                                                 Func aUpdateFunc)
    {
        return std::pair<std::string, std::unique_ptr<System>>(
            std::make_pair(aName, std::make_unique<GenericSystem<Func, Components...>>(aWorld, aUpdateFunc)));
    }

} // namespace Engine::Core

#endif /* !GENERICSYSTEM_HPP_ */
