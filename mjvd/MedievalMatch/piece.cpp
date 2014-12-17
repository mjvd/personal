#include "piece.h"
#include <algorithm>
#include <cassert>


namespace
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    int CharToInt(char c)
    {
        switch (c)
        {
        case 'R': return 0;
        case 'G': return 1;
        case 'B': return 2;
        case 'Y': return 3;
        default: assert(false); return 0;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    char IntToChar(int i)
    {
        switch (i)
        {
        case 0: return 'R';
        case 1: return 'G';
        case 2: return 'B';
        case 3: return 'Y';
        default: assert(false); return 'X';
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
mm::Piece::Piece(std::string const& s)
    : mRotation(0)
    , mMaxRotations(0)
    , mPosX(InvalidPosition.first)
    , mPosY(InvalidPosition.second)
    , isSpecial(false)
    , originalStr(s)
{
    mStr[0] = CharToInt(s[3]);
    mStr[1] = CharToInt(s[0]);
    mStr[2] = CharToInt(s[1]);
    mStr[3] = CharToInt(s[2]);

    mL[0] = 100*BL() + 10*TL() + TR();
    mL[1] = 100*TL() + 10*TR() + BR();
    mL[2] = 100*TR() + 10*BR() + BL();
    mL[3] = 100*BR() + 10*BL() + TL();

    if      (mStr[0] == mStr[1] && mStr[1] == mStr[2] && mStr[0] == mStr[3]) { mMaxRotations = 1; }
    else if (mStr[0] == mStr[2] && mStr[1] == mStr[3])                       { mMaxRotations = 2; }
    else                                                                     { mMaxRotations = 4; }

    if (s == "GGGG" || s == "BBBB" || s == "YYYY" || s == "YRBG")
    {
        isSpecial = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::IsSpecial() const -> bool
{
    return isSpecial;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::GetOriginalString() const -> std::string
{
    return originalStr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::GetString() const -> std::string
{
    std::string rv;
    rv.append(1, IntToChar(TL())).append(1, IntToChar(TR())).append(1, IntToChar(BR())).append(1, IntToChar(BL()));
    return rv;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::IsUsed() const -> bool
{
    return mPosX != mm::InvalidPosition.first;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::BL() const -> int { return mStr[(0 + mRotation) % 4]; }
auto mm::Piece::TL() const -> int { return mStr[(1 + mRotation) % 4]; }
auto mm::Piece::TR() const -> int { return mStr[(2 + mRotation) % 4]; }
auto mm::Piece::BR() const -> int { return mStr[(3 + mRotation) % 4]; }

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::SetPosition(Position const& p) -> void
{
    mPosX = p.first;
    mPosY = p.second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::RotateLeft() -> bool
{
    mRotation = (mRotation + 1) % mMaxRotations;
    return mRotation != 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::ResetRotation() -> void
{
    mRotation = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
auto mm::Piece::PieceFits(int L) -> bool
{
    auto p=begin(mL);
    if      (*p++ == L) { mRotation = 0; return true;  }
    else if (*p++ == L) { mRotation = 1; return true;  }
    else if (*p++ == L) { mRotation = 2; return true;  }
    else if (*p   == L) { mRotation = 3; return true;  }
    else                {                return false; }

    /*
    for (auto itr=begin(mL); itr!=end(mL); ++itr)
    {
        if (*itr == L)
        {
            mRotation = std::distance(begin(mL), itr);
            return true;
        }
    }
    return false;
    */
}




