#ifndef _SINGLETON_H_
#define _SINGLETON_H_

namespace mtpython {

template <typename T> class Singleton {
public:
    Singleton() { m_singleton = static_cast<T*>(this); }

    Singleton(const Singleton<T>&) = delete;
    Singleton<T>& operator=(const Singleton<T>&) = delete;

    ~Singleton() { m_singleton = nullptr; }

    static T& get_singleton() { return *m_singleton; }

    static T* get_singleton_ptr() { return m_singleton; }

protected:
    static T* m_singleton;
};

} // namespace mtpython

#endif
