#ifndef APP_HPP_
#define APP_HPP_

#include <memory>
#include "Exception.hpp"
#include "World.hpp"
#include <boost/container/flat_map.hpp>

namespace Engine {

    DEFINE_EXCEPTION(AppException);
    DEFINE_EXCEPTION_FROM(AppExceptionOutOfRange, AppException);
    DEFINE_EXCEPTION_FROM(AppExceptionKeyNotFound, AppException);
    DEFINE_EXCEPTION_FROM(AppExceptionKeyAlreadyExists, AppException);

    template<typename Key = std::size_t>
    class App
    {
        public:
            using world = std::unique_ptr<Core::World>;
            using worlds = boost::container::flat_map<Key, world>;

        private:
            worlds _worlds;
            Key _currentWorld;

        public:
#pragma region constructors / destructors
            App() = default;
            ~App() = default;

            App(const App &other) = delete;
            App &operator=(const App &other) = delete;

            App(App &&other) noexcept = delete;
            App &operator=(App &&other) noexcept = delete;
#pragma endregion constructors / destructors

#pragma region operators
            world &operator[](const Key &aKey)
            {
                if (_worlds.find(aKey) == _worlds.end()) {
                    throw AppExceptionKeyNotFound("The key doesn't exist");
                }
                return _worlds[aKey];
            }
#pragma endregion operators

#pragma region methods
            /**
             * @brief Add a level to the app
             *
             * @param key The key of the world
             * @param aWorld The world to add
             * @throw AppExceptionKeyAlreadyExists If the key already exists
             */
            world &addWorld(const Key &aKey, world &&aWorld)
            {
                if (_worlds.find(aKey) != _worlds.end()) {
                    throw AppExceptionKeyAlreadyExists("The key already exists");
                }
                _worlds[aKey] = std::move(aWorld);
                return _worlds[aKey];
            }

            /**
             * @brief Add a level to the app
             *
             * @param key The key of the world
             * @throw AppExceptionKeyAlreadyExists If the key already exists
             */
            world &addWorld(const Key &aKey)
            {
                if (_worlds.find(aKey) != _worlds.end()) {
                    throw AppExceptionKeyAlreadyExists("The key already exists");
                }
                _worlds[aKey] = std::make_unique<Core::World>();
                return _worlds[aKey];
            }

            /**
             * @brief Remove the world at the given key
             *
             * @param key The key of the world to remove
             * @throw AppExceptionKeyNotFound If the key doesn't exist
             */
            void removeWorld(const Key &aKey)
            {
                if (_worlds.find(aKey) == _worlds.end()) {
                    throw AppExceptionKeyNotFound("The key doesn't exist");
                }
                _worlds.erase(aKey);
            }

            /**
             * @brief Get the current world
             *
             * @throw AppExceptionKeyNotFound If the key doesn't exist
             * @return The current world
             */
            [[nodiscard]] const world &getCurrentWorld() const
            {
                if (_worlds.find(_currentWorld) == _worlds.end()) {
                    throw AppExceptionKeyNotFound("The key doesn't exist");
                }
                return _worlds.at(_currentWorld);
            }

            /**
             * @brief Get the current world
             * @throw AppExceptionKeyNotFound If the key doesn't exist
             * @return The current world
             */
            [[nodiscard]] world &getCurrentWorld()
            {
                if (_worlds.find(_currentWorld) == _worlds.end()) {
                    throw AppExceptionKeyNotFound("The key doesn't exist");
                }
                return _worlds.at(_currentWorld);
            }

            /**
             * @brief Set the current world
             * @throw AppExceptionKeyNotFound If the key doesn't exist
             * @param index The index of the world to set as current
             */
            void setCurrentWorld(const Key &key)
            {
                if (_worlds.find(key) == _worlds.end()) {
                    throw AppExceptionKeyNotFound("The key doesn't exist");
                }
                _currentWorld = key;
            }
#pragma endregion methods
    };
} // namespace Engine

#endif /* !APP_HPP_ */
