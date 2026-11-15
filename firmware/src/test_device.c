// SPDX-License-Identifier: Apache-2.0
#include "test_device.h"

#include <table/nanopb.h>
#include <vendor/nanopb/pb_decode.h>
#include <vendor/nanopb/pb_encode.h>

#include <math.h>
#include <string.h>

static const uint16_t default_curve_x[PROGRAMMOR_TEST_DEVICE_CURVE_POINTS] = {
    500u, 1000u, 1500u, 2000u, 2500u, 3000u, 3500u, 4000u};
static const uint8_t default_curve_cells[PROGRAMMOR_TEST_DEVICE_CURVE_POINTS] =
    {35u, 42u, 52u, 63u, 72u, 78u, 74u, 66u};
static const uint16_t default_map_x[PROGRAMMOR_TEST_DEVICE_MAP_X_POINTS] = {
    500u, 1000u, 1500u, 2000u, 2500u, 3000u, 3500u, 4000u};
static const uint8_t default_map_y[PROGRAMMOR_TEST_DEVICE_MAP_Y_POINTS] = {
    0u, 20u, 40u, 60u, 80u, 100u};
static const float default_map_cells[PROGRAMMOR_TEST_DEVICE_MAP_CELLS] = {
    32.0F, 35.0F, 40.0F, 45.0F, 50.0F, 52.0F, 50.0F, 46.0F, 38.0F, 42.0F,
    48.0F, 54.0F, 60.0F, 63.0F, 60.0F, 55.0F, 45.0F, 50.0F, 57.0F, 64.0F,
    70.0F, 73.0F, 70.0F, 64.0F, 52.0F, 58.0F, 65.0F, 72.0F, 78.0F, 82.0F,
    79.0F, 72.0F, 58.0F, 64.0F, 71.0F, 78.0F, 85.0F, 90.0F, 87.0F, 80.0F,
    62.0F, 68.0F, 75.0F, 82.0F, 89.0F, 95.0F, 92.0F, 85.0F};

static bool encode_message(const pb_msgdesc_t *fields, const void *message,
                           uint8_t *output, size_t capacity,
                           size_t *output_size) {
  pb_ostream_t stream;
  if (fields == NULL || message == NULL || output == NULL ||
      output_size == NULL) {
    return false;
  }
  stream = pb_ostream_from_buffer(output, capacity);
  if (!pb_encode(&stream, fields, message)) {
    return false;
  }
  *output_size = stream.bytes_written;
  return true;
}

static bool decode_message(const pb_msgdesc_t *fields, void *message,
                           const uint8_t *data, size_t data_size) {
  pb_istream_t stream;
  if (fields == NULL || message == NULL || (data_size != 0u && data == NULL)) {
    return false;
  }
  stream = pb_istream_from_buffer(data, data_size);
  return pb_decode(&stream, fields, message) && stream.bytes_left == 0u;
}

static bool valid_wiring(const fixtures_test_Share2 *value) {
  const bool analog_a =
      value->analogInputAPinId == 103u || value->analogInputAPinId == 104u;
  const bool analog_b =
      value->analogInputBPinId == 103u || value->analogInputBPinId == 104u;
  return value->frequencyInputPinId == 101u &&
         value->digitalOutputPinId == 102u && analog_a && analog_b &&
         value->analogInputAPinId != value->analogInputBPinId;
}

static bool valid_rows(const fixtures_test_Share8 *value) {
  size_t index;
  size_t previous;
  if (value->rows_count != PROGRAMMOR_TEST_DEVICE_TABLE_ROWS) {
    return false;
  }
  for (index = 0u; index < value->rows_count; ++index) {
    if (value->rows[index].id == 0u ||
        value->rows[index].status >
            fixtures_test_TableStatus_TABLE_STATUS_SUSPENDED) {
      return false;
    }
    for (previous = 0u; previous < index; ++previous) {
      if (value->rows[previous].id == value->rows[index].id) {
        return false;
      }
    }
  }
  return true;
}

static bool valid_float_step(float value) {
  const float scaled = value * 20.0F;
  return fabsf(scaled - roundf(scaled)) <= 0.0001F;
}

