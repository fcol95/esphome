#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace hlw8112 {

// References:
// https://redmine.laas.fr/attachments/download/3444/DS_HLW8110_HLW8112_EN_Rev1.01%20.pdf
// https://github.com/MahdaSystem/HLW811x
// https://github.com/libmcu/hlw811x

// TODO: Support SPI config
// TODO: Support variants (HLW8112 vs HLW8110)
// TODO: Ensure uart speed and CS/SCLK state are correct?

struct hlw8112_coeff {
  struct {
    uint16_t A; /* RMS conversion coefficient for current channel A */
    uint16_t B; /* RMS conversion coefficient for current channel B */
    uint16_t U; /* RMS conversion coefficient for voltage */
  } rms;
  struct {
    uint16_t A; /* Active power conversion coefficient for channel A */
    uint16_t B; /* Active power conversion coefficient for channel B */
    uint16_t S; /* Apparent power conversion coefficient */
  } power;
  struct {
    uint16_t A; /* Active energy conversion coefficient for channel A */
    uint16_t B; /* Active energy conversion coefficient for channel B */
  } energy;

  uint16_t hfconst; /* pulse frequency constant */
};

struct hlw811x_resistor_ratio {
  float K1_A; /* current channel A */
  float K1_B; /* current channel B */
  float K2;   /* voltage */
};

class HLW8112 : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void update() override;
  void dump_config() override;

  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor_1(sensor::Sensor *current_sensor_1) { current_sensor_1_ = current_sensor_1; }
  void set_current_sensor_2(sensor::Sensor *current_sensor_2) { current_sensor_2_ = current_sensor_2; }
  void set_power_sensor_1(sensor::Sensor *power_sensor_1) { power_sensor_1_ = power_sensor_1; }
  void set_power_sensor_2(sensor::Sensor *power_sensor_2) { power_sensor_2_ = power_sensor_2; }
  void set_energy_sensor_1(sensor::Sensor *energy_sensor_1) { energy_sensor_1_ = energy_sensor_1; }
  void set_energy_sensor_2(sensor::Sensor *energy_sensor_2) { energy_sensor_2_ = energy_sensor_2; }
  void set_energy_sensor_sum(sensor::Sensor *energy_sensor_sum) { energy_sensor_sum_ = energy_sensor_sum; }

 protected:
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_1_{nullptr};
  sensor::Sensor *current_sensor_2_{nullptr};
  // NB This may be negative as the circuits is seemingly able to measure
  // power in both directions
  sensor::Sensor *power_sensor_1_{nullptr};
  sensor::Sensor *power_sensor_2_{nullptr};
  sensor::Sensor *energy_sensor_1_{nullptr};
  sensor::Sensor *energy_sensor_2_{nullptr};
  sensor::Sensor *energy_sensor_sum_{nullptr};

  hlw8112_coeff coeffs_;

  // Low Level Functions
  uint8_t get_checksum_(const uint8_t command, const uint8_t *data, const size_t len);

  void write_reg_(const uint8_t reg_addr, const uint8_t *data, const size_t len);
  void read_reg_(const uint8_t reg_addr, uint8_t *data, size_t len);
  void read_reg_16_(const uint8_t reg_addr, uint16_t *data);

  // Special Commands
  void reset_chip_(void);
  void write_reg_enable_(void);
  void write_reg_protect_(void);

  // Calibration Functions
  void read_coeffs_(void);
  void set_resistor_ratio_(hlw811x_resistor_ratio *ratio);
  void get_resistor_ratio_(hlw811x_resistor_ratio *ratio);
  // TODO: Add higher level functions to read voltage, current, power, etc. (to be used by update)
};
}  // namespace hlw8112
}  // namespace esphome
