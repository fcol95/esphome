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

static uint16_t convert_float_to_uint16_centi_(float val) { return (uint16_t) (val * 100); }
static int32_t convert_24bits_to_int32(const uint8_t buf[3]) {
#if defined(HLW8112_BIG_ENDIAN)
  return (int32_t) ((buf[2] << 16) | (buf[1] << 8) | buf[0]);
#else
  return (int32_t) ((buf[0] << 16) | (buf[1] << 8) | buf[2]);
#endif
}

static uint8_t get_checksum_(const uint8_t command, const uint8_t *data, const size_t len) {
  uint8_t checksum = HLW8112_PACKET_HEADER;
  checksum += command;
  for (size_t i = 0; i < len; i++) {
    checksum += data[i];
  }
  checksum ^= 0xFF;  // Flip (invert) all bits
  return checksum;
}

//
// Public Methods
//
void HLW8112::setup() {
  // Reset Chip
  this->reset_chip_();
  // TODO: Ensure no sleep is needed here?
  // Enable Write Register
  this->write_reg_enable_();
  // Read Coefficients
  this->read_coeffs_();
  // Read PGA
  this->read_pga_();  // TODO: Set PGA? Gain 2?
  // Set data update frequency
  this->config_data_update_freq(HLW8112_DATA_UPDATE_FREQ_HZ_3_4);
  // Select Initial Channel for Metrics
  this->select_metrics_channel_(HLW8112_CHANNEL_A);
  // Set RMS Calculation Mode
  this->config_rms_calc_mode(HLW8112_RMS_MODE_AC);
  // Set Channel B Mode
  this->config_channel_b_mode_(HLW8112_B_MODE_NORMAL);
  // Enable Channel
  this->config_channel_enable_(HLW8112_CHANNEL_ALL, true);

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
  // TODO: Add actual config values, not just sensors
  LOG_SENSOR("", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("", "Current 1", this->current_sensor_1_);
  LOG_SENSOR("", "Current 2", this->current_sensor_2_);
  LOG_SENSOR("", "Power 1", this->power_sensor_1_);
  LOG_SENSOR("", "Power 2", this->power_sensor_2_);
  LOG_SENSOR("", "Energy 1", this->energy_sensor_1_);
  LOG_SENSOR("", "Energy 2", this->energy_sensor_2_);
  LOG_SENSOR("", "Energy sum", this->energy_sensor_sum_);
}

void HLW8112::config_pga(const hlw8112_pga_t pga) {
  uint8_t reg_addr = HLW8112_REG_SYSCON;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to write HLW8112 PGA - Read from register 0x%02X failed!", reg_addr);
    return;
  }

  reg &= ~(0x1FF << HLW8112_REG_SYSCON_PGAIA);          /* clear all PGA bits */
  reg |= ((pga.A & 0b111) << HLW8112_REG_SYSCON_PGAIA); /* set PGAIA bits */
  reg |= ((pga.U & 0b111) << HLW8112_REG_SYSCON_PGAU);  /* set PGAU bits */
  reg |= ((pga.B & 0b111) << HLW8112_REG_SYSCON_PGAIB); /* set PGAIB bits */

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to write HLW8112 PGA - Write to register 0x%02X failed!", reg_addr);
    return;
  }
  memcpy(&this->pga_, &pga, sizeof(hlw8112_pga_t));
  ESP_LOGV(TAG, "PGA set: A=%d, U=%d, B=%d", pga.A, pga.U, pga.B);

  return;
}

void HLW8112::config_data_update_freq(hlw8112_data_update_freq_t freq) {
  uint8_t reg_addr = HLW8112_REG_EMUCON2;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to set HLW8112 data update frequency - Read from register 0x%02X failed!", reg_addr);
    return;
  }
  reg &= ~(0b11 << HLW8112_REG_EMUCON2_DUPSEL);       /* clear DUP bits */
  reg |= (freq & 0b11 << HLW8112_REG_EMUCON2_DUPSEL); /* set DUP bits */

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to set HLW8112 data update frequency - Write to register 0x%02X failed!", reg_addr);
    return;
  }
}

void HLW8112::config_rms_calc_mode(const hlw8112_rms_mode_t mode) {
  uint8_t reg_addr = HLW8112_REG_EMUCON;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to set HLW8112 RMS calculation mode - Read from register 0x%02X failed!", reg_addr);
    return;
  }
  reg &= ~(0b1 << HLW8112_REG_EMUCON_DC_MODE);       /* clear DC_MODE bits */
  reg |= (mode & 0b1 << HLW8112_REG_EMUCON_DC_MODE); /* set DC_MODE bits */

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to set HLW8112 RMS calculation mode - Write to register 0x%02X failed!", reg_addr);
    return;
  }
  memcpy(&this->rms_mode_, &mode, sizeof(hlw8112_rms_mode_t));
}

