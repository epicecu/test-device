// SPDX-License-Identifier: Apache-2.0
#include <Arduino.h>
#include <corelib/device.hpp>

#if defined(PROGRAMMOR_TRANSPORT_SERIAL)
extern "C" {
#include <corelib/serial.h>
}
#endif

#include <cstring>

extern "C" {
#include "test_device.h"
}

namespace {
constexpr corelib_link_id_t kLinkId = 1;
constexpr size_t kPendingRequests = 8;
constexpr size_t kMaximumTransactionDataBytes = 512;
constexpr size_t kTransactionEnvelopeBytes = 19;
constexpr size_t kMaximumMessageBytes = 9 * CORELIB_FRAME_SIZE;
constexpr uint8_t kLedPin = LED_BUILTIN;

static_assert(kMaximumMessageBytes >=
              kMaximumTransactionDataBytes + kTransactionEnvelopeBytes);

corelib::Device<kMaximumMessageBytes, 2, 32, kPendingRequests> core;
programmor_test_device_t device;
uint8_t incoming_frame[CORELIB_FRAME_SIZE];
#if defined(PROGRAMMOR_TRANSPORT_SERIAL)
corelib_serial_decoder_t serial_decoder{};
#endif
corelib_transaction_id_t requests[kPendingRequests];
size_t request_head;
size_t request_count;

uint32_t hardware_serial() {
  uint32_t serial = HW_OCOTP_MAC0 & 0x00FFFFFFu;
  if (serial < 10000000u) {
    serial *= 10u;
  }
  return serial;
}

void set_node_uuid(uint8_t uuid[16], uint32_t serial) {
  constexpr uint8_t prefix[12] = {0x50, 0x52, 0x4f, 0x47, 0x52, 0x41,
                                  0x4d, 0x4d, 0x4f, 0x52, 0x10, 0x01};
  memcpy(uuid, prefix, sizeof(prefix));
  uuid[6] = static_cast<uint8_t>((uuid[6] & 0x0Fu) | 0x40u);
  uuid[8] = static_cast<uint8_t>((uuid[8] & 0x3Fu) | 0x80u);
  uuid[12] = static_cast<uint8_t>(serial >> 24u);
  uuid[13] = static_cast<uint8_t>(serial >> 16u);
  uuid[14] = static_cast<uint8_t>(serial >> 8u);
  uuid[15] = static_cast<uint8_t>(serial);
}

corelib_send_result_t send_frame(void *, corelib_link_id_t link_id, void *,
                                 const uint8_t frame[CORELIB_FRAME_SIZE]) {
  if (link_id != kLinkId) {
    return CORELIB_SEND_FAILED;
  }
#if defined(PROGRAMMOR_TRANSPORT_SERIAL)
  uint8_t encoded[CORELIB_SERIAL_WIRE_SIZE];
  const size_t encoded_size = corelib_serial_encode(frame, encoded);
  if (encoded_size == 0u ||
      static_cast<size_t>(Serial.availableForWrite()) < encoded_size) {
    return CORELIB_SEND_BUSY;
  }
  return Serial.write(encoded, encoded_size) == encoded_size
             ? CORELIB_SEND_ACCEPTED
             : CORELIB_SEND_FAILED;
#else
  const int result = RawHID.send(frame, 0);
  if (result == CORELIB_FRAME_SIZE) {
    return CORELIB_SEND_ACCEPTED;
  }
  return result == 0 ? CORELIB_SEND_BUSY : CORELIB_SEND_FAILED;
#endif
}

void transaction_received(void *, const corelib_transaction_t *value) {
  const bool common = value->id.action == CORELIB_ACTION_COMMON_PUBLISH;
  if (value->id.action == CORELIB_ACTION_COMMON_PUBLISH ||
      value->id.action == CORELIB_ACTION_SHARE_PUBLISH) {
    (void)programmor_test_device_publish(&device, common, value->id.share_id,
                                         value->data, value->data_size);
    return;
  }
  if (request_count < kPendingRequests) {
    requests[(request_head + request_count) % kPendingRequests] = value->id;
    ++request_count;
  }
}

bool initialise_corelib(uint32_t serial) {
  corelib_config_t config{};
  set_node_uuid(config.node_uuid, serial);
  config.heartbeat_interval_ms = 2000;
  config.application_response_timeout_ms = 1000;
  config.maximum_transaction_data_size = kMaximumTransactionDataBytes;
  config.callbacks.send_frame = send_frame;
  config.callbacks.transaction = transaction_received;
  return core.init(config) == CORELIB_OK &&
         corelib_add_link(core.get(), kLinkId, nullptr) == CORELIB_OK;
}

void service_request() {
  if (request_count == 0u) {
    return;
  }
  const corelib_transaction_id_t request = requests[request_head];
  request_head = (request_head + 1u) % kPendingRequests;
  --request_count;
  const bool common = request.action == CORELIB_ACTION_COMMON_REQUEST;
  uint8_t payload[kMaximumTransactionDataBytes];
  size_t payload_size = 0u;
  const bool encoded =
      programmor_test_device_encode(&device, common, request.share_id, payload,
                                    sizeof(payload), &payload_size);
  (void)corelib_respond(
      core.get(), &request,
      encoded ? CORELIB_RESULT_SUCCESS : CORELIB_RESULT_UNSUPPORTED,
      encoded ? payload : nullptr, encoded ? payload_size : 0u);
}

[[noreturn]] void signal_failure(uint8_t code) {
  for (;;) {
    for (uint8_t pulse = 0u; pulse < code; ++pulse) {
      digitalWriteFast(kLedPin, HIGH);
      delay(150);
      digitalWriteFast(kLedPin, LOW);
      delay(150);
    }
    delay(1000);
  }
}
} // namespace

void setup() {
  pinMode(kLedPin, OUTPUT);
#if defined(PROGRAMMOR_TRANSPORT_SERIAL)
  Serial.begin(115200);
#endif
  const uint32_t serial = hardware_serial();
  if (!programmor_test_device_init(&device, serial, PROGRAMMOR_FIRMWARE_VERSION,
                                   millis())) {
    signal_failure(7u);
  }
  if (!initialise_corelib(serial)) {
    signal_failure(8u);
  }
  for (uint8_t pulse = 0u; pulse < 3u; ++pulse) {
    digitalWriteFast(kLedPin, HIGH);
    delay(100);
    digitalWriteFast(kLedPin, LOW);
    delay(100);
  }
}

void loop() {
  const uint64_t now_ms = millis();
#if defined(PROGRAMMOR_TRANSPORT_SERIAL)
  constexpr size_t kMaximumSerialBytesPerLoop = 2u * CORELIB_SERIAL_WIRE_SIZE;
  size_t processed = 0u;
  while (Serial.available() > 0 && processed < kMaximumSerialBytesPerLoop) {
    const int value = Serial.read();
    if (value < 0) {
      break;
    }
    ++processed;
    const corelib_serial_result_t result = corelib_serial_decoder_push(
        &serial_decoder, static_cast<uint8_t>(value), incoming_frame);
    if (result == CORELIB_SERIAL_FRAME) {
      (void)core.receive(kLinkId, incoming_frame, now_ms);
    }
  }
#else
  if (RawHID.recv(incoming_frame, 0) == CORELIB_FRAME_SIZE) {
    (void)core.receive(kLinkId, incoming_frame, now_ms);
  }
#endif
  programmor_test_device_tick(&device, now_ms);
  service_request();
  (void)core.tick(now_ms);
}
