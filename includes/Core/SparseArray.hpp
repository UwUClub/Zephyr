/*

*/

#ifndef SPARSEARRAY_HPP_
#define SPARSEARRAY_HPP_

#include <optional>
#include <string>
#include <vector>
#include "Exception.hpp"

namespace Engine::Core {
    DEFINE_EXCEPTION(SparseArrayException);
    DEFINE_EXCEPTION_FROM(SparseArrayExceptionOutOfRange, SparseArrayException);
    DEFINE_EXCEPTION_FROM(SparseArrayExceptionEmpty, SparseArrayException);

    /**
     * @brief SparseArray is a class that store a vector of optional of a given type
     * It represents a ONE component type, each index in the array represent the component of the entity at the same
     * index
     *
     * @tparam Component The type of the components to store
     */
    template<typename Component>
    class SparseArray final
    {
        public:
            using compRef = Component &;
            using constCompRef = const Component &;
            using optComponent = std::optional<Component>;
            using optCompRef = optComponent &;
            using vectArray = std::vector<optComponent>;
            using vectIndex = typename vectArray::size_type;
            using iterator = typename vectArray::iterator;
            using constIterator = typename vectArray::const_iterator;

        private:
            vectArray _array;

        public:
#pragma region constructors / destructors
            SparseArray() = default;
            ~SparseArray() = default;

            SparseArray(const SparseArray &other) = default;
            SparseArray &operator=(const SparseArray &other) = default;

            SparseArray(SparseArray &&other) noexcept = default;
            SparseArray &operator=(SparseArray &&other) noexcept = default;
#pragma endregion constructors / destructors

#pragma region operators

            /**
             * @brief Get the component at the given index
             * @throw SparseArrayExceptionOutOfRange if the index is out of range or if the index is empty
             * @throw SparseArrayExceptionEmpty if the component is empty
             * @param index The index to get
             * @return compRef The component at the given index
             */
            compRef operator[](vectIndex aIndex)
            {
                if (aIndex >= _array.size() || aIndex < 0) {
                    throw SparseArrayExceptionOutOfRange("index out of range: " + std::to_string(aIndex));
                }
                if (!_array[aIndex].has_value()) {
                    throw SparseArrayExceptionEmpty("index is empty: " + std::to_string(aIndex));
                }
                return _array[aIndex].value();
            }

            /**
             * @brief Get the component at the given index
             * @throw SparseArrayExceptionOutOfRange if the index is out of range or if the index is empty
             * @throw SparseArrayExceptionEmpty if the component is empty
             * @param index The index to get
             * @return compRef The component at the given index
             */
            constCompRef operator[](vectIndex aIndex) const
            {
                if (aIndex >= _array.size() || aIndex < 0) {
                    throw SparseArrayExceptionOutOfRange("index out of range: " + std::to_string(aIndex));
                }
                if (!_array[aIndex].has_value()) {
                    throw SparseArrayExceptionEmpty("index is empty: " + std::to_string(aIndex));
                }
                return _array[aIndex].value();
            }

#pragma endregion operators

#pragma region methods

            /**
             * @brief Get the component at the given index
             * @throw SparseArrayExceptionOutOfRange if the index is out of range or if the index is empty
             * @throw SparseArrayExceptionEmpty if the component is empty
             * @param index The index to get
             * @return compRef The component at the given index
             */
            compRef get(vectIndex aIndex)
            {
                if (aIndex >= _array.size() || aIndex < 0) {
                    throw SparseArrayExceptionOutOfRange("index out of range: " + std::to_string(aIndex));
                }
                if (!_array[aIndex].has_value()) {
                    throw SparseArrayExceptionEmpty("index is empty: " + std::to_string(aIndex));
                }
                return _array[aIndex].value();
            }

            /**
             * @brief Set the component at the given index
             * @throw SparseArrayExceptionOutOfRange if the index is out of range
             * @param index The index to set
             * @param value The value to set
             */
            void set(vectIndex aIndex, Component &&aValue)
            {
                if (aIndex >= _array.size() || aIndex < 0) {
                    throw SparseArrayExceptionOutOfRange("index out of range: " + std::to_string(aIndex));
                }
                _array[aIndex] = std::move(aValue);
            }

            /**
             * @brief Check if the component at the given index is set
             * @throw SparseArrayExceptionOutOfRange if the index is out of range
             * @param index The index to check
             * @return true if the component is set
             * @return false if the component is not set
             */
            bool has(vectIndex aIndex) const
            {
                if (aIndex >= _array.size() || aIndex < 0) {
                    throw SparseArrayExceptionOutOfRange("index out of range: " + std::to_string(aIndex));
                }
                return _array[aIndex].has_value();
            }

            /**
             * @brief Init the component at the given index, will resize the array if needed and set each value to
             * std::nullopt
             * @param index The index to set
             */
            void init(vectIndex aIndex)
            {
                if (aIndex >= _array.size()) {
                    _array.resize(aIndex + 1, std::nullopt);
                }
                _array[aIndex] = std::nullopt;
            }

            /**
             * @brief Emplace the component at the given index, will resize the array if needed and set each value to
             * std::nullopt
             * @param index The index to set
             * @param args The arguments to emplace
             * @return compRef The component at the given index (should be the one inserted)
             */
            template<typename... Args>
            compRef emplace(vectIndex aIndex, Args &&...aArgs)
            {
                if (aIndex >= _array.size()) {
                    _array.resize(aIndex + 1);
                }
                _array[aIndex].emplace(Component(std::forward<Args>(aArgs)...));
                return _array[aIndex].value();
            }

            /**
             * @brief Erase the component at the given index, will change the value of the component to std::nullopt,
             * won't resize the array
             * @throw SparseArrayExceptionOutOfRange if the index is out of range
             * @param index The index to erase
             */
            void erase(vectIndex aIndex)
            {
                if (aIndex >= _array.size() || aIndex < 0) {
                    throw SparseArrayExceptionOutOfRange("index out of range: " + std::to_string(aIndex));
                }
                _array[aIndex].reset();
            }

            /**
             * @brief Destroy all the components
             */
            void clear()
            {
                _array.clear();
            }

#pragma endregion methods

#pragma region iterator

            iterator begin()
            {
                return _array.begin();
            }

            iterator end()
            {
                return _array.end();
            }

            constIterator begin() const
            {
                return _array.begin();
            }

            constIterator end() const
            {
                return _array.end();
            }

            constIterator cbegin() const
            {
                return _array.cbegin();
            }

            constIterator cend() const
            {
                return _array.cend();
            }

            vectIndex size() const
            {
                return _array.size();
            }

#pragma endregion iterator
    };
} // namespace Engine::Core

#endif /* !SPARSEARRAY_HPP_ */
