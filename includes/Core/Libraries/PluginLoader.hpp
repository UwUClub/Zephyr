#ifndef LIBRARYLOADER_HPP_
#define LIBRARYLOADER_HPP_

#include <dlfcn.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>
#include <utility>
#include "Exception.hpp"

namespace Engine::Plugin {
    DEFINE_EXCEPTION(PluginLoaderException);
    DEFINE_EXCEPTION_FROM(PluginLoaderExceptionAlreadyLoaded, PluginLoaderException);
    DEFINE_EXCEPTION_FROM(PluginLoaderExceptionNotLoaded, PluginLoaderException);

    template<typename Plugin>
    class PluginLoader
    {
        protected:
            Plugin *_plugin;
            std::string _path;
            void *_library;

        public:
            explicit PluginLoader(std::string aPath)
                : _path(std::move(aPath)),
                  _plugin(nullptr),
                  _library(nullptr)
            {}
            explicit PluginLoader()
                : _plugin(nullptr),
                  _library(nullptr)
            {}

            ~PluginLoader()
            {
                try {
                    unload();
                } catch (const PluginLoaderException &e) {
                    spdlog::error(e.what());
                }
            }

            PluginLoader &operator=(const PluginLoader &) = default;
            PluginLoader &operator=(PluginLoader &&) = default;

            PluginLoader(const PluginLoader &) = default;
            PluginLoader(PluginLoader &&) = default;

            Plugin &load(const std::string &aPath)
            {
                if (_path == aPath) {
                    throw PluginLoaderExceptionAlreadyLoaded(aPath + ": Library is already loaded");
                }
                unload();
                _path = aPath;
                return load();
            }

            Plugin &load()
            {
                if (_plugin != nullptr) {
                    try {
                        unload();
                    } catch (const PluginLoaderException &e) {
                        throw PluginLoaderException(e.what());
                    }
                }

                _library = dlopen(_path.c_str(), RTLD_LAZY);
                if (_library == nullptr) {
                    throw PluginLoaderException("An error occured while trying to open " + _path + ": " + dlerror());
                }

                try {
                    auto pluginGetter = reinterpret_cast<Plugin *(*) ()>(getFunction("getPlugin"));

                    _plugin = pluginGetter();
                } catch (const PluginLoaderException &e) {
                    throw PluginLoaderException(e.what());
                }
                return *_plugin;
            }

            Plugin &getPlugin()
            {
                if (_plugin == nullptr) {
                    throw PluginLoaderExceptionNotLoaded("No library is loaded");
                }
                return *_plugin;
            }

            void unload()
            {
                if (_plugin == nullptr) {
                    return;
                }

                try {
                    auto pluginDeleter = reinterpret_cast<void (*)(Plugin *)>(getFunction("deletePlugin"));

                    pluginDeleter(_plugin);
                } catch (const PluginLoaderException &e) {
                    spdlog::error(e.what());
                }

                if (dlclose(_library) != 0) {
                    throw PluginLoaderException("An error occured while trying to close " + _path + ": " + dlerror());
                }
                _plugin = nullptr;
                _path = "";
                _library = nullptr;
            }

        private:
            [[nodiscard]] void *getFunction(const std::string &aFunctionName) const
            {
                if (_library == nullptr) {
                    throw PluginLoaderExceptionNotLoaded("No library is loaded");
                }

                void *function = dlsym(_library, aFunctionName.c_str());
                if (function == nullptr) {
                    throw PluginLoaderException("An error occured while trying to get " + aFunctionName + ": "
                                                + dlerror());
                }
                return function;
            }
    };
} // namespace Engine::Plugin

#endif /* !LIBRARYLOADER_HPP_ */
