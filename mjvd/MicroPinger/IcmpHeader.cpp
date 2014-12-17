#include "stdafx.h"
#include "IcmpHeader.h"
#include <WinSock2.h>

mvd::IcmpHeader::IcmpHeader() { m.fill(0); }

// accessors
auto mvd::IcmpHeader::Type           () const -> MessageType   { return static_cast<MessageType>(m[0]); }
auto mvd::IcmpHeader::Code           () const -> std::uint8_t  { return m[1]; }
auto mvd::IcmpHeader::Checksum       () const -> std::uint16_t { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[2])); }
auto mvd::IcmpHeader::Identifier     () const -> std::uint16_t { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[4])); }
auto mvd::IcmpHeader::SequenceNumber () const -> std::uint16_t { return ntohs(*reinterpret_cast<std::uint16_t const*>(&m[6])); }

// modifiers
auto mvd::IcmpHeader::Type           (MessageType   v) -> void { m[0] = static_cast<std::uint8_t>(v); }
auto mvd::IcmpHeader::Code           (std::uint8_t  v) -> void { m[1] = v; }
auto mvd::IcmpHeader::Checksum       (std::uint16_t v) -> void { *reinterpret_cast<std::uint16_t*>(&m[2]) = htons(v); }
auto mvd::IcmpHeader::Identifier     (std::uint16_t v) -> void { *reinterpret_cast<std::uint16_t*>(&m[4]) = htons(v); }
auto mvd::IcmpHeader::SequenceNumber (std::uint16_t v) -> void { *reinterpret_cast<std::uint16_t*>(&m[6]) = htons(v); }