static bool encode_share9(const programmor_test_device_t *device,
                          uint8_t *output, size_t capacity,
                          size_t *output_size) {
  fixtures_test_Share9 value = fixtures_test_Share9_init_zero;
  table_nanopb_curve_encoder_t curve_encoder;
  table_nanopb_map_encoder_t map_encoder;
  value.has_curve = true;
  value.has_map = true;
  value.engineSpeed = device->engine_speed;
  value.load = device->load;
  if (table_nanopb_curve_encode_init(&curve_encoder, &value.curve,
                                     &device->curve.view) != TABLE_OK ||
      table_nanopb_map_encode_init(&map_encoder, &value.map,
                                   &device->map.view) != TABLE_OK) {
    return false;
  }
  return encode_message(fixtures_test_Share9_fields, &value, output, capacity,
                        output_size);
}

static bool publish_share9(programmor_test_device_t *device,
                           const uint8_t *data, size_t data_size) {
  fixtures_test_Share9 value = fixtures_test_Share9_init_zero;
  table_nanopb_curve_decoder_t curve_decoder;
  table_nanopb_map_decoder_t map_decoder;
  uint16_t curve_x[PROGRAMMOR_TEST_DEVICE_CURVE_POINTS];
  uint8_t curve_cells[PROGRAMMOR_TEST_DEVICE_CURVE_POINTS];
  uint16_t map_x[PROGRAMMOR_TEST_DEVICE_MAP_X_POINTS];
  uint8_t map_y[PROGRAMMOR_TEST_DEVICE_MAP_Y_POINTS];
  float map_cells[PROGRAMMOR_TEST_DEVICE_MAP_CELLS];
  table_view_t curve_view;
  table_view_t map_view;
  pb_istream_t stream;

  if (table_nanopb_curve_decode_init(&curve_decoder, &value.curve, curve_x,
                                     PROGRAMMOR_TEST_DEVICE_CURVE_POINTS,
                                     TABLE_SCALAR_U16, curve_cells,
                                     TABLE_SCALAR_U8) != TABLE_OK ||
      table_nanopb_map_decode_init(
          &map_decoder, &value.map, map_x, PROGRAMMOR_TEST_DEVICE_MAP_X_POINTS,
          TABLE_SCALAR_U16, map_y, PROGRAMMOR_TEST_DEVICE_MAP_Y_POINTS,
          TABLE_SCALAR_U8, map_cells, TABLE_SCALAR_F32) != TABLE_OK ||
      (data_size != 0u && data == NULL)) {
    return false;
  }
  stream = pb_istream_from_buffer(data, data_size);
  if (!pb_decode(&stream, fixtures_test_Share9_fields, &value) ||
      stream.bytes_left != 0u || !value.has_curve || !value.has_map ||
      table_nanopb_curve_decode_finish(&curve_decoder, &value.curve,
                                       &curve_view) != TABLE_OK ||
      table_nanopb_map_decode_finish(&map_decoder, &value.map, &map_view) !=
          TABLE_OK) {
    return false;
  }
  if (table_replace(&device->curve, &curve_view) != TABLE_OK ||
      table_replace(&device->map, &map_view) != TABLE_OK) {
    return false;
  }
  return true;
}

