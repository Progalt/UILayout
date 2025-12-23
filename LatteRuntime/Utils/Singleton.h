
#ifndef LATTE_SINGLETON_H
#define LATTE_SINGLETON_H

#include <memory>
#include <mutex>

template <typename T>
class Singleton 
{
public:
    // Delete copy constructor and assignment operator
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

    // Get the singleton instance
    static T& getInstance() 
    {
        std::call_once(initFlag, &Singleton::initSingleton);
        return *instance;
    }

protected:
    Singleton() = default;
    virtual ~Singleton() = default;

private:
    static std::unique_ptr<T> instance;
    static std::once_flag initFlag;

    static void initSingleton() 
    {
        instance = std::make_unique<T>();
    }
};

// Static member definitions
template <typename T>
std::unique_ptr<T> Singleton<T>::instance = nullptr;

template <typename T>
std::once_flag Singleton<T>::initFlag{};

#endif // LATTE_SINGLETON_H