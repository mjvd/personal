#ifndef INCLUDED_PIECE_HEADER
#define INCLUDED_PIECE_HEADER

#include <utility>
#include <string>
#include <array>

namespace mm
{
    typedef std::pair<int, int> Position;
    Position const InvalidPosition(-1, -1);


    ///////////////////////////////////////////////////////////////////////////////////////////////
    class Piece
    {
    public:
        explicit Piece(std::string const& s);

        auto IsSpecial() const -> bool;
        auto GetOriginalString() const -> std::string;
        auto GetString() const -> std::string;
        auto IsUsed() const -> bool;
        auto TL()     const -> int;
        auto TR()     const -> int;
        auto BL()     const -> int;
        auto BR()     const -> int;

        auto RotateLeft()                   -> bool;
        auto ResetRotation()                -> void;
        auto PieceFits(int L)               -> bool;
        auto SetPosition(Position const& p) -> void;

    private:
        int mRotation;
        int mMaxRotations;
        int mPosX;
        int mPosY;
        std::array<int, 4> mStr;
        std::array<int, 4> mL;
        bool isSpecial;
        std::string originalStr;
    };
} 

#endif  // INCLUDED_PIECE_HEADER


