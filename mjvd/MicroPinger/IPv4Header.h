#pragma once

#include <array>
#include <cstdint>
namespace boost { namespace asio { namespace ip { class address_v4; } } }


// The wire format of an IPv4 header is:
// 
// 0               8               16                             31
// +-------+-------+---------------+------------------------------+      ---
// |       |       |               |                              |       ^
// |version|header |    type of    |    total length in bytes     |       |
// |  (4)  | length|    service    |                              |       |
// +-------+-------+---------------+-+-+-+------------------------+       |
// |                               | | | |                        |       |
// |        identification         |0|D|M|    fragment offset     |       |
// |                               | |F|F|                        |       |
// +---------------+---------------+-+-+-+------------------------+       |
// |               |               |                              |       |
// | time to live  |   protocol    |       header checksum        |   20 bytes
// |               |               |                              |       |
// +---------------+---------------+------------------------------+       |
// |                                                              |       |
// |                      source IPv4 address                     |       |
// |                                                              |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                   destination IPv4 address                   |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
// |                                                              |       ^
// |                                                              |       |
// /                        options (if any)                      /    0 - 40
// /                                                              /     bytes
// |                                                              |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---

namespace mvd
{
    class IPv4Header
    {
    public:
        IPv4Header();

        auto Version            () const -> std::uint8_t;
        auto HeaderLength       () const -> std::uint16_t;
        auto TypeOfService      () const -> std::uint8_t;
        auto TotalLength        () const -> std::uint16_t;
        auto Identification     () const -> std::uint16_t;
        auto DoNotFragment      () const -> bool;
        auto MoreFragments      () const -> bool;
        auto FragmentOffset     () const -> std::uint16_t;
        auto TimeToLive         () const -> std::uint8_t;
        auto Protocol           () const -> std::uint8_t;
        auto Checksum           () const -> std::uint16_t;
        auto SourceAddress      () const -> boost::asio::ip::address_v4;
        auto DestinationAddress () const -> boost::asio::ip::address_v4;

        friend std::istream& operator>>(std::istream& is, IPv4Header& header)
        {
            is.read(reinterpret_cast<char*>(header.m.data()), 20);
            if (header.Version() != 4) { is.setstate(std::ios::failbit); }
            auto optionsLength = header.HeaderLength() - 20;
            if (optionsLength < 0 || optionsLength > 40) { is.setstate(std::ios::failbit); }
            is.read(reinterpret_cast<char*>(header.m.data()) + 20, optionsLength);
            return is;
        }

    private:
        std::array<std::uint8_t, 60> m;
    };
}

