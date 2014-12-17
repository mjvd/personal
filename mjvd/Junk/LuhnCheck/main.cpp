#include <iostream>
#include <string>

//=================================================================================================
//  1) double every second digit (starting from the right).  If result is > 9 then subtract 9
//  2) sum the resulting digits
//  3) if sum is a multiple of 10, then the number passes the Luhn test
//
bool CheckLuhn(std::string const& cardNum)
{
    auto val = 0;
    auto shift = 1 - (cardNum.size() % 2);
    for (auto c : cardNum)
    {
        auto i = (c-'0') << shift;
        val += i>9 ? i-9 : i;
        shift = 1 - shift;
    }
    return val % 10 == 0 && cardNum.size() > 1;
}

//=================================================================================================
int main()
{
    std::string const card = "4111111111111111";
    std::cout << card << ":  " << (CheckLuhn(card) ? "passed" : "failed") << " Luhn check" << std::endl;
}

