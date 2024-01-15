#ifndef SYSTEM_HPP_
#define SYSTEM_HPP_

namespace Engine::Core {
    class System
    {
        public:
            System() = default;
            virtual ~System() = default;
            virtual void update() = 0;

            System &operator=(const System &) = default;
            System &operator=(System &&) = default;

            System(const System &) = default;
            System(System &&) = default;

        public:
            bool _isActivated = true;

        private:
    };
} // namespace Engine::Core

#endif /* !SYSTEM_HPP_ */
