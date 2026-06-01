#include "ams/iface/vita/AckRPacket.h"
#include <iostream>
#include <array>
#include <cassert>
#include <span>
#include <stdexcept>

using namespace ams::iface::vita;

void test_ack_packet() {
    constexpr size_t BUFFER_SIZE = 1024;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    // Setup an AckX packet with 1 error
    // With a strict 9-word prologue:
    // 0: Word1
    // 1: Stream ID
    // 2: Class ID OUI
    // 3: Class ID Codes
    // 4: Timestamp Int
    // 5: Timestamp Frac Hi
    // 6: Timestamp Frac Lo
    // 7: CAM
    // 8: Message ID
    // 9: EIF0
    // 10: Error Payload
    
    constexpr uint32_t PKT_TYPE = 7;
    uint32_t word1 = (PKT_TYPE << masks::PKT_TYPE_SHIFT) | masks::CBIT_MASK | masks::CMD_ACK_MASK;
    word1 |= (3 << masks::TSI_SHIFT); // Int ts
    word1 |= (2 << masks::TSF_SHIFT); // Frac ts
    word1 |= 11; // pkt size: 9 prologue + 1 EIF + 1 Payload
    buffer[0] = htonl(word1);
    
    // CAM is at buffer[7]
    constexpr size_t CAM_INDEX = 7;
    buffer[CAM_INDEX] = htonl(ackx_detail::ACKX_CAM_REQUIRED_MASK | masks::CAM_ACKER_MASK);
    
    // Class IDs
    buffer[2] = htonl(0xAAAAAA);
    buffer[3] = htonl(0x04000C05);
    
    // Message ID is at buffer[8]
    constexpr size_t MSG_ID_INDEX = 8;
    buffer[MSG_ID_INDEX] = 0;
    
    // EIF0 indicating BANDWIDTH error starts at buffer[9]
    constexpr size_t EIF0_INDEX = 9;
    buffer[EIF0_INDEX] = htonl(masks::CIF0_BANDWIDTH_MASK);
    
    // Error Payload
    constexpr size_t ERROR_PAYLOAD_INDEX = 10;
    const uint32_t ERROR_PAYLOAD_VALUE = ackx_detail::pack_error_payload(
        AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1U);
    buffer[ERROR_PAYLOAD_INDEX] = htonl(ERROR_PAYLOAD_VALUE);

    AckXView view(buffer);
    assert(view.isAckX());
    assert(view.hasErrors());
    assert(!view.isSchX());
    
    auto payloads = view.getParsedErrorPayloads();
    assert(payloads.size() == 1);
    assert(payloads[0].subject == AckXErrorSubject::Bandwidth);
    assert(payloads[0].error_flags == ackx_detail::ERROR_NOT_EXECUTED);
    assert(payloads[0].error_enum_index == 1);

    std::cout << "test_ack_packet passed\n";
}

void test_ackx_builder() {
    constexpr size_t BUFFER_SIZE = 12; // 9 prologue + 1 EIF0 + 2 payloads
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.addError(AckXErrorSubject::DataFormat, ackx_detail::ERROR_INVALID_VALUE, 2);
    builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1);

    size_t size = builder.finalize();
    assert(size == BUFFER_SIZE);

    AckXView view(buffer);
    assert(view.isValid());
    assert(view.hasErrors());
    assert(view.isSchX());
    auto payloads = view.getParsedErrorPayloads();
    assert(payloads.size() == 2);
    assert(payloads[0].subject == AckXErrorSubject::Bandwidth);
    assert(payloads[0].error_flags == ackx_detail::ERROR_NOT_EXECUTED);
    assert(payloads[0].error_enum_index == 1);
    assert(payloads[1].subject == AckXErrorSubject::DataFormat);
    assert(payloads[1].error_flags == ackx_detail::ERROR_INVALID_VALUE);
    assert(payloads[1].error_enum_index == 2);

    std::cout << "test_ackx_builder passed\n";
}