bool programmor_test_device_init(programmor_test_device_t *device,
                                 uint32_t serial_number,
                                 uint32_t firmware_version, uint64_t now_ms) {
  size_t index;
  static const char *const names[PROGRAMMOR_TEST_DEVICE_TABLE_ROWS] = {
      "Alex Morgan", "Priya Shah", "Jordan Lee", "Sam Taylor"};
  static const char *const suburbs[PROGRAMMOR_TEST_DEVICE_TABLE_ROWS] = {
      "Norwood", "Glenelg", "Prospect", "Unley"};
  static const uint32_t ages[PROGRAMMOR_TEST_DEVICE_TABLE_ROWS] = {34u, 28u,
                                                                   45u, 52u};
  static const fixtures_test_TableStatus
      statuses[PROGRAMMOR_TEST_DEVICE_TABLE_ROWS] = {
          fixtures_test_TableStatus_TABLE_STATUS_ACTIVE,
          fixtures_test_TableStatus_TABLE_STATUS_PENDING,
          fixtures_test_TableStatus_TABLE_STATUS_SUSPENDED,
          fixtures_test_TableStatus_TABLE_STATUS_ACTIVE};
  if (device == NULL) {
    return false;
  }
  memset(device, 0, sizeof(*device));
  device->common.id = 2u;
  device->common.registry_id = 1001u;
  device->common.serial_number = serial_number;
  device->common.shares_version = PROGRAMMOR_TEST_DEVICE_CONFIG_VERSION;
  device->common.firmware_version = firmware_version;
  strcpy(device->common.device_name, "Programmor Test Device");
  device->share1.endingNumber = 20;
  device->share2.frequencyInputPinId = 101u;
  device->share2.digitalOutputPinId = 102u;
  device->share2.analogInputAPinId = 103u;
  device->share2.analogInputBPinId = 104u;
  strcpy(device->share4.welcomeText, "Hello there!");
  device->share5.floatNumber = 23.45F;
  device->share5.doubleNumber = 1.2345678910111213;
  strcpy(device->share5.ipAddress, "192.168.1.5");
  device->share5.portNumber = 8080;
  strcpy(device->share5.dateTime, "2023-08-01T00:00:00Z");
  device->share5.booleanValue = true;
  device->share6.dayOfTheWeek = 1;
  device->share7.memory.size = PROGRAMMOR_TEST_DEVICE_MEMORY_BYTES;
  for (index = 0u; index < PROGRAMMOR_TEST_DEVICE_MEMORY_BYTES; ++index) {
    device->share7.memory.bytes[index] = (uint8_t)index;
  }
  device->share8.rows_count = PROGRAMMOR_TEST_DEVICE_TABLE_ROWS;
  for (index = 0u; index < PROGRAMMOR_TEST_DEVICE_TABLE_ROWS; ++index) {
    fixtures_test_TableRow *row = &device->share8.rows[index];
    row->id = (uint32_t)index + 1u;
    strcpy(row->name, names[index]);
    strcpy(row->suburb, suburbs[index]);
    row->age = ages[index];
    row->enabled = index != 2u;
    row->status = statuses[index];
  }
  memcpy(device->curve_x, default_curve_x, sizeof(default_curve_x));
  memcpy(device->curve_cells, default_curve_cells, sizeof(default_curve_cells));
  memcpy(device->map_x, default_map_x, sizeof(default_map_x));
  memcpy(device->map_y, default_map_y, sizeof(default_map_y));
  memcpy(device->map_cells, default_map_cells, sizeof(default_map_cells));
  if (table_curve_init(&device->curve, device->curve_x,
                       PROGRAMMOR_TEST_DEVICE_CURVE_POINTS, TABLE_SCALAR_U16,
                       device->curve_cells, TABLE_SCALAR_U8) != TABLE_OK ||
      table_map_init(
          &device->map, device->map_x, PROGRAMMOR_TEST_DEVICE_MAP_X_POINTS,
          TABLE_SCALAR_U16, device->map_y, PROGRAMMOR_TEST_DEVICE_MAP_Y_POINTS,
          TABLE_SCALAR_U8, device->map_cells, TABLE_SCALAR_F32) != TABLE_OK) {
    return false;
  }
  device->engine_speed = 500.0F;
  device->load = 0.0F;
  device->next_counter_ms = now_ms + 1000u;
  device->next_telemetry_ms = now_ms + 100u;
  device->loop_window_ms = now_ms + 1000u;
  (void)table_curve_lookup(&device->curve.view, device->engine_speed,
                           &device->curve_output);
  (void)table_map_lookup(&device->map.view, device->engine_speed, device->load,
                         &device->map_output);
  return true;
}

void programmor_test_device_tick(programmor_test_device_t *device,
                                 uint64_t now_ms) {
  if (device == NULL) {
    return;
  }
  ++device->loop_count;
  while (now_ms >= device->next_counter_ms) {
    ++device->share1.counter;
    if (device->share1.counter > device->share1.endingNumber) {
      device->share1.counter = device->share1.startingNumber;
    }
    device->next_counter_ms += 1000u;
  }
  while (now_ms >= device->next_telemetry_ms) {
    device->engine_speed += 100.0F;
    if (device->engine_speed > 4000.0F) {
      device->engine_speed = 500.0F;
      device->load += 5.0F;
      if (device->load > 100.0F) {
        device->load = 0.0F;
      }
    }
    (void)table_curve_lookup(&device->curve.view, device->engine_speed,
                             &device->curve_output);
    (void)table_map_lookup(&device->map.view, device->engine_speed,
                           device->load, &device->map_output);
    device->next_telemetry_ms += 100u;
  }
  if (now_ms >= device->loop_window_ms) {
    device->share3.loopsPerSecond = device->loop_count > INT32_MAX
                                        ? INT32_MAX
                                        : (int32_t)device->loop_count;
    device->loop_count = 0u;
    device->loop_window_ms = now_ms + 1000u;
  }
}

