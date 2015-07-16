#include <iostream>
#include <vector>
#include <cassert>
#include <ctime>


//=================================================================================================
// original version
//
int Bcd2IntCheckingOverflow1(const char* src, std::size_t len, bool* overflow)
{
    int result;
    size_t wLen;

    if (overflow != NULL)
        *overflow = false;

    wLen = (len + 1) / 2;
    for (result = 0; wLen != 0; wLen--, src++)
    {
        if (*src != 0)
            result = (result * 100) + (((*src & 0xF0) >> 1) + ((*src & 0xF0) >> 3)) + (*src & 0x0F);
        else
        {
            if (overflow != NULL && result > INT_MAX / 100)
                *overflow = true;
            result *= 100;
        }
    }

    return result;
}

//=================================================================================================
// my version
//
int Bcd2IntCheckingOverflow2(const char* src, std::size_t len, bool* overflow)
{
    int rv = 0;
    for (len = len + (len % 2); len != 0; ++src, len -= 2)
    {
        static auto const MaxIntDiv100 = INT_MAX / 100;
        static auto const MaxIntMod100_BCD = static_cast<char>(INT_MAX % 100 / 10 * 16 + INT_MAX % 10);
        if (rv > MaxIntDiv100 || (rv == MaxIntDiv100 && *src > MaxIntMod100_BCD))
        {
            if (overflow) { *overflow = true; }
            return 0;
        }

        rv = 10 * rv + ((*src >> 4) & 0x0f);
        rv = 10 * rv + ((*src >> 0) & 0x0f);
    }
    if (overflow) { *overflow = false; }
    return rv;
}

//=================================================================================================
auto Test(std::vector<char> const& src) -> bool
{
    bool overflow1, overflow2;

    std::size_t len1 = src.size()*2;
    std::size_t len2 = src.size()*2;
    if (len1 > 0 && (rand()%2==0)) { --len1; }
    if (len2 > 0 && (rand()%2==0)) { --len2; }
    auto const result1 = Bcd2IntCheckingOverflow1(src.data(), len1, &overflow1);
    auto const result2 = Bcd2IntCheckingOverflow2(src.data(), len2, &overflow2);
    
    auto const rv = (overflow1 && overflow2) || (!overflow1 && !overflow2 && result1 == result2);
    assert(rv);
    return rv;
}

//=================================================================================================
auto DoTests() -> bool
{
    bool rv = true;

    rv &= Test(std::vector<char> { });
    rv &= Test(std::vector<char> { '\x00' });
    rv &= Test(std::vector<char> { '\x01' });
    rv &= Test(std::vector<char> { '\x01' });
    rv &= Test(std::vector<char> { '\x10' });
    rv &= Test(std::vector<char> { '\x11' });
    rv &= Test(std::vector<char> { '\x99' });
    rv &= Test(std::vector<char> { '\x98' });
    rv &= Test(std::vector<char> { '\x09' });
    rv &= Test(std::vector<char> { '\x76' });
    rv &= Test(std::vector<char> { '\x00', '\x00' });
    rv &= Test(std::vector<char> { '\x01', '\x01' });
    rv &= Test(std::vector<char> { '\x10', '\x10' });
    rv &= Test(std::vector<char> { '\x11', '\x11' });
    rv &= Test(std::vector<char> { '\x99', '\x99' });
    rv &= Test(std::vector<char> { '\x98', '\x98' });
    rv &= Test(std::vector<char> { '\x09', '\x09' });
    rv &= Test(std::vector<char> { '\x76', '\x76' });
    rv &= Test(std::vector<char> { '\x00', '\x00', '\x00' });
    rv &= Test(std::vector<char> { '\x01', '\x01', '\x01' });
    rv &= Test(std::vector<char> { '\x10', '\x10', '\x10' });
    rv &= Test(std::vector<char> { '\x11', '\x11', '\x11' });
    rv &= Test(std::vector<char> { '\x99', '\x99', '\x99' });
    rv &= Test(std::vector<char> { '\x98', '\x98', '\x98' });
    rv &= Test(std::vector<char> { '\x09', '\x09', '\x09' });
    rv &= Test(std::vector<char> { '\x76', '\x76', '\x76' });

    return rv;
}

//=================================================================================================
auto main() -> int
{    
    srand(static_cast<unsigned int>(time(0)));
    std::cout << (DoTests() ? "all tests passed" : "one or more tests failed") << std::endl;
    return 0;
}