void test_ackr_builder() {
    constexpr size_t BUFFER_SIZE = 15; // 9 prologue + 3 CIFs (CIF0, CIF2, CIF4) + 3 payloads
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckRBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x5678);
    
    size_t size = builder.finalize();
    assert(size == BUFFER_SIZE);

    AckRView view(buffer);
    assert(view.isValid());
    assert(view.isAckR());
    assert(!view.hasErrors());
    assert(view.getClassIdOui() == 0xAAAAAAU);
    assert(isAmsClassIdCodes(view.getClassIdCodes(), 0x0BU));
    assert(view.getTimestampInt() == 0U);
    assert(view.getTimestampFracHigh() == 0U);
    assert(view.getTimestampFracLow() == 0U);
    assert(view.hasClassId());
    assert(view.hasTsi());
    assert(view.hasTsf());

    std::cout << "test_ackr_builder passed\n";
}

void test_ackx_oob_read() {
    constexpr size_t BUFFER_SIZE = 10; // 9 prologue + 1 EIF0
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    // Setup an AckX packet that claims to have an error payload, but size is too small
    constexpr uint32_t PKT_TYPE = 7;
    uint32_t word1 = (PKT_TYPE << masks::PKT_TYPE_SHIFT) | masks::CBIT_MASK | masks::CMD_ACK_MASK;
    word1 |= (3 << masks::TSI_SHIFT);
    word1 |= (2 << masks::TSF_SHIFT);
    word1 |= BUFFER_SIZE; // Packet size matches actual buffer size, leaving no room for the payload
    buffer[0] = htonl(word1);
    
    constexpr size_t CAM_INDEX = 7;
    buffer[CAM_INDEX] = htonl(ackx_detail::ACKX_CAM_REQUIRED_MASK | masks::CAM_ACKER_MASK);
    buffer[2] = htonl(0xAAAAAA);
    buffer[3] = htonl(0x04000C05);
    
    constexpr size_t EIF0_INDEX = 9;
    buffer[EIF0_INDEX] = htonl(masks::CIF0_BANDWIDTH_MASK); // Claims 1 error payload

    // Pass the buffer. The view should be invalid because it lacks the claimed payload space.
    AckXView view(std::span<const uint32_t>(buffer.data(), buffer.size()));
    assert(!view.isValid());
    
    // getParsedErrorPayloads should return an empty vector, not read past the end of the buffer
    auto payloads = view.getParsedErrorPayloads();
    assert(payloads.empty());

    std::cout << "test_ackx_oob_read passed\n";
}

void test_ackx_subtype_exclusivity() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    size_t size = builder.finalize();
    assert(size == 9);

    AckXView valid_view(std::span<const uint32_t>(buffer.data(), size));
    assert(valid_view.isValid());

    uint32_t original_cam = ntohl(buffer[7]);
    buffer[7] = htonl(original_cam | masks::CAM_ACKV_MASK);
    AckXView ackv_ackx_view(std::span<const uint32_t>(buffer.data(), size));
    assert(!ackv_ackx_view.isValid() && "AckX must reject simultaneous AckV");

    buffer[7] = htonl(original_cam | masks::CAM_ACKS_MASK);
    AckXView acks_ackx_view(std::span<const uint32_t>(buffer.data(), size));
    assert(!acks_ackx_view.isValid() && "AckX must reject simultaneous AckS");

    buffer[7] = htonl(original_cam | masks::CAM_ACKR_MASK);
    AckXView ackr_ackx_view(std::span<const uint32_t>(buffer.data(), size));
    assert(!ackr_ackx_view.isValid() && "AckX must reject AckR mode");

    std::cout << "test_ackx_subtype_exclusivity passed\n";
}

void test_ackx_rejects_unsupported_warnings() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    size_t size = builder.finalize();
    assert(size == 9);

    buffer[7] = htonl(ntohl(buffer[7]) | masks::CAM_ACKW_MASK);
    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(!view.isValid() && "AckX must reject unsupported warning payload mode");

    std::cout << "test_ackx_rejects_unsupported_warnings passed\n";
}

