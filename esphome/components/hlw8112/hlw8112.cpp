#include "hlw8112.h"
#include "esphome/core/log.h"
#include <cinttypes>

#define WRITE_REG_COMMAND(command) ((command) | 0x80)  // Set the MSB to 1
#define READ_REG_COMMAND(command) ((command) &0x7F)    // Reset the MSB to 0
#define LENGTH_OF(x) (sizeof(x) / sizeof(x[0]))

namespace esphome {
namespace hlw8112 {

static const char *const TAG = "hlw8112";

// UART Packet Header
static const uint8_t HLW8112_PACKET_HEADER = 0xA5;

// Commands Value
static const uint8_t HLW8112_REG_SPECIAL = 0xEA;            // Special command operations
static const uint8_t HLW8112_COMMAND_WRITE_EN = 0xE5;       // Special command operations
static const uint8_t HLW8112_COMMAND_WRITE_PROTECT = 0xDC;  // Special command operations
static const uint8_t HLW8112_COMMAND_SELECT_CH_A = 0x5A;    // Special command operations
static const uint8_t HLW8112_COMMAND_SELECT_CH_B = 0xA5;    // Special command operations
static const uint8_t HLW8112_COMMAND_RESET = 0x96;          // Special command operations

// Register addresses
static const uint8_t HLW8112_REG_SYSCON = 0x00;   // System Control Register
static const uint8_t HLW8112_REG_EMUCON = 0x01;   // Meter Control Register
static const uint8_t HLW8112_REG_HFCONST = 0x02;  // Pulse Frequency Register
static const uint8_t HLW8112_REG_PSTARTA = 0x03;  // Active Start Power Setting for Channel A
static const uint8_t HLW8112_REG_PSTARTB = 0x04;  // Active Start Power Setting for Channel B
static const uint8_t HLW8112_REG_PAGAIN = 0x05;   // Channel A Power Gain Calibration Register
static const uint8_t HLW8112_REG_PBGAIN = 0x06;   // Channel B Power Gain Calibration Register
static const uint8_t HLW8112_REG_PHASEA = 0x07;   // Channel A Phase Calibration Register
static const uint8_t HLW8112_REG_PHASEB = 0x08;   // Channel B Phase Calibration Register
static const uint8_t HLW8112_REG_PAOS = 0x0A;     // Channel A Active Power Offset Calibration
static const uint8_t HLW8112_REG_PBOS = 0x0B;     // Channel B Active Power Offset Calibration
static const uint8_t HLW8112_REG_RMSIAOS = 0x0E;  // Current Channel A RMS Offset Compensation
static const uint8_t HLW8112_REG_RMSIBOS = 0x0F;  // Current Channel B RMS Offset Compensation
static const uint8_t HLW8112_REG_IBGAIN = 0x10;   // Current Channel B Gain Settings
static const uint8_t HLW8112_REG_PSGAIN = 0x11;   // Apparent Power Gain Calibration
static const uint8_t HLW8112_REG_PSOS = 0x12;     // Apparent Power Offset Compensation
static const uint8_t HLW8112_REG_EMUCON2 = 0x13;  // Meter Control Register 2
static const uint8_t HLW8112_REG_DCIA = 0x14;     // IA Channel DC Offset Correction Register
static const uint8_t HLW8112_REG_DCIB = 0x15;     // IB Channel DC Offset Correction Register
static const uint8_t HLW8112_REG_DCIC = 0x16;     // U Channel DC Offset Correction Register
static const uint8_t HLW8112_REG_SAGCYC = 0x17;   // Voltage Sag Period Setting
static const uint8_t HLW8112_REG_SAGLVL = 0x18;   // Voltage Sag Threshold Setting
static const uint8_t HLW8112_REG_OVLVL = 0x19;    // Voltage Overvoltage Threshold Setting
static const uint8_t HLW8112_REG_OIALVL = 0x1A;   // Current Channel A Overcurrent Threshold Setting
static const uint8_t HLW8112_REG_OIBLVL = 0x1B;   // Current Channel B Overcurrent Threshold Setting
static const uint8_t HLW8112_REG_OPLVL = 0x1C;    // Threshold Setting of Active Power Overload
static const uint8_t HLW8112_REG_INT = 0x1D;      // INT1/INT2 Interrupt Setting

// Meter Parameter and Status Registers
static const uint8_t HLW8112_REG_PFCntPA = 0x20;       // Fast Combination Active Pulse Counting of Channel A
static const uint8_t HLW8112_REG_PFCntPB = 0x21;       // Fast Combination Active Pulse Counting of Channel B
static const uint8_t HLW8112_REG_ANGLE = 0x22;         // Angle between Current and Voltage (Channel A/B)
static const uint8_t HLW8112_REG_UFREQ = 0x23;         // Voltage Frequency (L Line)
static const uint8_t HLW8112_REG_RMSIA = 0x24;         // RMS Current for Channel A (3 bytes)
static const uint8_t HLW8112_REG_RMSIB = 0x25;         // RMS Current for Channel B (3 bytes)
static const uint8_t HLW8112_REG_RMSU = 0x26;          // RMS Voltage (3 bytes)
static const uint8_t HLW8112_REG_POWER_FACTOR = 0x27;  // Power Factor Register (Channel A or B)
static const uint8_t HLW8112_REG_ENERGY_PA = 0x28;     // Channel A Active Power (reset after reading)
static const uint8_t HLW8112_REG_ENERGY_PB = 0x29;     // Channel B Active Power (reset after reading)
static const uint8_t HLW8112_REG_POWER_PA = 0x2C;      // Active Power of Channel A (4 bytes)
static const uint8_t HLW8112_REG_POWER_PB = 0x2D;      // Active Power of Channel B (4 bytes)
static const uint8_t HLW8112_REG_POWER_S = 0x2E;       // Apparent Power of Channel A/B (4 bytes)
static const uint8_t HLW8112_REG_EMU_STATUS = 0x2F;    // Measurement Status and Check Register
static const uint8_t HLW8112_REG_PEAKIA = 0x30;        // Peak of Current Channel A
static const uint8_t HLW8112_REG_PEAKIB = 0x31;        // Peak of Current Channel B
static const uint8_t HLW8112_REG_PEAKU = 0x32;         // Peak Value of Voltage Channel U
static const uint8_t HLW8112_REG_INSTANTIA = 0x33;     // Instantaneous Value of Current Channel A
static const uint8_t HLW8112_REG_INSTANTIB = 0x34;     // Instantaneous Value of Current Channel B
static const uint8_t HLW8112_REG_INSTANTU = 0x35;      // Instantaneous Value of Voltage Channel
static const uint8_t HLW8112_REG_WAVEIA = 0x36;        // Waveform of Current Channel A
static const uint8_t HLW8112_REG_WAVEIB = 0x37;        // Waveform of Current Channel B
static const uint8_t HLW8112_REG_WAVEU = 0x38;         // Waveform of Voltage Channel U
static const uint8_t HLW8112_REG_INSTANTP = 0x3C;      // Instantaneous Active Power (Channel A or B)
static const uint8_t HLW8112_REG_INSTANTS = 0x3D;      // Instantaneous Apparent Power (Channel A or B)

// Interrupt Registers
static const uint8_t HLW8112_REG_IE = 0x40;   // Interrupt Enable Register
static const uint8_t HLW8112_REG_IF = 0x41;   // Interrupt Flag Register
static const uint8_t HLW8112_REG_RIF = 0x42;  // Reset Interrupt Status Register

// System Status Registers
static const uint8_t HLW8112_REG_SYS_STATUS = 0x43;  // System Status Register
static const uint8_t HLW8112_REG_RDATA = 0x44;       // Data Read by SPI last time
static const uint8_t HLW8112_REG_WDATA = 0x45;       // Data Written by the last SPI

// Calibration Coefficients
static const uint8_t HLW8112_REG_RMSIAC = 0x70;     // Current Channel A RMS Conversion Coefficient
static const uint8_t HLW8112_REG_RMSIBC = 0x71;     // Current Channel B RMS Conversion Coefficient
static const uint8_t HLW8112_REG_RMSUC = 0x72;      // Voltage Channel RMS Conversion Coefficient
static const uint8_t HLW8112_REG_POWER_PAC = 0x73;  // Active Power Conversion Coefficient for Channel A
static const uint8_t HLW8112_REG_POWER_PBC = 0x74;  // Active Power Conversion Coefficient for Channel B
static const uint8_t HLW8112_REG_POWER_SC = 0x75;   // Apparent Power Conversion Coefficient
static const uint8_t HLW8112_REG_ENERGY_AC = 0x76;  // Energy Conversion Coefficient for Channel A
static const uint8_t HLW8112_REG_ENERGY_BC = 0x77;  // Energy Conversion Coefficient for Channel B

// Functions
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

void HLW8112::setup() {
  // TODO: Reset instruction? 0xEA command with 0x96 data - should take two clock cycles
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

}  // namespace hlw8112
}  // namespace esphome
