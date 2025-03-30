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

typedef enum {
  CALC_TYPE_RMS = 0,
  CALC_TYPE_POWER,
  CALC_TYPE_ENERGY,
} hlw8112_calc_type_t;

typedef enum {
  hlw8112_PGA_GAIN_1 = 0,
  hlw8112_PGA_GAIN_2,
  hlw8112_PGA_GAIN_4,
  hlw8112_PGA_GAIN_8,
  hlw8112_PGA_GAIN_16,
} hlw8112_pga_gain_t;

typedef enum {
  HLW8112_CHANNEL_A = 0x01,
  HLW8112_CHANNEL_B = 0x02,
  HLW8112_CHANNEL_U = 0x04,
  HLW8112_CHANNEL_ALL = (HLW8112_CHANNEL_A | HLW8112_CHANNEL_B | HLW8112_CHANNEL_U),
} hlw8112_channel_t;

typedef enum {
  HLW8112_DATA_UPDATE_FREQ_HZ_3_4 = 0,
  HLW8112_DATA_UPDATE_FREQ_HZ_6_8,
  HLW8112_DATA_UPDATE_FREQ_HZ_13_65,
  HLW8112_DATA_UPDATE_FREQ_HZ_27_3,
} hlw8112_data_update_freq_t;

typedef enum {
  HLW8112_RMS_MODE_AC = 0,
  HLW8112_RMS_MODE_DC,
} hlw8112_rms_mode_t;

typedef enum {
  HLW8112_B_MODE_TEMPERATURE = 0, /* measure temperature inside the chip only */
  HLW8112_B_MODE_NORMAL,
} hlw8112_channel_b_mode_t;

typedef struct {
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
} hlw8112_coeff_t;

typedef struct {
  float K1_A; /* current channel A */
  float K1_B; /* current channel B */
  float K2;   /* voltage */
} hlw8112_resistor_ratio_t;

typedef struct {
  uint8_t reg_addr;       // TODO: replace with an enum typedef?
  uint16_t coeff;         /* calibration coefficient */
  uint16_t ratio;         /* resistor ratio */
  hlw8112_pga_gain_t pga; /* PGA gain */
  uint8_t mult;           /* multiplier */
  int64_t resol;          /* resolution */
} hlw8112_calc_param_t;

typedef struct {
  hlw8112_pga_gain_t A;
  hlw8112_pga_gain_t B;
  hlw8112_pga_gain_t U;
} hlw8112_pga_t;

class HLW8112 : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void update() override;
  // TODO: Need Loop?
  void dump_config() override;

  // Setters
  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage_sensor_ = voltage_sensor; }
  void set_current_sensor_1(sensor::Sensor *current_sensor_1) { current_sensor_1_ = current_sensor_1; }
  void set_current_sensor_2(sensor::Sensor *current_sensor_2) { current_sensor_2_ = current_sensor_2; }
  void set_power_sensor_1(sensor::Sensor *power_sensor_1) { power_sensor_1_ = power_sensor_1; }
  void set_power_sensor_2(sensor::Sensor *power_sensor_2) { power_sensor_2_ = power_sensor_2; }
  void set_energy_sensor_1(sensor::Sensor *energy_sensor_1) { energy_sensor_1_ = energy_sensor_1; }
  void set_energy_sensor_2(sensor::Sensor *energy_sensor_2) { energy_sensor_2_ = energy_sensor_2; }
  void set_energy_sensor_sum(sensor::Sensor *energy_sensor_sum) { energy_sensor_sum_ = energy_sensor_sum; }
  // Config
  void config_pga(const hlw8112_pga_t pga);
  void config_data_update_freq(const hlw8112_data_update_freq_t freq);
  void config_rms_calc_mode(const hlw8112_rms_mode_t mode);

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

  // Configs
  hlw8112_coeff_t coeffs_;

  hlw8112_pga_t pga_;

  hlw8112_data_update_freq_t data_update_freq_;

  hlw8112_channel_t current_metrics_channel_;

  hlw8112_rms_mode_t rms_mode_;

  hlw8112_channel_b_mode_t channel_b_mode_;

  // Params
  hlw8112_resistor_ratio_t resistor_ratio_ = {
      .K1_A = 1.0f,
      .K1_B = 1.0f,
      .K2 = 1.0f,
  };

  uint32_t clki_hz_ = 3579545UL;  // Clock frequency in Hz (= 3.579545MHz)

  // Low Level Functions
  void write_reg_(const uint8_t reg_addr, const uint8_t *data, const size_t len);
  void write_reg_16_(const uint8_t reg_addr, const uint16_t data);
  void read_reg_(const uint8_t reg_addr, uint8_t *data, size_t len);
  void read_reg_16_(const uint8_t reg_addr, uint16_t *data);

  void get_calc_param_rms_(const hlw8112_channel_t channel, hlw8112_calc_param_t *param);
  void get_calc_param_power_(const hlw8112_channel_t channel, hlw8112_calc_param_t *param);
  void get_calc_param_energy_(const hlw8112_channel_t channel, hlw8112_calc_param_t *param);
  void get_calc_param_(const hlw8112_channel_t channel, const hlw8112_calc_type_t type, hlw8112_calc_param_t *param);

  // Special Commands
  void reset_chip_(void);
  void write_reg_enable_(void);
  void write_reg_protect_(void);

  // Calibration Functions
  void read_coeffs_(void);
  void read_pga_();

  // Control and Config Functions
  void config_channel_enable_(hlw8112_channel_t channel, bool enable);
  void config_channel_b_mode_(hlw8112_channel_b_mode_t mode);  // Set channel B mode (normal or temperature measurement)
  void select_metrics_channel_(
      hlw8112_channel_t channel);  // Set which channel is used to compute metrics/special measurements (apparent power,
                                   // power factor, phase angle, instantaneous apparent power and active power overload)
  // TODO: Add config channel b mode (normal or temperature measurement)
  // TODO: Add config active Power Calculation Method Function
  // TODO: Add config Digital High Pass Filter Function
  // TODO: Add config enabling PFA/B pulses (energy accumulation pulse)
  // TODO: Add config clearing energy cumuation on read for a channel function
  // TODO: Add config enabling Zero Cross Detect Method Function
  // TODO: Add config enabling power factor function
  // TODO: Add config enabling waveform data function
  // TODO: Add config enabling detection functions (Sag, Overvolt, zero crossing, peak)
  // TODO: Add config for comparators
  // TODO: Add config interupt pins management
  // TODO: Add config of SDO pin (open drain)

  // Data Functions
  void measure_channel_rms_(hlw8112_channel_t channel, float *rms);  // Need RMS calculation method set.

  // TODO: void measure_channel_power_(
  //     hlw8112_channel_t channel,
  //     float *power);  // Need both voltage and current channels enabled! Needs power calculation method set.
  // TODO: void measure_channel_energy_(hlw8112_channel_t channel, float *energy);  // Needs enabling enabling PFA/B
  // pulses!
  // TODO: void measure_frequency_(float *frequency);  // Needs waveform and zero crossing detection enabled!

  // TODO: Add reading special measurements of the selected metrics channel (apparent power, power factor, phase angle,
  // instantaneous apparent power and active power overload)
};
}  // namespace hlw8112
}  // namespace esphome
