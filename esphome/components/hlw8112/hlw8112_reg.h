#pragma once

#include <cinttypes>

namespace esphome {
namespace hlw8112 {

// UART Packet Header
static const uint8_t HLW8112_PACKET_HEADER = 0xA5;

// Register addresses
static const uint8_t HLW8112_REG_SPECIAL = 0xEA;  // Special command operations
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
static const uint8_t HLW8112_REG_RMSIAC = 0x70;       // Current Channel A RMS Conversion Coefficient
static const uint8_t HLW8112_REG_RMSIBC = 0x71;       // Current Channel B RMS Conversion Coefficient
static const uint8_t HLW8112_REG_RMSUC = 0x72;        // Voltage Channel RMS Conversion Coefficient
static const uint8_t HLW8112_REG_POWER_PAC = 0x73;    // Active Power Conversion Coefficient for Channel A
static const uint8_t HLW8112_REG_POWER_PBC = 0x74;    // Active Power Conversion Coefficient for Channel B
static const uint8_t HLW8112_REG_POWER_SC = 0x75;     // Apparent Power Conversion Coefficient
static const uint8_t HLW8112_REG_ENERGY_AC = 0x76;    // Energy Conversion Coefficient for Channel A
static const uint8_t HLW8112_REG_ENERGY_BC = 0x77;    // Energy Conversion Coefficient for Channel B
static const uint8_t HLW8112_REG_COEFF_CHKSM = 0x6F;  // Coefficients Checksum Register

// Special Register Commands Value
static const uint8_t HLW8112_COMMAND_WRITE_EN = 0xE5;       // Special command operations
static const uint8_t HLW8112_COMMAND_WRITE_PROTECT = 0xDC;  // Special command operations
static const uint8_t HLW8112_COMMAND_SELECT_CH_A = 0x5A;    // Special command operations
static const uint8_t HLW8112_COMMAND_SELECT_CH_B = 0xA5;    // Special command operations
static const uint8_t HLW8112_COMMAND_RESET = 0x96;          // Special command operations

/**
 * @brief  Register bits of SYSCON register
 */
static const uint8_t HLW8112_REG_SYSCON_ADC3ON = 11;
static const uint8_t HLW8112_REG_SYSCON_ADC2ON = 10;
static const uint8_t HLW8112_REG_SYSCON_ADC1ON = 9;
static const uint8_t HLW8112_REG_SYSCON_PGAIB = 6;
static const uint8_t HLW8112_REG_SYSCON_PGAU = 3;
static const uint8_t HLW8112_REG_SYSCON_PGAIA = 0;

/**
 * @brief  Register bits of EMUCON register
 */
static const uint8_t HLW8112_REG_EMUCON_Tsensor_Step = 14;
static const uint8_t HLW8112_REG_EMUCON_tensor_en = 13;
static const uint8_t HLW8112_REG_EMUCON_comp_off = 12;
static const uint8_t HLW8112_REG_EMUCON_Pmode = 10;
static const uint8_t HLW8112_REG_EMUCON_DC_MODE = 9;
static const uint8_t HLW8112_REG_EMUCON_ZXD1 = 8;
static const uint8_t HLW8112_REG_EMUCON_ZXD0 = 7;
static const uint8_t HLW8112_REG_EMUCON_HPFIBOFF = 6;
static const uint8_t HLW8112_REG_EMUCON_HPFIAOFF = 5;
static const uint8_t HLW8112_REG_EMUCON_HPFUOFF = 4;
static const uint8_t HLW8112_REG_EMUCON_PBRUN = 1;
static const uint8_t HLW8112_REG_EMUCON_PARUN = 0;

/**
 * @brief  Register bits of EMUCON2 register
 */
static const uint8_t HLW8112_REG_EMUCON2_SDOCmos = 12;
static const uint8_t HLW8112_REG_EMUCON2_EPB_CB = 11;
static const uint8_t HLW8112_REG_EMUCON2_EPB_CA = 10;
static const uint8_t HLW8112_REG_EMUCON2_DUPSEL = 8;
static const uint8_t HLW8112_REG_EMUCON2_CHS_IB = 7;
static const uint8_t HLW8112_REG_EMUCON2_PfactorEN = 6;
static const uint8_t HLW8112_REG_EMUCON2_WaveEN = 5;
static const uint8_t HLW8112_REG_EMUCON2_SAGEN = 4;
static const uint8_t HLW8112_REG_EMUCON2_OverEN = 3;
static const uint8_t HLW8112_REG_EMUCON2_ZxEN = 2;
static const uint8_t HLW8112_REG_EMUCON2_PeakEN = 1;
static const uint8_t HLW8112_REG_EMUCON2_VrefSel = 0;

/**
 * @brief  Register bits of INT register
 */
static const uint8_t HLW8112_REG_INT_P2sel = 4;
static const uint8_t HLW8112_REG_INT_P1sel = 0;

/**
 * @brief  Register bits of EMUStatus register
 */
static const uint8_t HLW8112_REG_EMUStatus_Channel_sel = 21;
static const uint8_t HLW8112_REG_EMUStatus_NopldB = 20;
static const uint8_t HLW8112_REG_EMUStatus_NopldA = 19;
static const uint8_t HLW8112_REG_EMUStatus_REVPB = 18;
static const uint8_t HLW8112_REG_EMUStatus_REVPA = 17;
static const uint8_t HLW8112_REG_EMUStatus_ChksumBusy = 16;
static const uint8_t HLW8112_REG_EMUStatus_Chksum = 0;

/**
 * @brief  Register bits of IE register
 */
static const uint8_t HLW8112_REG_IE_LeakageIE = 15;
static const uint8_t HLW8112_REG_IE_ZX_UIE = 14;
static const uint8_t HLW8112_REG_IE_ZX_IBIE = 13;
static const uint8_t HLW8112_REG_IE_ZX_IAIE = 12;
static const uint8_t HLW8112_REG_IE_SAGIE = 11;
static const uint8_t HLW8112_REG_IE_OPIE = 10;
static const uint8_t HLW8112_REG_IE_OVIE = 9;
static const uint8_t HLW8112_REG_IE_OIBIE = 8;
static const uint8_t HLW8112_REG_IE_OIAIE = 7;
static const uint8_t HLW8112_REG_IE_INSTANIE = 6;
static const uint8_t HLW8112_REG_IE_PEBOIE = 4;
static const uint8_t HLW8112_REG_IE_PEAOIE = 3;
static const uint8_t HLW8112_REG_IE_PFBIE = 2;
static const uint8_t HLW8112_REG_IE_PFAIE = 1;
static const uint8_t HLW8112_REG_IE_DUPDIE = 0;

/**
 * @brief  Register bits of IF register
 */
static const uint8_t HLW8112_REG_IF_LeakageIF = 15;
static const uint8_t HLW8112_REG_IF_ZX_UIF = 14;
static const uint8_t HLW8112_REG_IF_ZX_IBIF = 13;
static const uint8_t HLW8112_REG_IF_ZX_IAIF = 12;
static const uint8_t HLW8112_REG_IF_SAGIF = 11;
static const uint8_t HLW8112_REG_IF_OPIF = 10;
static const uint8_t HLW8112_REG_IF_OVIF = 9;
static const uint8_t HLW8112_REG_IF_OIBIF = 8;
static const uint8_t HLW8112_REG_IF_OIAIF = 7;
static const uint8_t HLW8112_REG_IF_INSTANIF = 6;
static const uint8_t HLW8112_REG_IF_PEBOIF = 4;
static const uint8_t HLW8112_REG_IF_PEAOIF = 3;
static const uint8_t HLW8112_REG_IF_PFBIF = 2;
static const uint8_t HLW8112_REG_IF_PFAIF = 1;
static const uint8_t HLW8112_REG_IF_DUPDIF = 0;

/**
 * @brief  Register bits of RIF register
 */
static const uint8_t HLW8112_REG_RIF_RleakageIF = 15;
static const uint8_t HLW8112_REG_RIF_RZX_UIF = 14;
static const uint8_t HLW8112_REG_RIF_RZX_IBIF = 13;
static const uint8_t HLW8112_REG_RIF_RZX_IAIF = 12;
static const uint8_t HLW8112_REG_RIF_RSAGIF = 11;
static const uint8_t HLW8112_REG_RIF_ROPIF = 10;
static const uint8_t HLW8112_REG_RIF_ROVIF = 9;
static const uint8_t HLW8112_REG_RIF_ROIBIF = 8;
static const uint8_t HLW8112_REG_RIF_ROIAIF = 7;
static const uint8_t HLW8112_REG_RIF_RINSTANIF = 6;
static const uint8_t HLW8112_REG_RIF_RPEBOIF = 4;
static const uint8_t HLW8112_REG_RIF_RPEAOIF = 3;
static const uint8_t HLW8112_REG_RIF_RPFBIF = 2;
static const uint8_t HLW8112_REG_RIF_RPFAIF = 1;
static const uint8_t HLW8112_REG_RIF_RDUPDIF = 0;

/**
 * @brief  Register bits of SysStatus register
 */
static const uint8_t HLW8112_REG_SysStatus_clksel = 6;
static const uint8_t HLW8112_REG_SysStatus_WREN = 5;
static const uint8_t HLW8112_REG_SysStatus_RST = 0;

}  // namespace hlw8112
}  // namespace esphome
