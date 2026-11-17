// SPDX-License-Identifier: Apache-2.0
#ifndef PROGRAMMOR_TEST_DEVICE_H
#define PROGRAMMOR_TEST_DEVICE_H

#include "generated/common.pb.h"
#include "generated/test.pb.h"

#include <table/table.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  PROGRAMMOR_TEST_DEVICE_CONFIG_VERSION = 4,
  PROGRAMMOR_TEST_DEVICE_CURVE_POINTS = 8,
  PROGRAMMOR_TEST_DEVICE_MAP_X_POINTS = 8,
  PROGRAMMOR_TEST_DEVICE_MAP_Y_POINTS = 6,
  PROGRAMMOR_TEST_DEVICE_MAP_CELLS = 48,
  PROGRAMMOR_TEST_DEVICE_MEMORY_BYTES = 64,
  PROGRAMMOR_TEST_DEVICE_TABLE_ROWS = 4,
};

typedef struct programmor_test_device {
  epicecu_programmor_common_v1_Common1 common;
  fixtures_test_Share1 share1;
  fixtures_test_Share2 share2;
  fixtures_test_Share3 share3;
  fixtures_test_Share4 share4;
  fixtures_test_Share5 share5;
  fixtures_test_Share6 share6;
  fixtures_test_Share7 share7;
  fixtures_test_Share8 share8;
  uint16_t curve_x[PROGRAMMOR_TEST_DEVICE_CURVE_POINTS];
  uint8_t curve_cells[PROGRAMMOR_TEST_DEVICE_CURVE_POINTS];
  uint16_t map_x[PROGRAMMOR_TEST_DEVICE_MAP_X_POINTS];
  uint8_t map_y[PROGRAMMOR_TEST_DEVICE_MAP_Y_POINTS];
  float map_cells[PROGRAMMOR_TEST_DEVICE_MAP_CELLS];
  table_mutable_t curve;
  table_mutable_t map;
  float engine_speed;
  float load;
  float curve_output;
  float map_output;
  uint64_t next_counter_ms;
  uint64_t next_telemetry_ms;
  uint64_t loop_window_ms;
  uint32_t loop_count;
} programmor_test_device_t;

bool programmor_test_device_init(programmor_test_device_t *device,
                                 uint32_t serial_number,
                                 uint32_t firmware_version, uint64_t now_ms);
void programmor_test_device_tick(programmor_test_device_t *device,
                                 uint64_t now_ms);
bool programmor_test_device_encode(const programmor_test_device_t *device,
                                   bool common, uint32_t share_id,
                                   uint8_t *output, size_t capacity,
                                   size_t *output_size);
bool programmor_test_device_publish(programmor_test_device_t *device,
                                    bool common, uint32_t share_id,
                                    const uint8_t *data, size_t data_size);

#ifdef __cplusplus
}
#endif

#endif