//
// Private Methods
//
// Low Level Functions

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
      ESP_LOGV(TAG, "Failed to write HLW8112 register 0x%02X", reg_addr);
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
    ESP_LOGV(TAG, "Failed to read HLW8112 register 0x%02X - read array failed!", reg_addr);
    this->mark_failed();
    return;
  }
  uint8_t checksum = get_checksum_(command, data, len - 1);  // Ignore last received byte which is checksum
  if (data[len - 1] != checksum) {
    ESP_LOGV(TAG, "Failed to read HLW8112 register 0x%02X - wrong checksum!", reg_addr);
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
  hlw8112_coeff_t coeffs;

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

  memcpy(&this->coeffs_, &coeffs, sizeof(hlw8112_coeff_t));

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

  this->pga_.A = (hlw8112_pga_gain_t) ((reg >> HLW8112_REG_SYSCON_PGAIA) & 0b111);
  this->pga_.U = (hlw8112_pga_gain_t) ((reg >> HLW8112_REG_SYSCON_PGAU) & 0b111);
  this->pga_.B = (hlw8112_pga_gain_t) ((reg >> HLW8112_REG_SYSCON_PGAIB) & 0b111);

  ESP_LOGV(TAG, "PGA read: A=%d, U=%d, B=%d", this->pga_.A, this->pga_.U, this->pga_.B);

  return;
}

void HLW8112::config_channel_b_mode_(hlw8112_channel_b_mode_t mode) {
  uint8_t reg_addr = HLW8112_REG_EMUCON2;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to set HLW8112 channel B mode - Read from register 0x%02X failed!", reg_addr);
    return;
  }
  reg &= ~(0b1 << HLW8112_REG_EMUCON2_CHS_IB);       /* clear CHS_IB bits */
  reg |= (mode & 0b1 << HLW8112_REG_EMUCON2_CHS_IB); /* set CHS_IB bits */

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to set HLW8112 channel B mode - Write to register 0x%02X failed!", reg_addr);
    return;
  }
}

void HLW8112::config_channel_enable_(hlw8112_channel_t channel, bool enable) {
  const char *enable_str = enable ? "enable" : "disable";
  uint8_t reg_addr = HLW8112_REG_SYSCON;
  uint16_t reg;
  this->read_reg_16_(reg_addr, &reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to %s HLW8112 channel %d - Read from register 0x%02X failed!", enable_str, channel, reg_addr);
    return;
  }
  if (enable) {
    if (channel & HLW8112_CHANNEL_A) {
      reg |= 1 << HLW8112_REG_SYSCON_ADC1ON; /* Set ADC1ON bit */
    }
    if (channel & HLW8112_CHANNEL_B) {
      reg |= 1 << HLW8112_REG_SYSCON_ADC2ON; /* Set ADC2ON bit */
    }
    if (channel & HLW8112_CHANNEL_U) {
      reg |= 1 << HLW8112_REG_SYSCON_ADC3ON; /* Set ADC3ON bit */
    }
  } else {
    if (channel & HLW8112_CHANNEL_A) {
      reg &= ~(1 << HLW8112_REG_SYSCON_ADC1ON); /* Clear ADC1ON bit */
    }
    if (channel & HLW8112_CHANNEL_B) {
      reg &= ~(1 << HLW8112_REG_SYSCON_ADC2ON); /* Clear ADC2ON bit */
    }
    if (channel & HLW8112_CHANNEL_U) {
      reg &= ~(1 << HLW8112_REG_SYSCON_ADC1ON); /* Clear ADC3ON bit */
    }
  }

  this->write_reg_16_(reg_addr, reg);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to %s HLW8112 channel %d - Write to register 0x%02X failed!", enable_str, channel, reg_addr);
    return;
  }
  ESP_LOGV(TAG, "Channel enabled: %d", channel);

  // TODO: Save enabled/disabled channel in object field?

  return;
}

void HLW8112::select_metrics_channel_(hlw8112_channel_t channel) {
  uint8_t cmd;

  switch (channel) {
    case HLW8112_CHANNEL_A:
      cmd = HLW8112_COMMAND_SELECT_CH_A;
      break;
    case HLW8112_CHANNEL_B:
      cmd = HLW8112_COMMAND_SELECT_CH_B;
      break;
    default:
      ESP_LOGE(TAG, "Invalid channel for metrics! Got %d. Expect A or B.", channel);
      return;
  }

  write_reg_(HLW8112_REG_SPECIAL, &cmd, 1);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to select HLW8112 channel %d - Write to register 0x%02X failed!", channel,
             HLW8112_REG_SPECIAL);
    return;
  }
  this->current_metrics_channel_ = channel;
}