bool programmor_test_device_encode(const programmor_test_device_t *device,
                                   bool common, uint32_t share_id,
                                   uint8_t *output, size_t capacity,
                                   size_t *output_size) {
  if (device == NULL || output == NULL || output_size == NULL) {
    return false;
  }
  if (common) {
    return share_id == 1u &&
           encode_message(epicecu_programmor_common_v1_Common1_fields,
                          &device->common, output, capacity, output_size);
  }
  switch (share_id) {
  case 1u:
    return encode_message(fixtures_test_Share1_fields, &device->share1, output,
                          capacity, output_size);
  case 2u:
    return encode_message(fixtures_test_Share2_fields, &device->share2, output,
                          capacity, output_size);
  case 3u:
    return encode_message(fixtures_test_Share3_fields, &device->share3, output,
                          capacity, output_size);
  case 4u:
    return encode_message(fixtures_test_Share4_fields, &device->share4, output,
                          capacity, output_size);
  case 5u:
    return encode_message(fixtures_test_Share5_fields, &device->share5, output,
                          capacity, output_size);
  case 6u:
    return encode_message(fixtures_test_Share6_fields, &device->share6, output,
                          capacity, output_size);
  case 7u:
    return encode_message(fixtures_test_Share7_fields, &device->share7, output,
                          capacity, output_size);
  case 8u:
    return encode_message(fixtures_test_Share8_fields, &device->share8, output,
                          capacity, output_size);
  case 9u:
    return encode_share9(device, output, capacity, output_size);
  default:
    return false;
  }
}

bool programmor_test_device_publish(programmor_test_device_t *device,
                                    bool common, uint32_t share_id,
                                    const uint8_t *data, size_t data_size) {
  if (device == NULL || common) {
    return false;
  }
  switch (share_id) {
  case 1u: {
    fixtures_test_Share1 value = fixtures_test_Share1_init_zero;
    if (!decode_message(fixtures_test_Share1_fields, &value, data, data_size) ||
        value.startingNumber > value.endingNumber) {
      return false;
    }
    device->share1.startingNumber = value.startingNumber;
    device->share1.endingNumber = value.endingNumber;
    if (device->share1.counter < value.startingNumber ||
        device->share1.counter > value.endingNumber) {
      device->share1.counter = value.startingNumber;
    }
    return true;
  }
  case 2u: {
    fixtures_test_Share2 value = fixtures_test_Share2_init_zero;
    if (!decode_message(fixtures_test_Share2_fields, &value, data, data_size) ||
        !valid_wiring(&value)) {
      return false;
    }
    device->share2 = value;
    return true;
  }
  case 3u:
    return false;
  case 4u: {
    fixtures_test_Share4 value = fixtures_test_Share4_init_zero;
    if (!decode_message(fixtures_test_Share4_fields, &value, data, data_size)) {
      return false;
    }
    device->share4 = value;
    return true;
  }
  case 5u: {
    fixtures_test_Share5 value = fixtures_test_Share5_init_zero;
    if (!decode_message(fixtures_test_Share5_fields, &value, data, data_size) ||
        !isfinite(value.floatNumber) || !isfinite(value.doubleNumber) ||
        value.floatNumber < 0.0F || value.floatNumber > 100.0F ||
        !valid_float_step(value.floatNumber) || value.portNumber < 0 ||
        value.portNumber > 65535) {
      return false;
    }
    device->share5 = value;
    return true;
  }
  case 6u: {
    fixtures_test_Share6 value = fixtures_test_Share6_init_zero;
    if (!decode_message(fixtures_test_Share6_fields, &value, data, data_size) ||
        value.dayOfTheWeek < 1 || value.dayOfTheWeek > 7) {
      return false;
    }
    device->share6 = value;
    return true;
  }
  case 7u: {
    fixtures_test_Share7 value = fixtures_test_Share7_init_zero;
    if (!decode_message(fixtures_test_Share7_fields, &value, data, data_size) ||
        value.memory.size != PROGRAMMOR_TEST_DEVICE_MEMORY_BYTES) {
      return false;
    }
    device->share7 = value;
    return true;
  }
  case 8u: {
    fixtures_test_Share8 value = fixtures_test_Share8_init_zero;
    if (!decode_message(fixtures_test_Share8_fields, &value, data, data_size) ||
        !valid_rows(&value)) {
      return false;
    }
    device->share8 = value;
    return true;
  }
  case 9u:
    return publish_share9(device, data, data_size);
  default:
    return false;
  }
}