void test_ack_rejects_unsupported_identifier_and_warning_cam_bits() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckRBuilder ackr_builder(buffer.data(), buffer.size());
    ackr_builder.setStreamId(0x1234);
    size_t ackr_size = ackr_builder.finalize();
    assert(ackr_size > 0);

    const uint32_t ackr_cam = ntohl(buffer[7]);
    const uint32_t unsupported_ackr_bits[] = {
        masks::CAM_CE_MASK,
        masks::CAM_IE_MASK,
        masks::CAM_CR_MASK,
        masks::CAM_IR_MASK,
        masks::CAM_ACKW_MASK
    };
    for (uint32_t bit : unsupported_ackr_bits) {
        buffer[7] = htonl(ackr_cam | bit);
        AckRView view(std::span<const uint32_t>(buffer.data(), ackr_size));
        assert(!view.isValid() && "AckR must reject unsupported identifier and warning CAM bits");
    }

    buffer.fill(0);
    AckXBuilder ackx_builder(buffer.data(), buffer.size());
    ackx_builder.setStreamId(0x1234);
    size_t ackx_size = ackx_builder.finalize();
    assert(ackx_size == 9);

    const uint32_t ackx_cam = ntohl(buffer[7]);
    const uint32_t unsupported_ackx_bits[] = {
        masks::CAM_CE_MASK,
        masks::CAM_IE_MASK,
        masks::CAM_CR_MASK,
        masks::CAM_IR_MASK,
        masks::CAM_ACKW_MASK
    };
    for (uint32_t bit : unsupported_ackx_bits) {
        buffer[7] = htonl(ackx_cam | bit);
        AckXView view(std::span<const uint32_t>(buffer.data(), ackx_size));
        assert(!view.isValid() && "AckX must reject unsupported identifier and warning CAM bits");
    }

    std::cout << "test_ack_rejects_unsupported_identifier_and_warning_cam_bits passed\n";
}

void test_ackx_rejects_invalid_header_and_cam_bits() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    size_t size = builder.finalize();
    assert(size == 9);

    const uint32_t original_word1 = ntohl(buffer[0]);
    buffer[0] = htonl(original_word1 & ~masks::CMD_ACK_MASK);
    assert(!AckXView(std::span<const uint32_t>(buffer.data(), size)).isValid());
    buffer[0] = htonl(original_word1 | (1U << 24));
    assert(!AckXView(std::span<const uint32_t>(buffer.data(), size)).isValid());
    buffer[0] = htonl(original_word1);

    const uint32_t original_cam = ntohl(buffer[7]);
    buffer[7] = htonl(original_cam | masks::CAM_ACKP_MASK);
    assert(!AckXView(std::span<const uint32_t>(buffer.data(), size)).isValid());
    buffer[7] = htonl((original_cam & ~masks::CAM_ACTION_MASK) | masks::CAM_ACKX_MASK);
    assert(!AckXView(std::span<const uint32_t>(buffer.data(), size)).isValid());

    std::cout << "test_ackx_rejects_invalid_header_and_cam_bits passed\n";
}

void test_ackx_accepts_tailored_rx_class_id() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.setInfoClassType(InfoClassCodeType::RxCommBaseSet);
    size_t size = builder.finalize();
    assert(size == 9);
    assert(ntohl(buffer[3]) == 0x05000C05U);

    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(view.isValid());
    assert(view.getClassIdCodes() == 0x05000C05U);

    buffer[3] = htonl(0x06000C05U);
    assert(!AckXView(std::span<const uint32_t>(buffer.data(), size)).isValid());
    buffer[3] = htonl(0x05010C05U);
    assert(!AckXView(std::span<const uint32_t>(buffer.data(), size)).isValid());

    std::cout << "test_ackx_accepts_tailored_rx_class_id passed\n";
}

void test_ackx_accepts_tailored_schedule_request_type() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.setScheduleRequestType(1U);
    size_t size = builder.finalize();
    assert(size == 9);

    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(view.isValid());
    assert(view.getScheduleRequestType() == 1U);

    buffer[7] = htonl((ntohl(buffer[7]) & ~masks::CAM_SCH_REQ_TYP_MASK) | (15U << masks::CAM_SCH_REQ_TYP_SHIFT));
    AckXView max_view(std::span<const uint32_t>(buffer.data(), size));
    assert(max_view.isValid());
    assert(max_view.getScheduleRequestType() == 15U);

    bool rejected_bad_type = false;
    try {
        builder.setScheduleRequestType(16U);
    } catch (const std::invalid_argument&) {
        rejected_bad_type = true;
    }
    assert(rejected_bad_type && "AckXBuilder must reject schedule request types wider than 4 bits");

    std::cout << "test_ackx_accepts_tailored_schedule_request_type passed\n";
}

