#define _SCL_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>

#define NOMINMAX
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include <boost/multiprecision/gmp.hpp>
namespace mp = boost::multiprecision;


namespace
{
    // currently limited to 2^32-1 digits
    unsigned long long const MAX_DIGITS = 4294967295;


    ///////////////////////////////////////////////////////////////////////////////////////////////
    std::string ElapsedTime(DWORD start, DWORD end)
    {
        std::ostringstream oss;
        oss << ((end-start)/1000) << "." << std::setw(3) << std::setfill('0') << ((end-start)%1000) << " seconds";
        return oss.str();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    mp::mpz_int IntSqrt(mp::mpz_int const& n, mp::mpz_int const& one)
    {
        mp::mpz_int n_one = n*one;
        mp::mpz_int x = n / 2;
        while (true)
        {
            mp::mpz_int x_old = x;
            x = (x+ n_one/x) / 2;
            if (x == x_old) { break; }
        }
        return x;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    mp::mpz_int MySqrt(unsigned long long n, unsigned long long digits)
    {
        mp::mpf_float::default_precision(static_cast<unsigned int>(digits));
        mp::mpf_float const sqrt_n_float = sqrt(mp::mpf_float(n));
        mp::mpz_int sqrt_n_int;
        sqrt_n_int.assign(sqrt_n_float * pow(mp::mpf_float(10), digits));
        return sqrt_n_int;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    mp::mpz_int CalculatePi(unsigned long long digits)
    {
        unsigned long long ONE_PERCENT = std::max(digits / 14 / 100, 1ull);
        std::cout << "\n\n|-------------------------------------------------------------------------------------------------|" << std::endl;

        mp::mpz_int const ONE = pow(mp::mpz_int(10), static_cast<unsigned int>(digits));
        mp::mpz_int const MY_CONST = pow(mp::mpz_int(640320), 3) / 24;

        mp::mpz_int k      = 1;
        mp::mpz_int a_k    = ONE;
        mp::mpz_int a_sum  = ONE;
        mp::mpz_int b_sum  = 0;

        unsigned long long iterations = 0;
        while (!a_k.is_zero())
        {
            if ((++iterations % ONE_PERCENT) == 0) { std::cout << "." << std::flush; }

            a_k *= -(6*k-5) * (2*k-1) * (6*k-1);
            a_k /= k*k*k*MY_CONST;
            a_sum += a_k;
            b_sum += k * a_k;
            ++k;
        }
        std::cout << "\n\n\n" << std::flush;

        mp::mpz_int const total = 13591409*a_sum + 545140134*b_sum;
        mp::mpz_int const sqrt_10005 = MySqrt(10005, digits);
        mp::mpz_int const pi = 426880 * sqrt_10005 * ONE / total;
        return pi;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    void BinarySplit(unsigned long long a, unsigned long long b, mp::mpz_int& P, mp::mpz_int& Q, mp::mpz_int& T)
    {
        static mp::mpz_int const C3_OVER_24 = pow(mp::mpz_int(640320), 3) / 24;

        if (b-a == 1)
        {
            P = (a == 0) ? mp::mpz_int(1) : (6*a-5) * (2*a-1) * (6*a-1);
            Q = (a == 0) ? mp::mpz_int(1) : a*a*a*C3_OVER_24;
            T = ((a & 1) ? -1 : 1) * P * (13591409 + 545140134*a);
        }
        else
        {
            unsigned long long const m = (a+b)/2;

            mp::mpz_int Pam, Qam, Tam;
            mp::mpz_int Pmb, Qmb, Tmb;
            BinarySplit(a, m, Pam, Qam, Tam);
            BinarySplit(m, b, Pmb, Qmb, Tmb);

            P = Pam * Pmb;
            Q = Qam * Qmb;
            T = (Qmb * Tam) + (Pam * Tmb);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    mp::mpz_int CalculatePi2(unsigned long long digits)
    {
        // calculate how many terms we need to sum up for the required accuracy
        double const DIGITS_PER_TERM = std::log10(pow(640320.0, 3) / 24 / 6 / 2 / 6);
        unsigned long long const NUM_TERMS = static_cast<unsigned long long>(digits / DIGITS_PER_TERM + 1);
        std::cout << "    summing " << NUM_TERMS << " terms of the Chudnovsky series      " << std::flush;

        // calculate P, Q, and T using binary splitting method
        DWORD const start = GetTickCount();
        mp::mpz_int P, Q, T;
        BinarySplit(0, NUM_TERMS, P, Q, T);
        DWORD const end1 = GetTickCount();
        std::cout << ElapsedTime(start, end1) << std::endl;
        
        // final calculation to produce pi
        std::cout << "    final calculation                                " << std::flush;
        mp::mpz_int const sqrt_10005 = MySqrt(10005, digits);
        mp::mpz_int const pi = Q * 426880 * sqrt_10005 / T;
        DWORD const end2 = GetTickCount();
        std::cout << ElapsedTime(end1, end2) << std::endl;
        return pi;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    try
    {
        if (sizeof(int*) != 8)
        {
            std::cout << "ERROR: this program must be compiled in x64 mode" << std::endl;
            return EXIT_FAILURE;
        }

        unsigned long long digits = (argc >= 2) ? std::stoull(argv[1]) : 0;
        std::string const output_file = (argc >= 3) ? argv[2] : "";

        if (argc != 3 || digits < 1 || digits > MAX_DIGITS)
        {
            std::cout 
                << "\n    USAGE:  pi_mvd <digits> <output_file>"
                << "\n"
                << "\n    where <digits> is any number between 1 and " << MAX_DIGITS
                << "\n    and <output_file> is where the results will be written to."
                << "\n"
                << "\n"
                << "\n    Author:       Mark van Dijk"
                << "\n    Date:         2013-May-23"
                << "\n    Description:  This is an extremely simple PI calculator built in C++ on"
                << "\n                  top of boost::multiprecision (with MPIR backend).  It uses"
                << "\n                  the famous Chudnovsky formula (with binary splitting)."
                << "\n"
                << "\n"
                << std::endl;

            return EXIT_FAILURE;
        }

        std::cout << "    calculating " << digits << " digits of pi" << std::endl;
        DWORD const t1 = GetTickCount();
        mp::mpz_int const pi = CalculatePi2(digits + 20);
        
        std::cout << "    converting to base-10                            " << std::flush;
        DWORD const t2 = GetTickCount();
        std::string const pi_str = pi.str().erase(digits+1);
        DWORD const t3 = GetTickCount();
        std::cout << ElapsedTime(t2, t3) << std::endl;

        std::cout << "    creating output file                             " << std::flush;
        std::ofstream f(output_file, std::ios::trunc);
        f << "3.";
        for (std::size_t i=1; i<pi_str.size(); i+=100)
        {
            f << "\n" << pi_str.substr(i, 100);
        }
        DWORD const t4 = GetTickCount();
        std::cout << ElapsedTime(t3, t4) << std::endl;
        f << "\n\ncalculation completed in " << ElapsedTime(t1, t3) << std::endl;
        std::cout 
            << "    -----------------------------------------------------------------------"
            << "\n    finished                                         " << ElapsedTime(t1, t4) << "\n\n" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cout << "main():  std::exception caught: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "main():  unknown exception caught" << std::endl;
        return EXIT_FAILURE;
    }
}


