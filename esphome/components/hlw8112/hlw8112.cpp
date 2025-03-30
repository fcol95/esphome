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
  // Enable Write Register
  this->write_reg_enable_();
  // Read Coefficients
  this->read_coeffs_();
  // Read PGA
  this->read_pga_();  // TODO: Set PGA?

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
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with ATM90E32 failed!");
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("", "Current 1", this->current_sensor_1_);
  LOG_SENSOR("", "Current 2", this->current_sensor_2_);
  LOG_SENSOR("", "Power 1", this->power_sensor_1_);
  LOG_SENSOR("", "Power 2", this->power_sensor_2_);
  LOG_SENSOR("", "Energy 1", this->energy_sensor_1_);
  LOG_SENSOR("", "Energy 2", this->energy_sensor_2_);
  LOG_SENSOR("", "Energy sum", this->energy_sensor_sum_);
}

void HLW8112::write_pga(const hlw8112_pga_t *const pga) {
  uint8_t reg_addr = HLW8112_REG_SYSCON;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to write HLW8112 PGA - Read from register 0x%02X failed!", reg_addr);
    return;
  }

  reg &= ~(0x1FF << 0); /* clear PGA bits */
  reg |= (pga->A << 0) | (pga->U << 3) | (pga->B << 6);

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to write HLW8112 PGA - Write to register 0x%02X failed!", reg_addr);
    return;
  }
  memcpy(&this->pga_, pga, sizeof(hlw8112_pga_t));
  ESP_LOGV(TAG, "PGA set: A=%d, U=%d, B=%d", pga->A, pga->U, pga->B);

  return;
}

//
// Private Methods
//
// Low Level Functions
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
void HLW8112::write_reg_16_(const uint8_t reg_addr, const uint16_t data) {
  uint8_t buffer[2] = {(uint8_t) ((data >> 8) & 0xFF), (uint8_t) (data & 0xFF)};
  this->write_reg_(reg_addr, buffer, 2);
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
    return;
  }
  uint8_t checksum = get_checksum_(command, data, len - 1);  // Ignore last received byte which is checksum
  if (data[len - 1] != checksum) {
    ESP_LOGE(TAG, "Failed to read HLW8112 register 0x%02X - wrong checksum!", reg_addr);
    this->mark_failed();
  }
  return;
}
void HLW8112::read_reg_16_(const uint8_t reg_addr, uint16_t *data) {
  uint8_t readback[2] = {0};
  this->read_reg_(reg_addr, readback, 2);
  *data = (readback[0] << 8) | readback[1];
}

// Special Commands
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
void HLW8112::read_coeffs_(void) {
  uint16_t coeffs_checksum = {0};
  hlw8112_coeff coeffs;

  this->read_reg_16_(HLW8112_REG_HFCONST, &coeffs.hfconst);  // Not really a coefficient, but still needed
  this->read_reg_16_(HLW8112_REG_RMSIAC, &coeffs.rms.A);
  this->read_reg_16_(HLW8112_REG_RMSIBC, &coeffs.rms.B);
  this->read_reg_16_(HLW8112_REG_RMSUC, &coeffs.rms.U);
  this->read_reg_16_(HLW8112_REG_POWER_PAC, &coeffs.power.A);
  this->read_reg_16_(HLW8112_REG_POWER_PBC, &coeffs.power.B);
  this->read_reg_16_(HLW8112_REG_POWER_SC, &coeffs.power.S);
  this->read_reg_16_(HLW8112_REG_ENERGY_AC, &coeffs.energy.A);
  this->read_reg_16_(HLW8112_REG_ENERGY_BC, &coeffs.energy.B);
  this->read_reg_16_(HLW8112_REG_COEFF_CHKSM, &coeffs_checksum);

  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to read HLW8112 coefficients!");
    return;
  }

  const uint16_t coeffs_checksum_calc =
      (uint16_t) ~(0xFFFFu + coeffs.rms.A + coeffs.rms.B + coeffs.rms.U + coeffs.power.A + coeffs.power.B +
                   coeffs.power.S + coeffs.energy.A + coeffs.energy.B);

  if (coeffs_checksum != coeffs_checksum_calc) {
    this->mark_failed();
    ESP_LOGE(TAG, "HLW8112 coefficients checksum mismatch! Expected: 0x%04X, got: 0x%04X", coeffs_checksum_calc,
             coeffs_checksum);
    return;
  }

  memcpy(&this->coeffs_, &coeffs, sizeof(hlw8112_coeff));

  ESP_LOGV(TAG,
           "Coefficients: HFConst=%d, "
           "RMS_A=%d, RMS_B=%d, RMS_U=%d, "
           "Power_A=%d, Power_B=%d, Power_S=%d, "
           "Energy_A=%d, Energy_B=%d",
           coeffs.hfconst, coeffs.rms.A, coeffs.rms.B, coeffs.rms.U, coeffs.power.A, coeffs.power.B, coeffs.power.S,
           coeffs.energy.A, coeffs.energy.B);

  return;
}

void HLW8112::read_pga_(void) {
  uint8_t reg_addr = HLW8112_REG_SYSCON;
  uint16_t reg;

  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to read HLW8112 PGA - Read from register 0x%02X failed!", reg_addr);
    return;
  }

  this->pga_.A = (hlw8112_pga_gain_t) ((reg >> 0) & 0x07);
  this->pga_.U = (hlw8112_pga_gain_t) ((reg >> 3) & 0x07);
  this->pga_.B = (hlw8112_pga_gain_t) ((reg >> 6) & 0x07);

  ESP_LOGV(TAG, "PGA read: A=%d, U=%d, B=%d", this->pga_.A, this->pga_.U, this->pga_.B);

  return;
}

void HLW8112::enable_channel_(hlw8112_channel_t channel) {
  uint8_t reg_addr = HLW8112_REG_SYSCON;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to enable HLW8112 channel %d - Read from register 0x%02X failed!", channel, reg_addr);
    return;
  }

  if (channel & HLW8112_CHANNEL_A) {
    reg |= 1 << 9; /* ADC1ON */
  }
  if (channel & HLW8112_CHANNEL_B) {
    reg |= 1 << 10; /* ADC2ON */
  }
  if (channel & HLW8112_CHANNEL_U) {
    reg |= 1 << 11; /* ADC3ON */
  }

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to enable HLW8112 channel %d - Write to register 0x%02X failed!", channel, reg_addr);
    return;
  }
  ESP_LOGV(TAG, "Channel enabled: %d", channel);

  return;
}

void HLW8112::disable_channel_(hlw8112_channel_t channel) {
  uint8_t reg_addr = HLW8112_REG_SYSCON;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to disable HLW8112 channel %d - Read from register 0x%02X failed!", channel, reg_addr);
    return;
  }

  if (channel & HLW8112_CHANNEL_A) {
    reg &= ~(1 << 9); /* ADC1ON */
  }
  if (channel & HLW8112_CHANNEL_B) {
    reg &= ~(1 << 10); /* ADC2ON */
  }
  if (channel & HLW8112_CHANNEL_U) {
    reg &= ~(1 << 11); /* ADC3ON */
  }

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to disable HLW8112 channel %d - Write to register 0x%02X failed!", channel, reg_addr);
    return;
  }
  ESP_LOGV(TAG, "Channel disable: %d", channel);

  return;
}

}  // namespace hlw8112
}  // namespace esphome
