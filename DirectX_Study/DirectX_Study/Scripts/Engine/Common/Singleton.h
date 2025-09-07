#pragma once

#include <mutex>

namespace DK
{
    template <class T>
    class Singleton
    {
    protected:
        Singleton() {}
        ~Singleton() {}

    public:
        Singleton(Singleton<T>&) = delete;
        void operator=(const Singleton<T>&) = delete;

        static T* GetInstance();

    private:
        //static T* m_instance;
        static std::mutex m_mutex;
    };

    /*template <class T>
    T* Singleton<T>::m_instance = nullptr;*/

    template <class T>
    std::mutex Singleton<T>::m_mutex;

    template <class T>
    T* Singleton<T>::GetInstance()
    {
        static T instance;
        return &instance;
    }

}