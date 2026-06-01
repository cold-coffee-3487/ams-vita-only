#include "ams/iface/vita/ExtensionDataContextPacket.h"
#include <iostream>
#include <array>
#include <cassert>
#include <stdexcept>

using namespace ams::iface::vita;

void test_context_packet() {
    constexpr size_t BUFFER_SIZE = 1024;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    ExtensionDataContextBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.setPacketCount(5);
    builder.addGain(0x12345678);
    builder.setGainStage2(0x1234U);
    builder.setGainStage1(0x5678U);
    builder.setBeamWidthHorizontal(0x1111U);
    builder.setBeamWidthVertical(0x2222U);
    builder.setPolarizationTiltAngle(0x3333U);
    builder.setPolarizationEllipticity(0x4444U);
    builder.setPointing3dElevation(0x5555U);
    builder.setPointing3dAzimuth(0x6666U);
    builder.setRfFigureOfMeritLowerBound(0x7777U);
    builder.setRfFigureOfMeritRequested(0x8888U);
    builder.setDataFormat(DataFormat::Complex16BitSigned);

    size_t size = builder.finalize();
    assert(size > 0);

    ExtensionDataContextView view(buffer);
    assert(view.isValid());
    assert(view.getStreamId() == 0x1234);
    assert(view.getPacketCount() == 5);
    
    auto gain = view.getGain();
    assert(gain);
    assert(*gain == 0x12345678);
    assert(view.getGainStage2().value_or(0U) == 0x1234U);
    assert(view.getGainStage1().value_or(0U) == 0x5678U);
    assert(view.getBeamWidthHorizontal().value_or(0U) == 0x1111U);
    assert(view.getBeamWidthVertical().value_or(0U) == 0x2222U);
    assert(view.getPolarizationTiltAngle().value_or(0U) == 0x3333U);
    assert(view.getPolarizationEllipticity().value_or(0U) == 0x4444U);
    assert(view.getPointing3dElevation().value_or(0U) == 0x5555U);
    assert(view.getPointing3dAzimuth().value_or(0U) == 0x6666U);
    assert(view.getRfFigureOfMeritLowerBound().value_or(0U) == 0x7777U);
    assert(view.getRfFigureOfMeritRequested().value_or(0U) == 0x8888U);

    bool replaced_data_format = false;
    const uint32_t supported_format_hi = static_cast<uint32_t>(static_cast<uint64_t>(DataFormat::Complex16BitSigned) >> 32);
    const uint32_t supported_format_lo = static_cast<uint32_t>(static_cast<uint64_t>(DataFormat::Complex16BitSigned));
    for (size_t i = 0; i + 1 < size; ++i) {
        if (ntohl(buffer[i]) == supported_format_hi && ntohl(buffer[i + 1]) == supported_format_lo) {
            buffer[i] = htonl(0x90000000U);
            buffer[i + 1] = htonl(0U);
            replaced_data_format = true;
            break;
        }
    }
    assert(replaced_data_format && "test packet should contain a dataFormat payload");
    ExtensionDataContextView unsupported_format_view(std::span<const uint32_t>(buffer.data(), size));
    assert(!unsupported_format_view.isValid() && "Context must reject unsupported AMS data formats");

    std::cout << "test_context_packet passed\n";
}

void test_context_zero_allocation() {
    ExtensionDataContextBuilder builder(nullptr, 0);
    builder.setStreamId(0x1234);
    builder.setPacketCount(5); // Should modify local word1 caching var, not crash
    size_t written = builder.finalize();
    assert(written == 0);

    std::cout << "test_context_zero_allocation passed\n";
}

void test_context_negative() {
    constexpr size_t BUFFER_SIZE = 1024;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    ExtensionDataContextBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.setPacketCount(5);
    
    // Add required payload fields to satisfy B masks
    builder.addBandwidth(5000000);
    builder.setDataFormat(DataFormat::Complex16BitSigned);
    // finalize() will automatically add the correct B-masks for ExtensionDataContextPacket.
    
    size_t written = builder.finalize();
    assert(written > 0 && "Builder should produce a valid context packet");

    ExtensionDataContextView valid_view(std::span<const uint32_t>(buffer.data(), written));
    assert(valid_view.isValid() && "Base Context packet should be valid");

    const uint32_t original_word1 = ntohl(buffer[0]);
    const uint32_t invalid_header_bits[] = {
        1U << 26, // .reservedBit26
        1U << 25, // .notV49p0Packet
        1U << 24  // .timeStampMode
    };
    for (uint32_t bit : invalid_header_bits) {
        buffer[0] = htonl(original_word1 | bit);
        ExtensionDataContextView invalid_header_view(std::span<const uint32_t>(buffer.data(), written));
        assert(!invalid_header_view.isValid() && "Context header fixed-zero bit should be rejected");
    }
    buffer[0] = htonl(original_word1);

    // Test Invalid OUI
    buffer[2] = htonl(0xBBBBBB);
    ExtensionDataContextView invalid_oui_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!invalid_oui_view.isValid() && "Context with invalid OUI should be rejected");
    buffer[2] = htonl(0xAAAAAA);

    // Test Missing B Field in CIF0
    uint32_t orig_cif0 = ntohl(buffer[7]);
    // Remove bit 29 (Bandwidth)
    buffer[7] = htonl(orig_cif0 & ~(1U << 29));
    ExtensionDataContextView missing_b_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!missing_b_view.isValid() && "Context missing a B field should be rejected");

    // Test Present N Field in CIF0
    buffer[7] = htonl(orig_cif0 | masks::EXTENSIONDATACONTEXTPACKET_CIF0_N_MASK);
    ExtensionDataContextView present_n_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!present_n_view.isValid() && "Context with an N field should be rejected");

    std::cout << "test_context_negative passed\n";
}

auto main() -> int {
    test_context_packet();
    test_context_zero_allocation();
    test_context_negative();
    std::cout << "All Context Packet tests passed!\n";
    return 0;
}
