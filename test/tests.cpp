#include <cstddef>
#include <cstdio>
#include <functional>
#include <iostream>
#include <memory>
#include "Core/Libraries/PluginLoader.hpp"
#include "Core/Systems/GenericSystem.hpp"
#include "Core/Systems/System.hpp"
#include "Core/TestPlugin.hpp"
#include "Core/World.hpp"
#include "ECS.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("App", "[App]")
{
    Engine::App app;
    auto world = std::make_unique<Engine::Core::World>();

    SECTION("Add a world")
    {
        app.addWorld(0, std::move(world));
        REQUIRE(app[0] != nullptr);
    }
    SECTION("Add a world with a key already used")
    {
        app.addWorld(0, std::move(world));
        REQUIRE_THROWS_AS(app.addWorld(0, std::move(world)), Engine::AppExceptionKeyAlreadyExists);
    }
    SECTION("Get a world with a key not used")
    {
        REQUIRE_THROWS_AS(app[0], Engine::AppExceptionKeyNotFound);
    }
    SECTION("Get a world with a key used")
    {
        app.addWorld(0, std::move(world));
        REQUIRE_NOTHROW(app[0]);
    }
    SECTION("Get a world with a key used and check if it's the same")
    {
        app.addWorld(0, std::move(world));
        REQUIRE(app[0] == app[0]);
    }
    SECTION("Get a world with a key used and check if it's not the same")
    {
        app.addWorld(0, std::move(world));
        app.addWorld(1, std::move(world));
        REQUIRE(app[0] != app[1]);
    }
    SECTION("Get a world with a key used and check if it's not the same after a move")
    {
        app.addWorld(0, std::move(world));
        auto tmp = std::move(app[0]);
        REQUIRE(app[0] != tmp);
    }
}

struct hp1
{
        int hp;
};

struct hp2
{
        int maxHp;
};

template<typename... Components>
class MySystemClass : public Engine::Core::System
{
    public:
        explicit MySystemClass(Engine::Core::World &world)
            : Engine::Core::System(),
              _world(world)
        {}

        MySystemClass(const MySystemClass &) = default;
        MySystemClass(MySystemClass &&) = default;

        MySystemClass &operator=(const MySystemClass &) = default;
        MySystemClass &operator=(MySystemClass &&) = default;

        void update() override
        {
            _world.get().template query<Components...>().forEach(
                _clock.getElapsedTime(), [this](Engine::Core::World & /*world*/, double /*deltaTime*/, std::size_t idx,
                                                Components &...components) {
                    this->updateSystem(_world.get(), _clock.getElapsedTime(), idx, components...);
                });
            _clock.restart();
        }

    private:
        std::reference_wrapper<Engine::Core::World> _world;
        Engine::Clock _clock;

        void updateSystem(Engine::Core::World & /*world*/, double /*deltaTime*/, std::size_t idx, hp1 &hp1Comp,
                          hp2 &hp2Comp)
        {
            hp1Comp.hp--;
            hp2Comp.maxHp -= 2;
        }
};

TEST_CASE("World", "[World]")
{
    Engine::Core::World world;
    auto MySystem = Engine::Core::createSystem<hp1, hp2>(
        world, "MySystem",
        [](Engine::Core::World & /*world*/, double /*deltaTime*/, std::size_t idx, hp1 &cop1, hp2 &cop2) {
            cop1.hp--;
            cop2.maxHp -= 2;
        });

    SECTION("Create an entity")
    {
        auto entity = world.createEntity();
        REQUIRE(entity == 0);
    }
    SECTION("Create an entity and check if it's not the same")
    {
        auto entity = world.createEntity();
        REQUIRE(entity != world.createEntity());
    }
    SECTION("Create an entity and kill it")
    {
        auto entity = world.createEntity();
        world.killEntity(entity);
        REQUIRE(entity == world.createEntity());
    }

    SECTION("Run a system")
    {
        constexpr int hps = 10;

        world.registerComponents<hp1, hp2>();
        world.addSystem(MySystem);

        auto entity = world.createEntity();
        auto entity2 = world.createEntity();
        auto entity3 = world.createEntity();
        auto &hp1Comp = world.addComponentToEntity(entity, hp1 {hps});
        auto &hp2Comp = world.addComponentToEntity(entity, hp2 {hps});
        auto &hp1Comp2 = world.addComponentToEntity(entity3, hp1 {hps});
        auto &hp2Comp2 = world.addComponentToEntity(entity3, hp2 {hps});
        auto &hp2Comp3 = world.addComponentToEntity(entity2, hp2 {hps});

        world.runSystems();
        REQUIRE(hp1Comp.hp == hps - 1);
        REQUIRE(hp2Comp.maxHp == hps - 2);
        REQUIRE(hp1Comp2.hp == hps - 1);
        REQUIRE(hp2Comp2.maxHp == hps - 2);
        REQUIRE(hp2Comp3.maxHp == hps);
        auto secondSystem = std::make_pair<std::string, std::unique_ptr<Engine::Core::System>>(
            "MySystemClass", std::make_unique<MySystemClass<hp1, hp2>>(world));
        world.addSystem(secondSystem);
        world.runSystems();
        REQUIRE(hp1Comp.hp == hps - 3);
        REQUIRE(hp2Comp.maxHp == hps - 6);
        REQUIRE(hp1Comp2.hp == hps - 3);
        REQUIRE(hp2Comp2.maxHp == hps - 6);
        REQUIRE(hp2Comp3.maxHp == hps);
        world.removeSystem(MySystem.first);
        world.runSystems();
        REQUIRE(hp1Comp.hp == hps - 4);
        REQUIRE(hp2Comp.maxHp == hps - 8);
        REQUIRE(hp1Comp2.hp == hps - 4);
        REQUIRE(hp2Comp2.maxHp == hps - 8);
        REQUIRE(hp2Comp3.maxHp == hps);
    }
}

TEST_CASE("Plugin")
{
    Engine::Plugin::PluginLoader<TestPlugin> loader;

    SECTION("Load a plugin")
    {
        loader.load("./libapi.so");
        REQUIRE_NOTHROW(loader.getPlugin());
    }
    SECTION("Load a plugin and unload it")
    {
        loader.load("./libapi.so");
        REQUIRE_NOTHROW(loader.unload());
    }
    SECTION("Load a plugin and exec cmd")
    {
        loader.load("./libapi.so");
        auto &plugin = loader.getPlugin();
        plugin.sayHello();
    }
}
