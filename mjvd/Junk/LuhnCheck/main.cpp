#include <iostream>
#include <string>
#include <cassert>
#include <sstream>
#include <ctime>

#define WIN32_EXTRA_LEAN
#include <windows.h>
#include <conio.h>

//  increase frequency of the system timer interrupt to 1kHz (up from a default of around 64Hz).
//  This will increase the precision of various Windows timing API functions, giving us true
//  millisecond timestamps in the log files.
//
#pragma comment(lib, "winmm.lib")
MMRESULT result = timeBeginPeriod(1);

//=================================================================================================
//  1) double every second digit (starting from the right).  If result is > 9 then subtract 9
//  2) sum the resulting digits
//  3) if sum is a multiple of 10, then the number passes the Luhn test
//
bool CheckLuhn(std::string const& cardNum)
{
    auto sum = 0;
    auto shift = 1 - (cardNum.size() % 2);
    for (auto const c : cardNum)
    {
        auto const digit = (c-'0') << shift;
        sum += digit>9 ? digit-9 : digit;
        shift = 1 - shift;
    }
    return sum % 10 == 0 && cardNum.size() > 1;
}

//=================================================================================================
bool CheckLuhn2(std::string const& mPan)
{
    long next, eluhn, cluhn = 0;
    long dig, multiplier = 2;
    const char* pan = mPan.c_str();
    long pan_length = mPan.size();
    if (pan_length <= 1)
        return false;
    next = pan_length - 1;
    eluhn = pan[next] & 0x0f;
    do
    {
        dig = (pan[--next] & 0x0f) * multiplier;
        if (dig > 9)
            dig -= 9;
        cluhn = (cluhn + dig) % 10;
        multiplier = (multiplier == 2) ? 1 : 2;
    } while (next);
    cluhn = (10 - cluhn) % 10;
    return (eluhn == cluhn);
}

//=================================================================================================
int main()
{

    auto const s = std::string{ "11223344" };
    auto const pos = s.find_first_not_of("0123456789");
    auto const rv = s.substr(0, pos);
    assert(rv == "11223344");

    int const ITERATIONS = 2000000;

    {
        int sum = 0;
        for (int i=0; i<1000; ++i)
        {
            if (CheckLuhn(std::to_string(1234567890123456789 + i))) { ++sum; }
        }
        std::cout << "preamble..." << sum << "\n" << std::endl;
    }

    for (int i=0; i<5; ++i)
    {
        {
            DWORD start = GetTickCount();
            int sum = 0;
            for (int i=0; i<ITERATIONS; ++i)
            {
                if (CheckLuhn(std::to_string(1234567890123456789 + i))) { ++sum; }
            }
            DWORD end = GetTickCount();
            std::cout << "CheckLuhn:   " << ITERATIONS << " iterations:  sum=" << sum << ":  " << (end - start) << " milliseconds" << std::endl;
        }
    }

    std::cout << "\n";
    for (int i = 0; i < 5; ++i)
    {
        DWORD start = GetTickCount();
        int sum = 0;
        for (int i = 0; i < ITERATIONS; ++i)
        {
            if (CheckLuhn2(std::to_string(1234567890123456789 + i))) { ++sum; }
        }
        DWORD end = GetTickCount();
        std::cout << "CheckLuhn2:  " << ITERATIONS << " iterations:  sum=" << sum << ":  " << (end - start) << " milliseconds" << std::endl;
    }

    _getch();
    return 0;

    srand(time(0));
    for (int i=0; i<1000000; ++i)
    {
        auto const panLength = (rand()%7) + 13;
        assert(panLength >= 13 && panLength <= 19);

        std::ostringstream oss;
        for (int j=0; j<panLength; ++j)
        {
            oss << rand()%10;
        }
        auto const pan = oss.str();

        if (CheckLuhn(pan) != CheckLuhn2(pan))
        {
            assert(false);
        }
    }
    return 0;

    std::string const card = "4111111111111111";
    std::cout << card << ":  " << (CheckLuhn(card) ? "passed" : "failed") << " Luhn check" << std::endl;
}

