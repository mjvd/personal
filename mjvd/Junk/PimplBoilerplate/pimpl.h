#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// VS2012 compiler does not support variadic templates, but the Nov CTP complier does.  Modify
// the #define below appropriately.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#define COMPILER_SUPPORTS_VARIADIC_TEMPLATES


#include <memory>

template<typename T>
class pimpl 
{
public:
#ifdef COMPILER_SUPPORTS_VARIADIC_TEMPLATES
    template<typename ...Args> pimpl(Args&& ...);
#else
    pimpl();
    template<typename Arg1>                                                             pimpl(Arg1&&);
    template<typename Arg1, typename Arg2>                                              pimpl(Arg1&&, Arg2&&);
    template<typename Arg1, typename Arg2, typename Arg3>                               pimpl(Arg1&&, Arg2&&, Arg3&&);
    template<typename Arg1, typename Arg2, typename Arg3, typename Arg4>                pimpl(Arg1&&, Arg2&&, Arg3&&, Arg4&&);
    template<typename Arg1, typename Arg2, typename Arg3, typename Arg3, typename Arg5> pimpl(Arg1&&, Arg2&&, Arg3&&, Arg4&&, Arg5&&);
#endif
    pimpl(pimpl<T> const&);
    pimpl(pimpl<T>&&);
    pimpl<T>& operator= (pimpl<T> rhs);
    ~pimpl();

    T* operator->();
    T& operator*();

    void swap(pimpl<T>& rhs);

private:
    std::unique_ptr<T> m;
};


