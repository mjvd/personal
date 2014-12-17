#include "stdafx.h"
#include "IPv4Header.h"
#include <boost/asio/ip/address_v4.hpp>
#include <WinSock2.h>

mvd::IPv4Header::IPv4Header() { m.fill(0); }
auto mvd::IPv4Header::Version            () const -> std::uint8_t                { return (m[0] >> 4) & 0x0f;                                               }
auto mvd::IPv4Header::HeaderLength       () const -> std::uint16_t               { return (m[0] & 0x0f) * 4;                                                }
auto mvd::IPv4Header::TypeOfService      () const -> std::uint8_t                { return m[1];                                                             }
auto mvd::IPv4Header::TotalLength        () const -> std::uint16_t               { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[2]));            }
auto mvd::IPv4Header::Identification     () const -> std::uint16_t               { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[4]));            }
auto mvd::IPv4Header::DoNotFragment      () const -> bool                        { return (m[6] & 0x40) != 0;                                               }
auto mvd::IPv4Header::MoreFragments      () const -> bool                        { return (m[6] & 0x20) != 0;                                               }
auto mvd::IPv4Header::FragmentOffset     () const -> std::uint16_t               { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[6])) & 0x1fff;   }
auto mvd::IPv4Header::TimeToLive         () const -> std::uint8_t                { return m[8];                                                             }
auto mvd::IPv4Header::Protocol           () const -> std::uint8_t                { return m[9];                                                             }
auto mvd::IPv4Header::Checksum           () const -> std::uint16_t               { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[10]));           }
auto mvd::IPv4Header::SourceAddress      () const -> boost::asio::ip::address_v4 { return boost::asio::ip::address_v4(ntohl(*reinterpret_cast<std::uint32_t const*>(&m[12]))); }
auto mvd::IPv4Header::DestinationAddress () const -> boost::asio::ip::address_v4 { return boost::asio::ip::address_v4(ntohl(*reinterpret_cast<std::uint32_t const*>(&m[16]))); }

