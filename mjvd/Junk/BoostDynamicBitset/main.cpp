#include <iostream>
#include <boost/dynamic_bitset.hpp>
#include <fstream>

#define WIN32_EXTRA_LEAN
#include <windows.h>


int main()
{
    boost::dynamic_bitset<> bs(1000000000);
    bs[0] = 1;
    bs[9999999] = 1;
    if (bs.size() % (sizeof (boost::dynamic_bitset<>::block_type)*8) != 0)
    {
        bs.resize(bs.size() + (sizeof(boost::dynamic_bitset<>::block_type) * 8) - (bs.size() % (sizeof (boost::dynamic_bitset<>::block_type)*8)));
    }



    /*
    {
        //
        // write out as a string of '0' and '1' characters to a file.  works, but uses 8x too much memory
        //
        std::ofstream of("d:\\temp\\delete_me.bin", std::ios::trunc | std::ios::binary);
        of << bs;
        of.close();

        boost::dynamic_bitset<> bs2;
        std::ifstream is("d:\\temp\\delete_me.bin", std::ios::binary);
        is >> bs2;
        assert(bs == bs2);
    }
    */
     

    {
        //
        // convert bitset to a raw vector, then dump out the vector to a file.  works, but uses a temporary vector
        //
        DWORD start1 = GetTickCount();
        std::vector<boost::dynamic_bitset<>::block_type> blocks;
        boost::to_block_range(bs, std::back_inserter(blocks));
        std::ofstream of("d:\\temp\\delete_me.bin", std::ios::trunc | std::ios::binary);
        of.write(reinterpret_cast<char const*>(&blocks[0]), blocks.size() * sizeof (boost::dynamic_bitset<>::block_type));
        of.close();
        DWORD end1 = GetTickCount();
        std::cout << "exported in " << (end1 - start1) << " milliseconds" << std::endl;

        DWORD start2 = GetTickCount();
        std::ifstream is("d:\\temp\\delete_me.bin", std::ios::binary);
        std::istream_iterator<unsigned char> dataBegin(is);
        std::istream_iterator<unsigned char> dataEnd;
        std::vector<unsigned char> const blocks2(dataBegin, dataEnd);
        DWORD end2 = GetTickCount();
        std::cout << "imported file in " << (end2 - start2) << " milliseconds" << std::endl;

        DWORD start3 = GetTickCount();
        boost::dynamic_bitset<>::block_type const* b1 = reinterpret_cast<boost::dynamic_bitset<>::block_type const*>(&blocks2[0]);
        boost::dynamic_bitset<>::block_type const* b2 = b1 + blocks2.size() / sizeof (boost::dynamic_bitset<>::block_type);
        boost::dynamic_bitset<> bs2(b1, b2);
        DWORD end3 = GetTickCount();
        std::cout << "constructed bitset in " << (end3 - start3) << " milliseconds" << std::endl;

        //std::cout << "\n" << bs2 << std::endl;
        std::cout << (bs == bs2 ? "ok" : "failed") << std::endl;
    }
    return 0;
}



