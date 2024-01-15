#ifndef WORLD_HPP_
#define WORLD_HPP_

#include <any>
#include <cstddef>
#include <functional>
#include <memory>
#include <tuple>
#include <typeindex>
#include <utility>
#include <vector>
#include "Exception.hpp"
#include "SparseArray.hpp"
#include "Systems/System.hpp"
#include <boost/container/flat_map.hpp>
namespace Engine::Core {
    DEFINE_EXCEPTION(WorldException);
    DEFINE_EXCEPTION_FROM(WorldExceptionComponentAlreadyRegistered, WorldException);
    DEFINE_EXCEPTION_FROM(WorldExceptionComponentNotRegistered, WorldException);
    DEFINE_EXCEPTION_FROM(WorldExceptionSystemAlreadyRegistered, WorldException);
    DEFINE_EXCEPTION_FROM(WorldExceptionSystemNotRegistered, WorldException);

    /**
     * @brief The world class represents a level, a scene
     * @details it contains the entities, components and systems used in the scene
     *
     */
    class World
    {
        public:
            using id = std::size_t;
            using containerFunc = std::function<void(World &, const id &)>;
            using container = std::pair<std::any, std::tuple<containerFunc, containerFunc>>;
            using containerMap = boost::container::flat_map<std::type_index, container>;
            using idsContainer = std::vector<id>;
            using systemFunc = std::unique_ptr<System>;
            using newSystemFunc = std::pair<std::string, std::unique_ptr<System>>;
            using systems = boost::container::flat_map<std::string, systemFunc>;

        protected:
            containerMap _components;
            idsContainer _ids;
            std::size_t _nextId = 0;
            systems _systems;

            template<typename... Components>
            class Query
            {
                public:
                    explicit Query(Core::World &world)
                        : _world(world)
                    {}

                    void
                    forEach(double deltaTime,
                            std::function<void(World &world, double deltaTime, std::size_t idx, Components &...)> func)
                    {
                        for (std::size_t idx = 0; idx < _world.get().getCurrentId(); idx++) {
                            if (_world.get().template hasComponents<Components...>(idx)) {
                                func(_world.get(), deltaTime, idx, _world.get().template getComponent<Components>().get(idx)...);
                            }
                        }
                    }

                private:
                    std::reference_wrapper<Core::World> _world;
            };

        public:
#pragma region constructors / destructors
            World() = default;
            ~World() = default;

            World(const World &other) = default;
            World &operator=(const World &other) = default;

            World(World &&other) noexcept = default;
            World &operator=(World &&other) noexcept = default;
#pragma endregion constructors / destructors

#pragma region methods

            template<typename... Components>
            Query<Components...> query()
            {
                return Query<Components...>(*this);
            }

            /**
             * @brief Add a component to the World
             *
             * @tparam Component Type of the component
             * @return SparseArray<Component>& Reference to the component SparseArray
             */
            template<typename Component>
            SparseArray<Component> &registerComponent()
            {
                auto typeIndex = std::type_index(typeid(Component));

                if (_components.find(typeIndex) != _components.end()) {
                    throw WorldExceptionComponentAlreadyRegistered("Component already registered");
                }
                _components[typeIndex] = std::make_pair(SparseArray<Component>(),
                                                        std::make_tuple(
                                                            [](World &aWorld, const std::size_t &aIdx) {
                                                                auto &myComponent = aWorld.getComponent<Component>();

                                                                myComponent.init(aIdx);
                                                            },
                                                            [](World &aWorld, const std::size_t &aIdx) {
                                                                auto &myComponent = aWorld.getComponent<Component>();

                                                                myComponent.erase(aIdx);
                                                            }));
                return std::any_cast<SparseArray<Component> &>(_components[typeIndex].first);
            }

            /**
             * @brief Add multiple components to the World
             *
             * @tparam Components The components to add
             */
            template<typename... Components>
            void registerComponents()
            {
                (registerComponent<Components>(), ...);
            }

            /**
             * @brief Get the Component object
             *
             * @tparam Component The type of the component
             * @return SparseArray<Component>& the SparseArray of the component
             */
            template<typename Component>
            SparseArray<Component> &getComponent()
            {
                auto typeIndex = std::type_index(typeid(Component));

                if (_components.find(typeIndex) == _components.end()) {
                    throw WorldExceptionComponentNotRegistered("Component not registered");
                }
                return std::any_cast<SparseArray<Component> &>(_components[typeIndex].first);
            }

            /**
             * @brief Get the Component object
             *
             * @tparam Component The type of the component
             * @return SparseArray<Component>& the SparseArray of the component
             */
            template<typename Component>
            SparseArray<Component> const &getComponent() const
            {
                auto typeIndex = std::type_index(typeid(Component));

                if (_components.find(typeIndex) == _components.end()) {
                    throw WorldExceptionComponentNotRegistered("Component not registered");
                }
                return std::any_cast<SparseArray<Component> const &>(_components.at(typeIndex).first);
            }

