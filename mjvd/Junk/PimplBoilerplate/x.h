#pragma once
#include "pimpl.h"

class X
{
public:
    auto foo() -> void;
    auto bar() -> void;


    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // WARNING:   VS2012 Nov CTP still does not correctly generate a default move constructor or 
    //            move assignment.  This is really annoying.  It means that we have to define our 
    //            own user-defined constructors/assignments that just do the obvious thing.  
    //
    //            hopefully this will be unnecessary in the next version of the compiler...
    //
    ///////////////////////////////////////////////////////////////////////////////////////////////


    //
    // TODO:  get rid of these default methods when the compiler properly supports default generated move constructors/assignments
    //
    X(double d, int i);

    X();
    X(X const& rhs);
    X(X&& rhs);
    auto operator=(X const& rhs) -> X&;
    auto operator=(X&& rhs) -> X&;

private:
    class Impl;
    pimpl<Impl> impl;
};