void test_ackx_rejects_unsupported_eif_bits() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1);
    size_t size = builder.finalize();
    assert(size == 11);

    buffer[9] = htonl(1U << 30); // EIF0 bit 30 is not supported by the AMS AckX tailoring.
    buffer[10] = htonl(ackx_detail::ERROR_NOT_EXECUTED |
                       (30U << ackx_detail::ERROR_EIF_BIT_SHIFT) |
                       1U);
    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(!view.isValid() && "AckX must reject unsupported EIF bits");

    std::cout << "test_ackx_rejects_unsupported_eif_bits passed\n";
}

void test_ackx_preserves_schedule_status_with_errors() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.setSchX(true);
    builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1);
    size_t size = builder.finalize();
    assert(size == 11);

    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(view.isValid());
    assert(view.hasErrors());
    assert(view.isSchX() && "AckX errors must preserve the reported schedule status");

    buffer[7] = htonl(ntohl(buffer[7]) & ~masks::CAM_SCHX_MASK);
    AckXView unscheduled_error_view(std::span<const uint32_t>(buffer.data(), size));
    assert(unscheduled_error_view.isValid() && "AckX may report errors for unscheduled execution");
    assert(!unscheduled_error_view.isSchX());

    std::cout << "test_ackx_preserves_schedule_status_with_errors passed\n";
}

void test_ackx_rejects_payload_identifier_mismatch() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1);
    size_t size = builder.finalize();
    assert(size == 11);

    buffer[10] = htonl(ackx_detail::pack_error_payload(AckXErrorSubject::RfRefFreq, ackx_detail::ERROR_NOT_EXECUTED, 1));
    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(!view.isValid() && "AckX error payload must identify its EIF bit");

    std::cout << "test_ackx_rejects_payload_identifier_mismatch passed\n";
}

void test_ackx_no_field_error_uses_ams_sentinel() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckXBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x1234);
    builder.addError(AckXErrorSubject::NoCorrespondingPayloadField, ackx_detail::ERROR_NOT_EXECUTED, 3);
    size_t size = builder.finalize();
    assert(size == 11);
    assert((ntohl(buffer[9]) & (1U << 31)) != 0U);

    constexpr uint32_t expected_payload_id = 7U << ackx_detail::ERROR_EIF_INDEX_SHIFT;
    assert((ntohl(buffer[10]) & (ackx_detail::ERROR_EIF_INDEX_MASK | ackx_detail::ERROR_EIF_BIT_MASK)) == expected_payload_id);

    AckXView view(std::span<const uint32_t>(buffer.data(), size));
    assert(view.isValid());
    auto payloads = view.getParsedErrorPayloads();
    assert(payloads.size() == 1);
    assert(payloads[0].subject == AckXErrorSubject::NoCorrespondingPayloadField);
    assert(payloads[0].error_flags == ackx_detail::ERROR_NOT_EXECUTED);
    assert(payloads[0].error_enum_index == 3);

    buffer[10] = htonl(ackx_detail::ERROR_NOT_EXECUTED |
                       (31U << ackx_detail::ERROR_EIF_BIT_SHIFT) |
                       3U);
    AckXView wrong_sentinel_view(std::span<const uint32_t>(buffer.data(), size));
    assert(!wrong_sentinel_view.isValid() && "AckX no-field errors must use the AMS 7/0 payload sentinel");

    std::cout << "test_ackx_no_field_error_uses_ams_sentinel passed\n";
}

