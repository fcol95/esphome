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

struct hlw8112_resistor_ratio {
  float K1_A; /* current channel A */
  float K1_B; /* current channel B */
  float K2;   /* voltage */
};

typedef enum {
  hlw8112_PGA_GAIN_1 = 0,
  hlw8112_PGA_GAIN_2,
  hlw8112_PGA_GAIN_4,
  hlw8112_PGA_GAIN_8,
  hlw8112_PGA_GAIN_16,
} hlw8112_pga_gain_t;

typedef struct {
  hlw8112_pga_gain_t A;
  hlw8112_pga_gain_t B;
  hlw8112_pga_gain_t U;
} hlw8112_pga_t;

typedef enum {
  HLW8112_CHANNEL_A = 0x01,
  HLW8112_CHANNEL_B = 0x02,
  HLW8112_CHANNEL_U = 0x04,
  HLW8112_CHANNEL_ALL = (HLW8112_CHANNEL_A | HLW8112_CHANNEL_B | HLW8112_CHANNEL_U),
} hlw8112_channel_t;

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
  void write_pga(const hlw8112_pga_t *const pga);

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

  hlw8112_pga_t pga_;

  hlw8112_resistor_ratio resistor_ratio_ = {
      .K1_A = 1.0f,
      .K1_B = 1.0f,
      .K2 = 1.0f,
  };

  // Low Level Functions
  uint8_t get_checksum_(const uint8_t command, const uint8_t *data, const size_t len);

  void write_reg_(const uint8_t reg_addr, const uint8_t *data, const size_t len);
  void write_reg_16_(const uint8_t reg_addr, const uint16_t data);
  void read_reg_(const uint8_t reg_addr, uint8_t *data, size_t len);
  void read_reg_16_(const uint8_t reg_addr, uint16_t *data);

  // Special Commands
  void reset_chip_(void);
  void write_reg_enable_(void);
  void write_reg_protect_(void);

  // Calibration Functions
  void read_coeffs_(void);
  void read_pga_();

  // Control Functions
  void enable_channel_(hlw8112_channel_t channel);
  void disable_channel_(hlw8112_channel_t channel);

  // TODO: Add higher level functions to read voltage, current, power, etc. (to be used by update)
};
}  // namespace hlw8112
}  // namespace esphome
