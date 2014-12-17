#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cassert>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <boost/assign.hpp>

#include "piece.h"

#include "CImg.h"
using namespace cimg_library;


namespace mm
{
    ULONGLONG appStartTime;
    long long twentyPrefix = 0;

    typedef std::vector<mm::Piece> Pieces;
    typedef std::vector<std::vector<mm::Piece const*>> PieceMap;

    unsigned char const 
        red[]    = { 255,   0 ,  0 },
        green[]  = {   0, 192,   0 },
        blue[]   = {   0,   0, 255 },
        yellow[] = { 255, 255,   0 },
        white[]  = { 255, 255, 255 },
        black[]  = {   0,   0,   0 };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto ToColour(char c) -> unsigned char const* const
    {
        switch (c)
        {
        case 'R': return red;
        case 'G': return green;
        case 'B': return blue;
        case 'Y': return yellow;
        case 'W': return white;
        case 'X': return black;
        default: assert(false); return black;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto ToColour(int c) -> unsigned char const* const
    {
        switch (c)
        {
        case 0: return red;
        case 1: return green;
        case 2: return blue;
        case 3: return yellow;
        case 4: return white;
        case 5: return black;
        default: assert(false); return black;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto DisplaySolution(Pieces const& pieces, PieceMap const& pieceMap, int maxX, int maxY) -> void
    {
        static int i = 0;
        static std::ofstream f("d:\\temp\\mm_results.txt", std::ios::trunc);
        
        ULONGLONG const t = GetTickCount64() - appStartTime;
        f << "\n\n    solution " << ++i << " found after [" << (t/1000) << "] seconds";
        std::cout << "\n    solution " << i << " found after [" << (t/1000) << "] seconds" << std::flush;

        for (int y=0; y!=maxY; ++y)
        {
            f << "\n        ";
            for (int x=0; x!=maxX; ++x)
            {
                Piece const* p = pieceMap[x][y];
                f << p->GetString() << " ";
            }
        }
        f << std::flush;

        /*
        int const squareSize = 10;
        CImg<unsigned char> img(2*maxX * squareSize + 1, 2*maxY * squareSize + 1, 1, 3, 255);
        
        for (int y=0; y!=maxY; ++y)
        {
            for (int x=0; x!=maxX; ++x)
            {
                Piece const* p = pieceMap[x][y];
                img.draw_rectangle((2*x+0) * squareSize, (2*y+0) * squareSize, (2*x+1) * squareSize - 1, (2*y+1) * squareSize - 1, ToColour(p->TL()));
                img.draw_rectangle((2*x+1) * squareSize, (2*y+0) * squareSize, (2*x+2) * squareSize - 1, (2*y+1) * squareSize - 1, ToColour(p->TR()));
                img.draw_rectangle((2*x+0) * squareSize, (2*y+1) * squareSize, (2*x+1) * squareSize - 1, (2*y+2) * squareSize - 1, ToColour(p->BL()));
                img.draw_rectangle((2*x+1) * squareSize, (2*y+1) * squareSize, (2*x+2) * squareSize - 1, (2*y+2) * squareSize - 1, ToColour(p->BR()));

                img.draw_line((2*x+0) * squareSize, (2*y+0) * squareSize, (2*x+2) * squareSize, (2*y+0) * squareSize, black);
                img.draw_line((2*x+0) * squareSize, (2*y+2) * squareSize, (2*x+2) * squareSize, (2*y+2) * squareSize, black);
                img.draw_line((2*x+0) * squareSize, (2*y+0) * squareSize, (2*x+0) * squareSize, (2*y+2) * squareSize, black);
                img.draw_line((2*x+2) * squareSize, (2*y+0) * squareSize, (2*x+2) * squareSize, (2*y+2) * squareSize, black);
            }
        }
        std::ostringstream oss;
        oss << "solution_" << maxX << "x" << maxY << "_" << std::setw(6) << std::setfill('0') << i << ".bmp";
        img.save_bmp(oss.str().c_str());
        */
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    auto FindSolutionsRecursive(Pieces& pieces, PieceMap& pieceMap, int maxX, int maxY, Position currentPosition) -> void
    {
        // do we have a complete solution?
        if (currentPosition.second == maxY)
        {
            DisplaySolution(pieces, pieceMap, maxX, maxY);
            return;
        }

        // Due to the symmetry of the colour arrangement on the pieces, each solution is actually just one of a set 
        // of 4 virtually identical solutions.  To get the other solutions, just apply any of the following transformations:
        //     1) flip horizontally
        //     2) flip vertically
        //     3) flip horizontally and vertically (i.e. rotate by 180 degrees)
        //
        // Additionally, for each of these 4 solutions, you can also re-arrange the colours in 24 different ways (4 factorial),
        // giving a total of 96 closely related solutions.
        // 
        // To greatly speed things up, there are ways to ensure that we only find one solution from each
        // of these sets of 96 solutions.
        //
        // Firstly, the colours.  We can find exactly ONE of the 24 combinations of colour arrangement for each solution
        // set by ensuring that the RRRR, GGGG, BBBB, and YYYY pieces are kept strictly in this order.  Every time we place
        // one of these 4 special pieces, we should check that their ordering is correct.
        //
        // Secondly, we can find exactly ONE of the horizontal OR vertical transformations by ensuring that the pieces
        // 'YRGB' and 'YRBG' are kept strictly in this order.  Note that this doesn't work if BOTH the horizontal/vertical
        // transformations have been applied - so we still need a third mechanism to detect this.
        //
        // Thirdly, we can eliminate the horizontal AND vertical transformation (which is effectively a 180 degree rotation)
        // by ensuring that the piece 'RRRR' remains in the FIRST HALF of the solution
        //
        //
        // Applying these 3 checks consistently will ensure that we find precisely ONE of each set of 96 solutions.


        // test 3:  ensure that RRRR is in the first half of the solution
        int const currentPieceNumber = currentPosition.second * maxX + currentPosition.first;
        if (currentPieceNumber == (pieces.size() / 2))
        {
            // we have now reached half way - check that the first piece 'RRRR' is used.  If so, we can continue, otherwise
            // we must back out now, for all solutions down this path will be duplicates.
            if (!pieces[0].IsUsed())
            {
                return;
            }
        }
        
        if (currentPieceNumber == 20)
        {
            if ((++twentyPrefix % 100000000) == 0)
            {
                std::cout << "found prefix [" << std::setw(12) << twentyPrefix << "]:  ";

                for (int y=0; y!=maxY; ++y)
                {
                    for (int x=0; x!=maxX; ++x)
                    {
                        if (y*maxX + x == 20) { goto loop_done; }
                        Piece const* p = pieceMap[x][y];
                        std::cout << p->GetString() << " ";
                    }
                }
                loop_done:
                std::cout << std::endl;
            }
            return;
        }

        // calculate the NEXT position
        Position nextPosition(currentPosition.first + 1, currentPosition.second);
        if (nextPosition.first == maxX)
        {
            nextPosition.first = 0;
            ++nextPosition.second;
        }

        // identify the pieces directly above and directly to the left of our current piece (may not exist)
        Piece const* pAbove = currentPosition.second == 0 ? nullptr : pieceMap[currentPosition.first][currentPosition.second - 1];
        Piece const* pLeft  = currentPosition.first  == 0 ? nullptr : pieceMap[currentPosition.first - 1][currentPosition.second];

        if (currentPosition.first == 0 || currentPosition.second == 0)
        {
            // grab each unused piece in turn
            for (auto& i : pieces)
            {
                if (!i.IsUsed())
                {
                    // test 1 and 2
                    if (i.IsSpecial())
                    {
                        if      (i.GetOriginalString() == "GGGG") { if (!pieces[0].IsUsed()) { continue; } }
                        else if (i.GetOriginalString() == "BBBB") { if (!pieces[1].IsUsed()) { continue; } }
                        else if (i.GetOriginalString() == "YYYY") { if (!pieces[2].IsUsed()) { continue; } }
                        else if (i.GetOriginalString() == "YRBG") { if (!pieces[4].IsUsed()) { continue; } }
                    }

                    // mark this piece as being used
                    i.ResetRotation();
                    i.SetPosition(currentPosition);
                    pieceMap[currentPosition.first][currentPosition.second] = &i;

                    do
                    {
                        if ((pAbove == nullptr || (pAbove->BL() == i.TL() && pAbove->BR() == i.TR())) &&
                            (pLeft  == nullptr || (pLeft->TR()  == i.TL() && pLeft->BR()  == i.BL())))
                        {
                            // yup - the piece fits in here nicely
                            FindSolutionsRecursive(pieces, pieceMap, maxX, maxY, nextPosition);
                        }
                    } while (i.RotateLeft());

                    // and finally mark the piece as NOT used
                    i.SetPosition(mm::InvalidPosition);
                    pieceMap[currentPosition.first][currentPosition.second] = nullptr;
                }
            }
        }
        else
        {
            //use more efficient matching process for non-edge pieces
            int const L = 100*pLeft->BR() + 10*pLeft->TR() + pAbove->BR();

            for (auto& i : pieces)
            {
                if (!i.IsUsed())
                {
                    // test 1 and 2
                    if (i.IsSpecial())
                    {
                        if      (i.GetOriginalString() == "GGGG") { if (!pieces[0].IsUsed()) { continue; } }
                        else if (i.GetOriginalString() == "BBBB") { if (!pieces[1].IsUsed()) { continue; } }
                        else if (i.GetOriginalString() == "YYYY") { if (!pieces[2].IsUsed()) { continue; } }
                        else if (i.GetOriginalString() == "YRBG") { if (!pieces[4].IsUsed()) { continue; } }
                    }

                    if (i.PieceFits(L))
                    {
                        i.SetPosition(currentPosition);
                        pieceMap[currentPosition.first][currentPosition.second] = &i;

                        FindSolutionsRecursive(pieces, pieceMap, maxX, maxY, nextPosition);

                        i.SetPosition(mm::InvalidPosition);
                        pieceMap[currentPosition.first][currentPosition.second] = nullptr;
                    }
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // this function kick-starts the solution finding process.  Note - the 'pieces' parameter is 
    // passed by value so that we don't modify the original here
    //
    auto FindSolutions(Pieces pieces, int x_size, int y_size) -> void
    {
        // do the sizes make sense?
        assert(x_size * y_size == pieces.size());

        // create our empty pieceMap
        PieceMap pieceMap;
        pieceMap.resize(x_size);
        for (auto& i : pieceMap)
            i.resize(y_size, nullptr);

        // and kick-start the recursive process of finding solutions
        std::cout << "\n\nfinding solutions for a " << x_size << "x" << y_size << " grid" << std::endl;
        FindSolutionsRecursive(pieces, pieceMap, x_size, y_size, Position(0, 0));
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
auto main() -> int
{
    try
    {
        // define our pieces
        mm::Pieces pieces = boost::assign::list_of
            ("RRRR")("GGGG")("BBBB")("YYYY")("YRGB")
            ("YRBG")("BBBG")("RYBG")("GYGB")("YYRG")
            ("RYGB")("RRYY")("YYGR")("GYGR")("BBYR")
            ("BYBG")("YYGG")("BBBY")("GBGR")("BBRY")
            ("RRRY")("GGBB")("BRGY")("RRBY")("BBGR")
            ("RRRG")("BBRR")("BYGR")("RRYB")("BBRG")
            ("RRRB")("RRGG")("RYRB")("RRBG")("BBYG")
            ("YYYR")("YYBB")("RGRB")("RRGB")("BBGY")
            ("YYYG")("RYRY")("RGRY")("RRYG")("GGBY")
            ("YYYB")("YGYG")("YGYR")("RRGY")("GGYB")
            ("GGGR")("GBGB")("YGYB")("YYBR")("GGBR")
            ("GGGY")("BRBR")("YRYB")("YYRB")("GGRB")
            ("GGGB")("RGRG")("BRBY")("YYBG")("GGYR")
            ("BBBR")("YBYB")("BRBG")("YYGB")("GGRY");

        mm::appStartTime = GetTickCount64();
        mm::FindSolutions(pieces, 5, 14);
        std::cout << "ninteenPrefix = " << mm::twentyPrefix << std::endl;
        mm::FindSolutions(pieces, 10, 7);
        std::cout << "\n\n search completed successfully" << std::endl;
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cout << "ERROR:  std::exception caught in main():  " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cout << "ERROR:  unknown exception caught in main()" << std::endl;
        return EXIT_FAILURE;
    }
}