            /**
             * @brief Check if the entity has all the components
             *
             * @tparam Components The components to check
             * @param aIndex The index of the entity
             * @return true if the entity has all the components
             * @return false if the entity doesn't have all the components
             */
            template<typename... Components>
            [[nodiscard]] bool hasComponents(std::size_t aIndex) const
            {
                return (... && getComponent<Components>().has(aIndex));
            }

            /**
             * @brief Remove a component
             *
             * @tparam Component The type of the component
             */
            template<typename Component>
            void removeComponent()
            {
                auto typeIndex = std::type_index(typeid(Component));

                if (_components.find(typeIndex) == _components.end()) {
                    throw WorldExceptionComponentNotRegistered("Component not registered");
                }
                _components.erase(typeIndex);
            }

            /**
             * @brief Add a component to an entity
             *
             * @tparam Component The type of the component to add
             * @param aIndex The index of the entity
             * @param aComponent The component to add
             * @return Component& The component added
             */
            template<typename Component>
            Component &addComponentToEntity(std::size_t aIndex, Component &&aComponent)
            {
                try {
                    auto &component = getComponent<Component>();

                    component.set(aIndex, std::forward<Component>(aComponent));
                    return component.get(aIndex);
                } catch (WorldExceptionComponentNotRegistered &e) {
                    throw WorldExceptionComponentNotRegistered("Component not registered");
                }
            }

            /**
             * @brief Build and add a component to an entity
             *
             * @tparam Component The type of the component to add
             * @tparam Args The types of the arguments to pass to the component constructor (infered)
             * @param aIndex The index of the entity
             * @param aArgs The arguments to pass to the component constructor
             * @return Component& The component added
             */
            template<typename Component, typename... Args>
            Component &emplaceComponentToEntity(std::size_t aIndex, Args &&...aArgs)
            {
                try {
                    auto &component = getComponent<Component>();

                    component.emplace(aIndex, std::forward<Args>(aArgs)...);
                    return component.get(aIndex);
                } catch (WorldExceptionComponentNotRegistered &e) {
                    throw WorldExceptionComponentNotRegistered("Component not registered");
                }
            }

            /**
             * @brief Remove a component from an entity
             *
             * @tparam Component The type of the component to remove
             * @param aIndex The index of the entity
             */
            template<typename Component>
            void removeComponentFromEntity(std::size_t aIndex)
            {
                try {
                    auto &component = getComponent<Component>();

                    component.erase(aIndex);
                } catch (WorldExceptionComponentNotRegistered &e) {
                    throw WorldExceptionComponentNotRegistered("Component not registered");
                }
            }

            /**
             * @brief Kill an entity
             * @details Call erase from each component on the entity, then add the id as a free id
             * @param aIndex The index of the entity to kill
             */
            void killEntity(std::size_t aIndex);

            /**
             * @brief Create an entity
             * @details Get the next free id, call init from each component on the entity, then return the id
             * @return std::size_t The id of the entity
             */
            std::size_t createEntity();

            /**
             * @brief Add a system to the world
             *
             * @tparam Components The components the system will use
             * @tparam Function The type of the system (infered)
             * @param aFunction The system to add
             */
            void addSystem(newSystemFunc &aSystem)
            {
                if (_systems.find(aSystem.first) != _systems.end()) {
                    throw WorldExceptionSystemAlreadyRegistered("System already registered");
                }

                _systems[aSystem.first] = std::move(aSystem.second);
            }

            /**
             * @brief Remove a system from the world
             *
             * @tparam Function The type of the system (infered)
             * @param aFunction The system to remove
             */
            void removeSystem(std::string &aFuncName)
            {
                if (_systems.find(aFuncName) == _systems.end()) {
                    throw WorldExceptionSystemNotRegistered("System not registered");
                }

                _systems.erase(aFuncName);
            }

            /**
             * @brief Run all the systems once
             *
             */
            void runSystems();

            /**
             * @brief Get the Current Id object
             *
             * @return std::size_t The current id
             */
            [[nodiscard]] std::size_t getCurrentId() const;

        protected:
            /**
             * @brief Get the Init Func used to init the component
             *
             * @param aTypeIndex The type index of the component
             * @return containerFunc The init function
             */
            containerFunc getInitFunc(std::type_index aTypeIndex)
            {
                return std::get<0>(_components[aTypeIndex].second);
            }

            /**
             * @brief Get the Erase Func used to erase the component
             *
             * @param aTypeIndex The type index of the component
             * @return containerFunc The erase function
             */
            containerFunc getEraseFunc(std::type_index aTypeIndex)
            {
                return std::get<1>(_components[aTypeIndex].second);
            }
#pragma endregion methods
    };
} // namespace Engine::Core

#endif /* !WORLD_HPP_ */
