#pragma once
#include <utility>

#ifdef COMPILER_SUPPORTS_VARIADIC_TEMPLATES

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
template<typename ...Args>
pimpl<T>::pimpl(Args&& ...args) : m(new T(std::forward<Args>(args)...)) {}

#else

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
pimpl<T>::pimpl() : m(new T) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename Arg1>
pimpl<T>::pimpl(Arg1&& arg1) : m(new T(std::forward<Arg1>(arg1))) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
template<typename Arg1, typename Arg2>
pimpl<T>::pimpl(Arg1&& arg1, Arg2&& arg2) : m(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2))) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
template<typename Arg1, typename Arg2, typename Arg3>
pimpl<T>::pimpl(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3) : m(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3))) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>
pimpl<T>::pimpl(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4) : m(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), std::forward<Arg4>(arg4))) {}

///////////////////////////////////////////////////////////////////////////////////////////////////
template <typename T>
template <typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
pimpl<T>::pimpl(Arg1&& arg1, Arg2&& arg2, Arg3&& arg3, Arg4&& arg4, Arg5&& arg5) : m(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), std::forward<Arg4>(arg4), std::forward<Arg5>(arg5))) {}

#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>        pimpl<T>::~pimpl()                                      {}
template<typename T>        pimpl<T>::pimpl(pimpl<T> const& rhs) : m(new T(*rhs.m)) {}
template<typename T>        pimpl<T>::pimpl(pimpl<T>&& rhs) : m(std::move(rhs.m))   {}
template<typename T>        pimpl<T>& pimpl<T>::operator=(pimpl<T> rhs)             { swap(rhs); return *this; }
template<typename T> T*     pimpl<T>::operator->()                                  { return  m.get(); }
template<typename T> T&     pimpl<T>::operator* ()                                  { return *m.get(); }
template<typename T> void   pimpl<T>::swap(pimpl<T>& rhs)                           { m.swap(rhs.m); }


