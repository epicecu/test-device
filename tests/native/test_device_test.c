// SPDX-License-Identifier: Apache-2.0
#include "test_device.h"

#include <table/table.h>
#include <vendor/nanopb/pb_encode.h>

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static size_t encode(const pb_msgdesc_t *fields, const void *value,
                     uint8_t *output, size_t capacity) {
  pb_ostream_t stream = pb_ostream_from_buffer(output, capacity);
  assert(pb_encode(&stream, fields, value));
  return stream.bytes_written;
}

static void test_defaults_and_requests(void) {
  programmor_test_device_t device;
  uint8_t payload[512];
  size_t size = 0u;
  assert(programmor_test_device_init(&device, 123456u, 1000u, 0u));
  assert(device.common.registry_id == 1001u);
  assert(device.common.shares_version == PROGRAMMOR_TEST_DEVICE_CONFIG_VERSION);
  assert(PROGRAMMOR_TEST_DEVICE_CONFIG_VERSION == 3u);
  assert(device.common.firmware_version == 1000u);
  assert(device.common.serial_number == 123456u);
  assert(strcmp(device.common.device_name, "Programmor Test Device") == 0);
  assert(device.share7.memory.size == 64u);
  assert(device.share8.rows_count == 4u);
  assert(fabsf(device.curve_output - 35.0F) < 0.001F);
  assert(fabsf(device.map_output - 32.0F) < 0.001F);

  assert(programmor_test_device_encode(&device, true, 1u, payload,
                                       sizeof(payload), &size));
  assert(size > 0u);
  for (uint32_t share = 1u; share <= 9u; ++share) {
    assert(programmor_test_device_encode(&device, false, share, payload,
                                         sizeof(payload), &size));
    assert(size <= sizeof(payload));
    assert(share == 3u || size > 0u);
  }
  assert(!programmor_test_device_encode(&device, false, 99u, payload,
                                        sizeof(payload), &size));
  assert(
      !programmor_test_device_encode(&device, false, 9u, payload, 8u, &size));
}

static void test_atomic_publications(void) {
  programmor_test_device_t device;
  programmor_test_device_t source;
  uint8_t payload[512];
  size_t size = 0u;
  assert(programmor_test_device_init(&device, 1u, 1000u, 0u));
  assert(programmor_test_device_init(&source, 2u, 1000u, 0u));

  source.share1.startingNumber = 10;
  source.share1.endingNumber = 30;
  assert(programmor_test_device_encode(&source, false, 1u, payload,
                                       sizeof(payload), &size));
  assert(programmor_test_device_publish(&device, false, 1u, payload, size));
  assert(device.share1.startingNumber == 10);

  source.share1.startingNumber = 31;
  source.share1.endingNumber = 30;
  assert(programmor_test_device_encode(&source, false, 1u, payload,
                                       sizeof(payload), &size));
  assert(!programmor_test_device_publish(&device, false, 1u, payload, size));
  assert(device.share1.startingNumber == 10);

  source.share5.floatNumber = 101.0F;
  assert(programmor_test_device_encode(&source, false, 5u, payload,
                                       sizeof(payload), &size));
  assert(!programmor_test_device_publish(&device, false, 5u, payload, size));
  assert(device.share5.floatNumber == 23.45F);

  source.share8.rows[1].id = source.share8.rows[0].id;
  size = encode(fixtures_test_Share8_fields, &source.share8, payload,
                sizeof(payload));
  assert(!programmor_test_device_publish(&device, false, 8u, payload, size));
  assert(device.share8.rows[1].id == 2u);

  assert(!programmor_test_device_publish(&device, false, 3u, payload, size));
  assert(!programmor_test_device_publish(&device, true, 1u, payload, size));
}

static void test_table_integration(void) {
  programmor_test_device_t device;
  programmor_test_device_t source;
  table_scalar_t value;
  table_scalar_t map_value;
  uint8_t payload[512];
  size_t size = 0u;
  float original;
  assert(programmor_test_device_init(&device, 1u, 1000u, 0u));
  assert(programmor_test_device_init(&source, 2u, 1000u, 0u));

  value.type = TABLE_SCALAR_U8;
  value.value.u8 = 99u;
  assert(table_curve_set_cell(&source.curve, 0u, value) == TABLE_OK);
  assert(programmor_test_device_encode(&source, false, 9u, payload,
                                       sizeof(payload), &size));
  assert(programmor_test_device_publish(&device, false, 9u, payload, size));
  assert(table_curve_lookup(&device.curve.view, 500.0F, &original) == TABLE_OK);
  assert(fabsf(original - 99.0F) < 0.001F);

  map_value.type = TABLE_SCALAR_F32;
  map_value.value.f32 = 33.5F;
  assert(table_map_set_cell(&source.map, 0u, 0u, map_value) == TABLE_OK);
  assert(programmor_test_device_encode(&source, false, 9u, payload,
                                       sizeof(payload), &size));
  assert(programmor_test_device_publish(&device, false, 9u, payload, size));
  assert(table_map_lookup(&device.map.view, 500.0F, 0.0F, &original) ==
         TABLE_OK);
  assert(fabsf(original - 33.5F) < 0.001F);

  original = device.map_cells[0];
  assert(size > 1u);
  assert(
      !programmor_test_device_publish(&device, false, 9u, payload, size - 1u));
  assert(device.map_cells[0] == original);
}

static void test_hex_and_record_table_publications(void) {
  programmor_test_device_t device;
  programmor_test_device_t source;
  uint8_t payload[512];
  size_t size = 0u;
  assert(programmor_test_device_init(&device, 1u, 1000u, 0u));
  assert(programmor_test_device_init(&source, 2u, 1000u, 0u));

  memset(source.share7.memory.bytes, 0xA5, PROGRAMMOR_TEST_DEVICE_MEMORY_BYTES);
  assert(programmor_test_device_encode(&source, false, 7u, payload,
                                       sizeof(payload), &size));
  assert(programmor_test_device_publish(&device, false, 7u, payload, size));
  assert(memcmp(device.share7.memory.bytes, source.share7.memory.bytes,
                PROGRAMMOR_TEST_DEVICE_MEMORY_BYTES) == 0);

  source.share8.rows[0].id = 10u;
  strcpy(source.share8.rows[0].suburb, "Kent Town");
  source.share8.rows[2].enabled = true;
  assert(programmor_test_device_encode(&source, false, 8u, payload,
                                       sizeof(payload), &size));
  assert(programmor_test_device_publish(&device, false, 8u, payload, size));
  assert(device.share8.rows[0].id == 10u);
  assert(strcmp(device.share8.rows[0].suburb, "Kent Town") == 0);
  assert(device.share8.rows[2].enabled);
}

static void test_tick(void) {
  programmor_test_device_t device;
  assert(programmor_test_device_init(&device, 1u, 1000u, 0u));
  programmor_test_device_tick(&device, 1000u);
  assert(device.share1.counter == 1);
  assert(device.share3.loopsPerSecond == 1);
  assert(device.engine_speed > 500.0F);
}

int main(void) {
  test_defaults_and_requests();
  test_atomic_publications();
  test_table_integration();
  test_hex_and_record_table_publications();
  test_tick();
  puts("Programmor Test Device native tests passed.");
  return 0;
}
