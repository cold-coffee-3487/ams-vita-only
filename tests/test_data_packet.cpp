#include "ams/iface/vita/DataPacket.h"
#include <iostream>
#include <cassert>
#include <array>
#include <vector>

using namespace ams::iface::vita;

void test_data_packet_getters_setters() {
    constexpr uint32_t STREAM_ID = 0x12345678;

    std::array<uint32_t, 1024> buffer{};
    DataPacketBuilder builder(buffer.data(), buffer.size());

    builder.setStreamId(STREAM_ID);

    std::array<uint32_t, 4> sample_payload = { htonl(0xAAAA), htonl(0xBBBB), htonl(0xCCCC), htonl(0xDDDD) };
    builder.setPayload(sample_payload);

    size_t written_words = builder.finalize();
    assert(written_words == 7 + 4 + 1); // 7 prologue + 4 payload + 1 trailer

    DataPacketView view(buffer);
    assert(view.isValid());
    // AMS VITA 49.2 Tailoring constraints enforced by finalize()
    assert(view.getPacketType() == 1);
    assert(view.getCBit() == true);
    assert(view.getTsi() == 3);
    assert(view.getTsf() == 2);
    assert(view.getPacketSize() == 12);
    assert(view.getStreamId() == STREAM_ID);
    assert((view.getWord1() & (masks::ND0_MASK | masks::SBIT_MASK)) == 0U);

    auto parsed_payload = view.getPayload();
    assert(parsed_payload.size() == 4);
    assert(parsed_payload[0] == htonl(0xAAAA));

    std::cout << "test_data_packet_getters_setters passed\n";
}

void test_data_packet_trailer() {
    std::array<uint32_t, 1024> buffer{};
    DataPacketBuilder builder(buffer.data(), buffer.size());

    std::array<uint32_t, 2> sample_payload = { htonl(0x1111), htonl(0x2222) };
    builder.setPayload(sample_payload);

    DataTrailerBuilder trailer;
    trailer.setSampleFrameIndicatorEnables(3);
    trailer.setSampleFrameIndicators(2);
    builder.setTrailer(trailer);

    size_t written = builder.finalize();
    assert(written == 7 + 2 + 1);

    DataPacketView view(std::span<const uint32_t>(buffer.data(), written));
    assert(view.isValid());
    assert(view.hasTrailer() == true);
    assert(view.getPacketSize() == 10);
    assert(view.getPayload().size() == 2);

    auto trailer_view = view.getTrailerView();
    assert(trailer_view.has_value());
    assert(trailer_view->getSampleFrameIndicatorEnables() == 3);
    assert(trailer_view->getSampleFrameIndicators() == 2);

    std::cout << "test_data_packet_trailer passed\n";
}

void test_data_packet_missing_trailer() {
    constexpr uint32_t STREAM_ID = 0x12345678;

    std::array<uint32_t, 1024> buffer{};
    
    // Manually construct a data packet without the trailer bit set
    constexpr uint32_t PKT_TYPE = 1;
    uint32_t word1 = (PKT_TYPE << masks::PKT_TYPE_SHIFT) | masks::CBIT_MASK;
    word1 |= 8; // 7 prologue + 1 payload
    
    buffer[0] = htonl(word1);
    buffer[1] = htonl(STREAM_ID);
    buffer[7] = htonl(0xAAAA); // Payload

    DataPacketView view(buffer);
    assert(!view.isValid() && "Data packet missing trailer bit should be invalid per AMS GRA tailoring");

    std::cout << "test_data_packet_missing_trailer passed\n";
}

void test_data_packet_oversize() {
    std::array<uint32_t, 65540> buffer{};
    DataPacketBuilder builder(buffer.data(), buffer.size());
    std::vector<uint32_t> large_payload(65530, 0x12345678);
    builder.setPayload(large_payload);
    
    // required_words will be 7 + 65530 = 65537, which is > 65535
    size_t written = builder.finalize();
    assert(written == 0); // Must fail instead of silently truncating size mask
    std::cout << "test_data_packet_oversize passed\n";
}

void test_data_packet_negative() {
    std::array<uint32_t, 1024> buffer{};
    
    constexpr uint32_t STREAM_ID = 0x12345678;
    constexpr uint32_t PKT_TYPE = 1;
    uint32_t word1 = (PKT_TYPE << masks::PKT_TYPE_SHIFT) | masks::CBIT_MASK | masks::DATA_TRAILER_MASK;
    word1 |= (3 << masks::TSI_SHIFT);
    word1 |= (2 << masks::TSF_SHIFT);
    word1 |= 8;
    
    buffer[0] = htonl(word1);
    buffer[1] = htonl(STREAM_ID);
    buffer[2] = htonl(0xAAAAAA); // Valid OUI
    buffer[3] = htonl(0x04000D05); // Valid Codes
    buffer[4] = 0; // ts_int
    buffer[5] = 0; // ts_frac_hi
    buffer[6] = 0; // ts_frac_lo
    buffer[7] = htonl(0x00C00000); // Trailer
    
    DataPacketView valid_view(buffer);
    assert(valid_view.isValid() && "Base packet should be valid");

    const uint32_t original_word1 = ntohl(buffer[0]);
    buffer[0] = htonl(original_word1 | masks::ND0_MASK);
    DataPacketView nd0_view(buffer);
    assert(!nd0_view.isValid() && "Data packet with notV49p0Packet set must be rejected");

    buffer[0] = htonl(original_word1 | masks::SBIT_MASK);
    DataPacketView sbit_view(buffer);
    assert(!sbit_view.isValid() && "AMS data packet with sBit set must be rejected");
    buffer[0] = htonl(original_word1);
    
    // Invalid OUI
    buffer[2] = htonl(0xBBBBBB);
    DataPacketView invalid_oui_view(buffer);
    assert(!invalid_oui_view.isValid() && "Packet with invalid OUI should be rejected");
    buffer[2] = htonl(0xAAAAAA); // Restore
    
    // Invalid Class ID Codes
    buffer[3] = htonl(0x04000D06);
    DataPacketView invalid_class_view(buffer);
    assert(!invalid_class_view.isValid() && "Packet with invalid Class ID should be rejected");
    buffer[3] = htonl(0x04000D05); // Restore

    // Invalid DataTrailer: only bits 23:22 and 11:10 are supported by the AMS tailoring.
    std::array<uint32_t, 1024> t_buffer{};
    DataPacketBuilder builder(t_buffer.data(), t_buffer.size());
    std::array<uint32_t, 2> sample_payload = { htonl(0x1111), htonl(0x2222) };
    builder.setPayload(sample_payload);
    DataTrailerBuilder trailer;
    builder.setTrailer(trailer);
    size_t written = builder.finalize();
    assert(written > 0);

    const uint32_t base_trailer = ntohl(t_buffer[written - 1]);
    for (uint32_t bit = 0; bit < 32; ++bit) {
        if (bit == 22 || bit == 23 || bit == 10 || bit == 11) continue;
        t_buffer[written - 1] = htonl(base_trailer | (1U << bit));
        DataPacketView trailer_view(std::span<const uint32_t>(t_buffer.data(), written));
        assert(!trailer_view.isValid() && "Trailer with unsupported bit set should make the packet invalid");
    }

    std::cout << "test_data_packet_negative passed\n";
}

auto main() -> int {
    test_data_packet_getters_setters();
    test_data_packet_trailer();
    test_data_packet_missing_trailer();
    test_data_packet_oversize();
    test_data_packet_negative();
    std::cout << "All DataPacket tests passed!\n";
    return 0;
}

