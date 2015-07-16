#include <iostream>
#include <string>

//=================================================================================================
//
//  step 1 - start with natural numbers:    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, ...
//  step 2 - exchange multiples of 2:       1, 4, 3, 2, 5, 8, 7, 6, 9, 12, 11, 10, 13, 16, ...
//  step 3 - exchange multiples of 3:       1, 4, 8, 2, 5, 3, 7, 6, 10, 12, 11, 9, 13, 16, ...
//  step 4 - exchange multiples of 4:       1, 4, 8, 6, 5, 3, 7, 2, 10, 12, 11, 14, 13, 16, ...
//  step 5 - exchange multiples of 5:       1, 4, 8, 6, 12, 3, 7, 2, 10, 5, 11, 14, 13, 16, ...
//  step 6 - exchange multiples of 6:       1, 4, 8, 6, 12, 14, 7, 2, 10, 5, 11, 3, 13, 16, ...
//  continue to infinity
//
//
//  The Shotgun Number sequence is:
//      1, 4, 8, 6, 12, 14, 16, 9, 18, 20, 24, 26, 28, 22, 39, 15, 36, 35, 40, 38, 57, 34, 48, 49, 51, 44,
//      46, 33, 60, 77, 64, 32, 75, 56, 81, 68, 76, 58, 100, 55, 84, 111, 88, 62, 125, 70, 96, 91, 98, 95,
//      134, 72, 108, 82, 141, 80, 140, 92, 120, 156, 124, 94, 121, 52, 152, 145, ...
//
//
//  Note that after the N'th step, the first N numbers are permanently fixed.
//
//  Also note that the Shotgun Sequence contains no primes!?
//


//=================================================================================================
// trick is to work backwards.  
//    * We want to work out the Nth Shotgun number, so start at step_i where 'i' = N
//    * the current N will be swapped (moved) iff it is a multiple of 'i' (i.e.  n%i == 0)
//    * if the current N is swapped, it is either the lower or higher number to be swapped.  I.e.
//      it will either be moved 'i' steps higher, or it will be moved 'i' steps lower.
//    * To tell whether N is the higher or lower, simply divide by 'i' and check whether it is
//      a multiple of 2.  If it is, it is the higher, otherwise it is the lower
//    * continue to work backwards to step_1 - then you're all done.
//
auto NthShotgunNumber(long long n) -> long long
{
    for (auto i=n; i>1; --i)
    {
        if (n%i == 0)
        {
            n += (n/i % 2 == 1) ? i : -i;
        }
    }
    return n;
}

//=================================================================================================
// read an integer greater than zero, N, from the command line.  Calculate the Nth 'Shotgun Number', and print to STDOUT
//
auto main(int argc, char** argv) -> int
{
    if (argc == 2)
    {
        auto const n = std::stoll(argv[1]);
        if (n > 0)
        {
            std::cout << NthShotgunNumber(n) << std::endl;
            return 0;
        }
    }
     
    std::cout << "Usage:  ShotgunNumber <n>" << std::endl;
    return 1;
}