// Data Functions
void HLW8112::get_calc_param_rms_(const hlw8112_channel_t channel, hlw8112_calc_param_t *param) {
  if (channel == HLW8112_CHANNEL_A) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_RMSIA,
        .coeff = this->coeffs_.rms.A,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K1_A),
        .pga = this->pga_.A,
        .resol = 1ll << 23,
    };
  } else if (channel == HLW8112_CHANNEL_B) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_RMSIB,
        .coeff = this->coeffs_.rms.B,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K1_B),
        .pga = this->pga_.B,
        .resol = 1ll << 23,
    };
  } else if (channel == HLW8112_CHANNEL_U) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_RMSU,
        .coeff = this->coeffs_.rms.U,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K2),
        .pga = this->pga_.U,
        .mult = 10,
        .resol = 1ll << 22,
    };
  } else {
    ESP_LOGE(TAG, "Invalid channel for RMS measurements: %d!", channel);
  }
}

void HLW8112::get_calc_param_power_(const hlw8112_channel_t channel, hlw8112_calc_param_t *param) {
  if (channel == HLW8112_CHANNEL_A) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_POWER_PA,
        .coeff = this->coeffs_.power.A,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K1_A),
        .pga = this->pga_.A,
    };
  } else if (channel == HLW8112_CHANNEL_B) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_POWER_PB,
        .coeff = this->coeffs_.power.B,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K1_B),
        .pga = this->pga_.B,
    };
  } else if (channel == HLW8112_CHANNEL_U) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_POWER_S,
        .coeff = this->coeffs_.power.S,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K2),
        .pga = this->pga_.U,
    };
  } else {
    ESP_LOGE(TAG, "Invalid channel for power measurement: %d!", channel);
  }

  param->resol = 1ll << 31;
}

void HLW8112::get_calc_param_energy_(const hlw8112_channel_t channel, hlw8112_calc_param_t *param) {
  if (channel == HLW8112_CHANNEL_A) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_ENERGY_PA,
        .coeff = this->coeffs_.energy.A,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K1_A),
        .pga = this->pga_.A,
    };
  } else if (channel == HLW8112_CHANNEL_B) {
    *param = (hlw8112_calc_param_t){
        .reg_addr = HLW8112_REG_ENERGY_PB,
        .coeff = this->coeffs_.energy.B,
        .ratio = convert_float_to_uint16_centi_(this->resistor_ratio_.K1_B),
        .pga = this->pga_.B,
    };
  } else {
    ESP_LOGE(TAG, "Invalid channel for energy measurement: %d!", channel);
  }

  param->resol = 1ll << 29;
}

void HLW8112::get_calc_param_(const hlw8112_channel_t channel, const hlw8112_calc_type_t type,
                              hlw8112_calc_param_t *param) {
  memset(param, 0, sizeof(*param));

  if (type == CALC_TYPE_RMS) {
    this->get_calc_param_rms_(channel, param);
  } else if (type == CALC_TYPE_POWER) {
    this->get_calc_param_power_(channel, param);
  } else if (type == CALC_TYPE_ENERGY) {
    this->get_calc_param_energy_(channel, param);
  } else {
    ESP_LOGE(TAG, "Invalid calculation type for getting parameters: %d!", type);
  }
}

void HLW8112::measure_channel_rms_(hlw8112_channel_t channel, float *rms) {
  uint8_t buf[3];
  hlw8112_calc_param_t param;

  this->get_calc_param_(channel, CALC_TYPE_RMS, &param);

  this->read_reg_(param.reg_addr, buf, sizeof(buf));
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Failed to read HLW8112 RMS register 0x%02X!", param.reg_addr);
    return;
  }

  const int32_t raw = convert_24bits_to_int32(buf);
  if (raw & (1 << 23)) {  // Check if last bit indicates a negative value
    ESP_LOGE(TAG, "HLW8112 RMS register 0x%02X returned negative value: %d", param.reg_addr, raw);
    this->mark_failed();
    return;
  }

  /* Multiply by 1000 first to milliunits and then divide by 10 to avoid losing
   * significant digits during the calculation. */
  int64_t val = ((int64_t) raw * param.coeff * 1000) / (param.ratio * param.resol);

  if (channel == HLW8112_CHANNEL_U) {  // Voltage
    val = val * param.mult / (1 << param.pga) / 10;
  } else {  // Current
    val = val * (16 >> param.pga) / 10;
  }

  // Convert to float and divide back by 1000 from milliunits
  *rms = (float) val / 1000.0f;

  return;
}

}  // namespace hlw8112
}  // namespace esphome
