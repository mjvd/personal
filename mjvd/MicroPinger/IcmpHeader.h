#pragma once
#include <array>
#include <cstdint>

// ICMP header for both IPv4 and IPv6.
//
// The wire format of an ICMP header is:
// 
// 0               8               16                             31
// +---------------+---------------+------------------------------+      ---
// |               |               |                              |       ^
// |     type      |     code      |          checksum            |       |
// |               |               |                              |       |
// +---------------+---------------+------------------------------+    8 bytes
// |                               |                              |       |
// |          identifier           |       sequence number        |       |
// |                               |                              |       v
// +-------------------------------+------------------------------+      ---


namespace mvd
{
    class IcmpHeader
    {
    public:
        enum class MessageType : std::uint8_t
        {
            EchoReply                   =  0,
            DestinationUnreachable      =  3,
            SourceQuench                =  4,
            Redirect                    =  5,
            AlternateHostAddress        =  6,
            EchoRequest                 =  8,
            RouterAdvertisement         =  9,
            RouterSelection             = 10,
            TimeExceeded                = 11,
            ParameterProblem            = 12,
            TimestampRequest            = 13,
            TimestampReply              = 14,
            InformationRequest          = 15,
            InformationReply            = 16,
            AddressMaskRequest          = 17,
            AddressMaskReply            = 18,
            Traceroute                  = 30,
            DatagramConversionError     = 31,
            MobileHostRedirect          = 32,
            IPv6_WhereAreYou            = 33,
            IPv6_IAmHere                = 34,
            MobileRegistrationRequest   = 35,
            MobileRegistrationReply     = 36,
            DomainNameRequest           = 37,
            DomainNameReply             = 38,
            SKIP                        = 39,
            Photuris                    = 40
        };

    public:
        IcmpHeader();

        // accessors
        auto Type           () const -> MessageType;
        auto Code           () const -> std::uint8_t;
        auto Checksum       () const -> std::uint16_t;
        auto Identifier     () const -> std::uint16_t;
        auto SequenceNumber () const -> std::uint16_t;

        // modifiers
        auto Type           (MessageType   v) -> void;
        auto Code           (std::uint8_t  v) -> void;
        auto Checksum       (std::uint16_t v) -> void;
        auto Identifier     (std::uint16_t v) -> void;
        auto SequenceNumber (std::uint16_t v) -> void;

        // streaming operators
        friend std::istream& operator>>(std::istream& is, IcmpHeader& header)       { return is.read (reinterpret_cast<char*>      (header.m.data()), 8); }
        friend std::ostream& operator<<(std::ostream& os, IcmpHeader const& header) { return os.write(reinterpret_cast<char const*>(header.m.data()), 8); }

        // convenient way to set the checksum
        template<class FwdIt>
        auto ComputeChecksum(FwdIt bodyBegin, FwdIt bodyEnd) -> void;

    private:
        std::array<std::uint8_t, 8> m;
    };



    //=============================================================================================
    template<class FwdIt>
    auto IcmpHeader::ComputeChecksum(FwdIt bodyBegin, FwdIt bodyEnd) -> void
    {
        unsigned int sum = (static_cast<std::uint8_t>(Type()) << 8) + Code() + Identifier() + SequenceNumber();
        while (bodyBegin != bodyEnd)
        {
            sum += (static_cast<std::uint8_t>(*bodyBegin++) << 8);
            if (bodyBegin != bodyEnd)
            {
                sum += static_cast<std::uint8_t>(*bodyBegin++);
            }
        }

        sum = (sum >> 16) + (sum & 0xffff);
        sum += (sum >> 16);
        Checksum(static_cast<std::uint16_t>(~sum));
    }


}

