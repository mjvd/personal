#include <string>
#include <iostream>
#include <conio.h>


// given a parameter of some type, simply return the type as a string
std::string TypeOf(char)                { return "char";                }
std::string TypeOf(signed char)         { return "signed char";         }
std::string TypeOf(unsigned char)       { return "unsigned char";       }
std::string TypeOf(short)               { return "short";               }
std::string TypeOf(unsigned short)      { return "unsigned short";      }
std::string TypeOf(int)                 { return "int";                 }
std::string TypeOf(unsigned int)        { return "unsigned int";        }
std::string TypeOf(long)                { return "long";                }
std::string TypeOf(unsigned long)       { return "unsigned long";       }
std::string TypeOf(long long)           { return "long long";           }
std::string TypeOf(unsigned long long)  { return "unsigned long long";  }


// display the value and type of a given parameter
template <typename T> void Display(T t)
{
    std::cout << "Value[" << t << "]   type[" << TypeOf(t) << "]" << std::endl;
}


int main()
{
    // we're talking 32-bit MS Windows build
    static_assert(sizeof(int)       == 4, "");
    static_assert(sizeof(long)      == 4, "");
    static_assert(sizeof(long long) == 8, "");


    //===============================================================================================================================================
    //
    //  UNADORNED DECIMAL INTEGER LITERALS:
    //
    //  according to C++11 standard, for unadorned decimal integer literals, results are well defined for the range [0..9223372036854775807], and 
    //  the resulting type is the first in { int, long, long long } that can represent that literal.
    //
    //  VS2012-CTP has a bug where the range [2147483648..4294967295] is incorrectly assigned as 'unsigned long' rather than 'long long'
    //
    Display(0);                     // int                  NOTE:  this is technically an octal integer literal
    Display(2147483647);            // int
    Display(2147483648);            // long long            NOTE:  complier bug here incorrectly assigns type 'unsigned long'
    Display(4294967295);            // long long            NOTE:  complier bug here incorrectly assigns type 'unsigned long'
    Display(4294967296);            // long long
    Display(9223372036854775807);   // long long
    Display(9223372036854775808);   // undefined behaviour  NOTE:  compiler assigns the type 'unsigned long long'
    Display(18446744073709551615);  // undefined behaviour  NOTE:  compiler assigns the type 'unsigned long long'
  //Display(18446744073709551616);  // undefined behaviour  NOTE:  compiler error 'constant too big'



    //===============================================================================================================================================
    //
    //  DECORATED DECIMAL INTEGER LITERALS:
    //
    //  For literals ending in 'u', C++11 defines type as first in { unsigned int, unsigned long, unsigned long long } that can represent the value.
    //
    //  VS2012-CTP seems to do things correctly.
    //
    Display(0u);                    // unsigned int         NOTE:  octal integer literal
    Display(4294967295u);           // unsigned int
    Display(4294967296u);           // unsigned long long
    Display(18446744073709551615u); // unsigned long long
  //Display(18446744073709551616u); // undefined behaviour  NOTE:  we get a compiler error



    //===============================================================================================================================================
    //
    //  NEGATIVE (?) DECIMAL INTEGER LITERALS:
    //
    //  in C++11, there is no such thing as a negative integer literal.  The above lines of code are actually comprised of the unary minus operator
    //  being applied to a positive integer literal.  this can have surprising results...  Trying to Display(-xxx) will work fine (according to C++11)
    //  for all literals in the range [0..9223372036854775807], though the type of '-2147483648' might be unexpected (long long, instead of int).
    //
    //  VS2012-CTP incorrectly treats literals in the range [2147483648..4294967295] as having type 'unsigned int', and so displays odd (but perfectly
    //  well defined) values for these literals after applying the unary minus operator.
    //
    Display(-0);                    // int
    Display(-1);                    // int
    Display(-2147483647);           // int
    Display(-2147483648);           // long long            NOTE:  compiler bug treats this as 'unsigned long', then applies the unary minus operator to it
    Display(-4294967296);           // long long
    Display(-9223372036854775807);  // long long
  //Display(-9223372036854775808);  // undefined behaviour  NOTE:  compiler error



    //===============================================================================================================================================
    //
    //  OCTAL/HEX INTEGER LITERALS:
    //
    //  Octal and hexadecimal integer literals are treated differently by C++11.  The type assigned by the compiler is the first type in the following
    //  set which is capable of representing the literal;  { int, unsigned int, long, unsigned long, long long, unsigned long long }
    //
    //  VS2012-CTP does things correctly here.
    //
    Display(2147483648);            // long long            NOTE:  compiler bug assigns type 'unsigned int'
    Display(0x80000000);            // unsigned int
    Display(9223372036854775808);   // undefined behaviour  NOTE:  compiler assigns type 'unsigned long long'
    Display(0x8000000000000000);    // unsigned long long



    //===============================================================================================================================================
    //
    //  INTEGRAL PROMOTION:
    //
    //  applying the unary minus operator to a signed type is pretty obvious.
    //  applying the unary minus operator to an unsigned type is perfectly well defined.  VS2014-CTP gives a warning.  To work out the results
    //  of this operation:
    //    1) integral promotion is first applied to the operand to at least the rank 'int' or 'unsigned int'
    //    2) calculation is simply:  2^n - <promoted operand>   where 'n' is the bit-size of the promoted operand
    //
    //  This has the surprising effect that the negation of a signed char/short will always be promoted to an integer.
    //
    Display(-static_cast<char>              (42));  // promoted to 'int'
    Display(-static_cast<unsigned char>     (42));  // promoted to 'int'
    Display(-static_cast<short>             (42));  // promoted to 'int'
    Display(-static_cast<unsigned short>    (42));  // promoted to 'int'
    Display(-static_cast<int>               (42));  // unchanged as 'int'
    Display(-static_cast<unsigned int>      (42));  // unchanged as 'unsigned int'
    Display(-static_cast<long>              (42));  // unchanged as 'long'
    Display(-static_cast<unsigned long>     (42));  // unchanged as 'unsigned long'
    Display(-static_cast<long long>         (42));  // unchanged as 'long long'
    Display(-static_cast<unsigned long long>(42));  // unchanged as 'unsigned long long'



    //===============================================================================================================================================
    //
    //  UNARY MINUS AND SIGNED OVERFLOW:
    //
    //  In C++11, unsigned integer overflow is well-defined and does what you would expect.  Signed integer overflow, however, is undefined behaviour.
    //  The rules for unary minus applied to a signed types are similar to unsigned types:
    //    1) if the operand's type has a rank lower than int, then it is promoted 
    //    2) the value is negated as expected.  If this results in an overflow (i.e. operand is the negative-most value of that signed type, then 
    //       the results are undefined.
    //
    signed char x1 = -127 - 1;
    Display(x1);
    Display(-x1);                            // promoted to 'int'
    short x2 = -32767 - 1;
    Display(x2);
    Display(-x2);                            // promoted to 'int'
    int x3 = -2147483647 - 1;
    Display(x3);
    Display(-x3);                            // undefined behaviour
    long x4 = -2147483647 - 1;
    Display(x4);
    Display(-x4);                            // undefined behaviour
    long long x5 = -9223372036854775807 - 1;
    Display(x5);
    Display(-x5);                            // undefined behaviour


    std::cout << "\n\n\n\nPress any key to exit..." << std::endl;
    _getch();
    return 0;
}