void test_ackx_builder_rejects_invalid_payload_args() {
    constexpr size_t BUFFER_SIZE = 16;
    std::array<uint32_t, BUFFER_SIZE> buffer{};
    AckXBuilder builder(buffer.data(), buffer.size());

    bool rejected_duplicate_subject = false;
    try {
        builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1);
        builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_INVALID_VALUE, 2);
    } catch (const std::invalid_argument&) {
        rejected_duplicate_subject = true;
    }
    assert(rejected_duplicate_subject && "AckXBuilder must reject duplicate error subjects");

    bool rejected_bad_flags = false;
    try {
        AckXBuilder bad_flags_builder(buffer.data(), buffer.size());
        bad_flags_builder.addError(AckXErrorSubject::Bandwidth, 1U, 1);
    } catch (const std::invalid_argument&) {
        rejected_bad_flags = true;
    }
    assert(rejected_bad_flags && "AckXBuilder must reject flags outside bits 31:19");

    bool rejected_bad_enum = false;
    try {
        AckXBuilder bad_enum_builder(buffer.data(), buffer.size());
        bad_enum_builder.addError(AckXErrorSubject::Bandwidth, ackx_detail::ERROR_NOT_EXECUTED, 1024);
    } catch (const std::invalid_argument&) {
        rejected_bad_enum = true;
    }
    assert(rejected_bad_enum && "AckXBuilder must reject enum indexes wider than 10 bits");

    std::cout << "test_ackx_builder_rejects_invalid_payload_args passed\n";
}

void test_ackx_zero_allocation() {
    AckXBuilder builder(nullptr, 0); // Zero size allocation bounds bypass check
    builder.setStreamId(0x1234); // Should not crash, variables cached internally
    size_t written = builder.finalize();
    assert(written == 0); // Buffer is protected from writing

    AckRBuilder builderR(nullptr, 0);
    builderR.setPacketCount(4);
    size_t writtenR = builderR.finalize();
    assert(writtenR == 0);

    std::cout << "test_ackx_zero_allocation passed\n";
}

void test_ack_negative() {
    constexpr size_t BUFFER_SIZE = 1024;
    std::array<uint32_t, BUFFER_SIZE> buffer{};

    AckRBuilder builder(buffer.data(), buffer.size());
    builder.setStreamId(0x5678);
    size_t written = builder.finalize();
    assert(written > 0 && "Builder should produce a valid packet");

    AckRView valid_view(std::span<const uint32_t>(buffer.data(), written));
    assert(valid_view.isValid() && "Base AckR packet should be valid");

    // Test Invalid OUI
    buffer[2] = htonl(0xBBBBBB);
    AckRView invalid_oui_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!invalid_oui_view.isValid() && "AckR with invalid OUI should be rejected");
    buffer[2] = htonl(0xAAAAAA); // Restore

    // Test Invalid Class ID Codes
    buffer[3] = htonl(0x04000B06);
    AckRView invalid_class_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!invalid_class_view.isValid() && "AckR with invalid Class ID should be rejected");
    buffer[3] = htonl(0x04000B05); // Restore

    // Test Missing B Field in CIF2
    uint32_t orig_cif2 = ntohl(buffer[10]);
    buffer[10] = htonl(orig_cif2 & ~(1U << 30)); // Remove CitedSid bit
    AckRView missing_b_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!missing_b_view.isValid() && "AckR missing a B field should be rejected");

    // Test Present N Field in CIF2
    buffer[10] = htonl(orig_cif2 | masks::SCHEDULEACKACKRPACKET_CIF2_N_MASK);
    AckRView present_n_view(std::span<const uint32_t>(buffer.data(), written));
    assert(!present_n_view.isValid() && "AckR with an N field should be rejected");

    std::cout << "test_ack_negative passed\n";
}

auto main() -> int {
    test_ack_packet();
    test_ackx_builder();
    test_ackr_builder();
    test_ackx_oob_read();
    test_ackx_subtype_exclusivity();
    test_ackx_rejects_unsupported_warnings();
    test_ack_rejects_unsupported_identifier_and_warning_cam_bits();
    test_ackx_rejects_invalid_header_and_cam_bits();
    test_ackx_accepts_tailored_rx_class_id();
    test_ackx_accepts_tailored_schedule_request_type();
    test_ackx_rejects_unsupported_eif_bits();
    test_ackx_preserves_schedule_status_with_errors();
    test_ackx_rejects_payload_identifier_mismatch();
    test_ackx_no_field_error_uses_ams_sentinel();
    test_ackx_builder_rejects_invalid_payload_args();
    test_ackx_zero_allocation();
    test_ack_negative();
    std::cout << "All Ack Packet tests passed!\n";
    return 0;
}

