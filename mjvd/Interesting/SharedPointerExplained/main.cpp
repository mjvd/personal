#include <iostream>
#include <memory>


struct X 
{
    int* xMember;

    X() : xMember(new int(33)) {}
    ~X() { delete xMember; }
};

struct Base {};
struct Derived : Base {};

int main()
{
    // std::shared_ptr<> maintains TWO internal pointers:
    //    * stored pointer:  this is what it is actually pointing to, and is what you get back by deferencing the shared_ptr<>
    //    * owned pointer:   this is a pointer to the resource whose lifetime is being shared/managed
    //
    // usually these two pointers point to the same thing.  But one of the shared_ptr<> constructors will create
    // something known as an 'aliased' shared_ptr - where these two internal pointers will actually point to different
    // things.
    //
    // When the two internal pointers point to the same object (i.e. the normal case), there are two possibilities for
    // the state of the shared_ptr<> - this being either 'null' or 'non-null'.  Examples follow:
    //
    std::shared_ptr<int> sp1;                       // null shared_ptr<>
    std::shared_ptr<int> sp2(new int(42));          // non-null shared_ptr<>



    // when we use aliasing, however, things get a bit more complex.  there are now four different states that the 
    // shared_ptr<> can find itself:
    //
    //    * empty and null
    //    * empty and non-null          // not very useful.  this basically acts as a normal non-owning raw pointer
    //    * non-empty and null          // allows you to manage lifetime of object - but disallows any access to it
    //    * non-empty and non-null      // possibly useful - one (marginal) use-case presented below...
    //
    // The concept of 'empty-ness' refers to the internal owned pointer.  I.e. that object that will be deleted when 
    // all shared_ptr's to that object have been destroyed.
    //
    // The concept of 'null-ness' refers to the internal stored pointer.  I.e. the actual pointer returned when you
    // call get() or operator->()
    //
    int* pi_null = 0;
    int* pi = new int(42);

    //std::shared_ptr<int> sp3(sp1, nullptr);       // empty and null - note:  due to bug in MSVC, this is an ambiguous call...
    std::shared_ptr<int> sp3(sp1, pi_null);         // empty and null
    std::shared_ptr<int> sp4(sp1, pi);              // empty and non-null
    std::shared_ptr<int> sp5(sp2, pi_null);         // non-empty and null
    std::shared_ptr<int> sp6(sp2, pi);              // non-empty and non-null



    // so what possible use are these 'aliased' shared pointers?  Well, take the 'X' structure above.  Say you
    // really want a shared_ptr<> to the internal X::xMember object - but you want the shared pointer to actually
    // manage the lifetime of the overall X object:
    //
    std::shared_ptr<X> sp7(new X);
    std::shared_ptr<int> sp8(sp7, sp7->xMember);



    // you can now use sp8 just as if it were a pointer directly to sp7->xMember, without worrying about it 
    // being destroyed behind your back.  Even when sp7 is reset(), the X object remains alive.  When sp8
    // is finally destroyed, only then will the X object itself be destroyed.
    //
    sp7.reset();        // the X object remains alive
    *sp8 = 1234;        // we can still directly modify the integer member of the X object
    sp8.reset();        // this destroys the X object
}

