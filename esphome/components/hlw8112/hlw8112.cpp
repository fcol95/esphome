#include "hlw8112.h"
#include "hlw8112_reg.h"

#include "esphome/core/log.h"

#include <cinttypes>

#define WRITE_REG_COMMAND(command) ((command) | 0x80)  // Set the MSB to 1
#define READ_REG_COMMAND(command) ((command) &0x7F)    // Reset the MSB to 0
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

namespace esphome {
namespace hlw8112 {

static const char *const TAG = "hlw8112";
//
// Public Methods
//
void HLW8112::setup() {
  // Reset Chip
  this->reset_chip_();
  // TODO: Remove write protection? 0xEA command with 0xE5 data (need to reanable it after??)
  // System Control: Enable Voltage and Current Channels - Enable all channels (default value)
  const uint8_t init_data_sys_cont[] = {0x0A, 0x04};
  this->write_reg_(HLW8112_REG_SYSCON, init_data_sys_cont, LENGTH_OF(init_data_sys_cont));
  // Energy Measure Control: Set measurement mode - Default value
  const uint8_t init_data_meas_cont[] = {0x00, 0x00};
  this->write_reg_(HLW8112_REG_EMUCON, init_data_meas_cont, LENGTH_OF(init_data_meas_cont));
  this->flush();
}

void HLW8112::update() {
  if (!this->available()) {
    this->status_set_warning("UART unavailable with HLW8112!");
    return;
  }
  this->flush();
  // TODO: Read measurements!!
}

void HLW8112::dump_config() {  // NOLINT(readability-function-cognitive-complexity)
  ESP_LOGCONFIG(TAG, "HLW8112:");
  LOG_SENSOR("", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("", "Current 1", this->current_sensor_1_);
  LOG_SENSOR("", "Current 2", this->current_sensor_2_);
  LOG_SENSOR("", "Power 1", this->power_sensor_1_);
  LOG_SENSOR("", "Power 2", this->power_sensor_2_);
  LOG_SENSOR("", "Energy 1", this->energy_sensor_1_);
  LOG_SENSOR("", "Energy 2", this->energy_sensor_2_);
  LOG_SENSOR("", "Energy sum", this->energy_sensor_sum_);
}

//
// Private Methods
//
uint8_t HLW8112::get_checksum_(const uint8_t command, const uint8_t *data, const size_t len) {
  uint8_t checksum = HLW8112_PACKET_HEADER;
  checksum += command;
  for (size_t i = 0; i < len; i++) {
    checksum += data[i];
  }
  checksum ^= 0xFF;  // Flip (invert) all bits
  return checksum;
}

void HLW8112::write_reg_(const uint8_t reg_addr, const uint8_t *data, size_t len) {
  const uint8_t command = WRITE_REG_COMMAND(reg_addr);
  this->flush();
  this->write_byte(HLW8112_PACKET_HEADER);
  this->write_byte(command);
  this->write_array(data, len);
  this->write_byte(get_checksum_(command, data, len));

  uint8_t readback[len] = {0};
  this->read_reg_(reg_addr, readback, len);
  for (size_t i = 0; i < len; i++) {
    if (readback[i] != data[i]) {
      ESP_LOGE(TAG, "Failed to write HLW8112 register 0x%02X", reg_addr);
      this->mark_failed();
      return;
    }
  }
}

void HLW8112::read_reg_(const uint8_t reg_addr, uint8_t *data, size_t len) {
  const uint8_t command = READ_REG_COMMAND(reg_addr);
  this->flush();
  this->write_byte(HLW8112_PACKET_HEADER);
  this->write_byte(command);
  bool success = this->read_array(data, len);
  if (!success) {
    ESP_LOGE(TAG, "Failed to read HLW8112 register 0x%02X - read failed!", reg_addr);
    this->mark_failed();
  }
  uint8_t checksum = get_checksum_(command, data, len - 1);  // Ignore last received byte which is checksum
  if (data[len - 1] != checksum) {
    ESP_LOGE(TAG, "Failed to read HLW8112 register 0x%02X - wrong checksum!", reg_addr);
    this->mark_failed();
  }
  return;
}
void HLW8112::reset_chip_(void) {
  uint8_t special_reg_value = HLW8112_COMMAND_RESET;
  this->write_reg_(HLW8112_REG_SPECIAL, &special_reg_value, 1);
}

void HLW8112::write_reg_enable_(void) {
  uint8_t special_reg_value = HLW8112_COMMAND_WRITE_EN;
  this->write_reg_(HLW8112_REG_SPECIAL, &special_reg_value, 1);
}

void HLW8112::write_reg_protect_(void) {
  uint8_t special_reg_value = HLW8112_COMMAND_WRITE_PROTECT;
  this->write_reg_(HLW8112_REG_SPECIAL, &special_reg_value, 1);
}

}  // namespace hlw8112
}  // namespace esphome
