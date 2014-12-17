#include "x.h"
#include "pimpl_implementation.h"
#include <windows.h>


class YYY
{
public:
    YYY() { Sleep(2000); }
    YYY(YYY const&) { Sleep(2000); }
    YYY& operator=(YYY const& rhs) { Sleep(2000); }
};

class X::Impl
{
public:
    Impl(double d, int i) {}
    YYY y;    
};


auto X::foo() -> void {}
auto X::bar() -> void {}




//
// TODO:  get rid of these default methods when the compiler properly supports default generated move constructors/assignments
//
X::X(double d, int i) : impl(d, i) {}

X::X()              : impl(3.14159, 42)         {}
X::X(X const& rhs)  : impl(rhs.impl)            {}
X::X(X&& rhs)       : impl(std::move(rhs.impl)) {}
auto X::operator=(X const& rhs) -> X& { impl = rhs.impl; return *this; }
auto X::operator=(X&& rhs)      -> X& { impl = std::move(rhs.impl); return *this; }

