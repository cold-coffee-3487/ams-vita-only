#include "ams/iface/vita/GeneratedMasks.h"
#include "ams/iface/vita/Primitives.h"
#include <iostream>
#include <cassert>
#include <arpa/inet.h>

using namespace ams::iface::vita;

void test_header_masks() {
    constexpr uint32_t PKT_TYPE = 7U;
    constexpr uint32_t TSI_VAL = 3U;
    constexpr uint32_t TSF_VAL = 2U;
    constexpr uint32_t PKT_COUNT = 15U;
    constexpr uint32_t PKT_SIZE = 1024U;

    uint32_t word1 = 0;
    // Packet Type: 0111 (7)
    word1 |= (PKT_TYPE << masks::PKT_TYPE_SHIFT) & masks::PKT_TYPE_MASK;
    // C-Bit: 1
    word1 |= masks::CBIT_MASK;
    // isAck: 0
    // isCanc: 0
    // tsi: 3 (11)
    word1 |= (TSI_VAL << masks::TSI_SHIFT) & masks::TSI_MASK;
    // tsf: 2 (10)
    word1 |= (TSF_VAL << masks::TSF_SHIFT) & masks::TSF_MASK;
    // packetCount: 15
    word1 |= (PKT_COUNT << masks::PKT_COUNT_SHIFT) & masks::PKT_COUNT_MASK;
    // packetSize: 1024
    word1 |= PKT_SIZE & masks::PKT_SIZE_MASK;

    // Convert to network byte order
    uint32_t net_word1 = htonl(word1);

    // Read back via host byte order
    uint32_t host_word1 = ntohl(net_word1);

    assert(((host_word1 & masks::PKT_TYPE_MASK) >> masks::PKT_TYPE_SHIFT) == PKT_TYPE);
    assert((host_word1 & masks::CBIT_MASK) != 0);
    assert((host_word1 & masks::CMD_ACK_MASK) == 0);
    assert(((host_word1 & masks::TSI_MASK) >> masks::TSI_SHIFT) == TSI_VAL);
    assert(((host_word1 & masks::TSF_MASK) >> masks::TSF_SHIFT) == TSF_VAL);
    assert(((host_word1 & masks::PKT_COUNT_MASK) >> masks::PKT_COUNT_SHIFT) == PKT_COUNT);
    assert((host_word1 & masks::PKT_SIZE_MASK) == PKT_SIZE);

    std::cout << "test_header_masks passed\n";
}

void test_cif_masks() {
    constexpr uint32_t cif0 = masks::CIF0_BANDWIDTH_MASK | masks::CIF0_SAMPLERATE_MASK;
    
    static_assert((cif0 & masks::CIF0_BANDWIDTH_MASK) != 0, "mask fail");
    static_assert((cif0 & masks::CIF0_SAMPLERATE_MASK) != 0, "mask fail");
    static_assert((cif0 & masks::CIF0_GAIN_MASK) == 0, "mask fail");

    std::cout << "test_cif_masks passed\n";
}

void test_truncated_views() {
    std::array<uint32_t, 3> short_buffer = {
        htonl(3), // array total words
        htonl(5 | (1 << masks::DAS_WORD2_RECORD_SIZE_SHIFT)), // 5 records
        htonl(0) // no SID
    };

    DataAddressStructureView das_view(short_buffer);
    assert(!das_view.isValid());
    size_t count = 0;
    for (auto it = das_view.begin(); it != das_view.end(); ++it) {
        count++;
    }
    assert(count == 0); // should not iterate

    std::array<uint32_t, 3> short_p3d = {
        htonl(3), // array total words
        htonl(10 | (3 << masks::P3D_WORD2_HEADER_SIZE_SHIFT)), // 10 records
        htonl(0) // no IRB
    };

    Pointing3dStructureView p3d_view(short_p3d);
    assert(!p3d_view.isValid());
    assert(!p3d_view.getGlobalIndexRefBeam().has_value());
    count = 0;
    for (auto it = p3d_view.begin(); it != p3d_view.end(); ++it) {
        count++;
    }
    assert(count == 0); // should not iterate

    std::cout << "test_truncated_views passed\n";
}

auto main() -> int {
    test_header_masks();
    test_cif_masks();
    test_truncated_views();
    std::cout << "All tests passed!\n";
    return 0;
}

