#pragma once
#include <dlfcn.h>
#include <stdexcept>
#include <string>

namespace Arcade {

/// @brief RAII wrapper around dlopen / dlsym / dlclose.
///
/// Automatically closes the shared library when destroyed.
/// Non-copyable, movable.
class DLLoader {
public:
    /// @brief Opens a shared library at the given path.
    /// @param path Filesystem path to the shared library (.so).
    /// @throws std::runtime_error if dlopen fails.
    explicit DLLoader(const std::string& path)
        : _handle(dlopen(path.c_str(), RTLD_LAZY | RTLD_LOCAL))
    {
        if (!_handle) {
            throw std::runtime_error(std::string("dlopen(") + path + "): " + dlerror());
        }
    }

    /// @brief Closes the shared library handle.
    ~DLLoader()
    {
        if (_handle) {
            dlclose(_handle);
        }
    }

    DLLoader(const DLLoader&)            = delete;
    DLLoader& operator=(const DLLoader&) = delete;

    /// @brief Move-constructs a DLLoader, transferring ownership of the handle.
    /// @param other Source loader; its handle is set to nullptr after the move.
    DLLoader(DLLoader&& other) noexcept
        : _handle(other._handle)
    {
        other._handle = nullptr;
    }

    /// @brief Move-assigns a DLLoader, closing any currently held handle first.
    /// @param other Source loader; its handle is set to nullptr after the move.
    /// @return Reference to this.
    DLLoader& operator=(DLLoader&& other) noexcept
    {
        if (this != &other) {
            if (_handle) {
                dlclose(_handle);
            }
            _handle       = other._handle;
            other._handle = nullptr;
        }
        return *this;
    }

    /// @brief Looks up a symbol in the loaded library and returns it as FuncPtr.
    /// @tparam FuncPtr Function-pointer type to cast the symbol to.
    /// @param name Symbol name to look up.
    /// @return The symbol cast to FuncPtr.
    /// @throws std::runtime_error if the symbol is not found.
    template<typename FuncPtr>
    FuncPtr getSymbol(const std::string& name) const
    {
        dlerror();
        void* sym       = dlsym(_handle, name.c_str());
        const char* err = dlerror();
        if (err) {
            throw std::runtime_error(std::string("dlsym(") + name + "): " + err);
        }
        FuncPtr fp;
        *reinterpret_cast<void**>(&fp) = sym;
        return fp;
    }

    /// @brief Checks whether a symbol exists in the loaded library.
    /// @param name Symbol name to probe.
    /// @return true if the symbol resolves without error, false otherwise.
    bool hasSymbol(const std::string& name) const
    {
        dlerror();
        dlsym(_handle, name.c_str());
        return dlerror() == nullptr;
    }

private:
    void* _handle = nullptr;
};

} // namespace Arcade
