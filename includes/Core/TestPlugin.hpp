#ifndef TESTPLUGIN_HPP_
#define TESTPLUGIN_HPP_

#include <iostream>
class TestPlugin
{
    public:
        explicit TestPlugin(std::string greetings)
            : _greetings(std::move(greetings))
        {}

        void sayHello() const
        {
            std::cout << "Hello from " << _greetings << std::endl;
        }

    private:
        std::string _greetings;

    protected:
    private:
};

#endif /* !TESTPLUGIN_HPP_ */
