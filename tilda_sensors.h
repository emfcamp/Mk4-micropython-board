#ifndef TILDA_SENSORS_H_INC
#define TILDA_SENSORS_H_INC

extern const mp_obj_type_t tilda_sensors_type;

/****************************************************/
/**\name    REGISTER ADDRESS DEFINITIONS (HDC2080)  */
/****************************************************/
#define HDC2080_TEMPERATURE_LSB_REG              (0x00U)  /*Temperature MSB Reg */
#define HDC2080_TEMPERATURE_MSB_REG              (0x01U)  /*Temperature LSB Reg */
#define HDC2080_HUMIDITY_LSB_REG                 (0x02U)  /*Humidity MSB Reg */
#define HDC2080_HUMIDITY_MSB_REG                 (0x03U)  /*Humidity LSB Reg */

#define HDC2080_DRDY_INT_STATUS_REG              (0x04U)  /*DataReady and Interrupt Status */
#define HDC2080_MAX_TEMPERATURE_REG              (0x05U)  /*Max Temperature Value measured(Peak Detector) */
#define HDC2080_MAX_HUMIDITY_REG                 (0x06U)  /*Max Humidity Value measured(Peak Detector) */
#define HDC2080_INT_MASK_REG                     (0x07U)  /*Interrupt Mask Register */

#define HDC2080_TEMPERATURE_OFF_REG              (0x08U)  /*Temperature Offset Adjustment*/
#define HDC2080_HUMIDITY_OFF_REG                 (0x09U)  /*Humidity Offset Adjustment*/

#define HDC2080_TEMPERATURE_THR_L_REG            (0x0AU)  /*Temperature Threshold Low*/
#define HDC2080_TEMPERATURE_THR_H_REG            (0x0BU)  /*Temperature Threshold High */
#define HDC2080_HUMIDITY_THR_L_REG               (0x0CU)  /*Humidity Threshold Low */
#define HDC2080_HUMIDITY_THR_H_REG               (0x0DU)  /*Humidity Threshold High */

#define HDC2080_RST_DRDY_INT_CONF_REG            (0x0EU)  /*Soft Reset and Interrupt Configuration */
#define HDC2080_MEAS_CONFIG_REG                  (0x0FU)  /*Measurement Configuration Register */

#define HDC2080_MANUFACTURER_ID_L_REG            (0xFCU)  /*Manufacturer ID LSB Register */
#define HDC2080_MANUFACTURER_ID_H_REG            (0xFDU)  /*Manufacturer ID MSB Registerr */
#define HDC2080_DEV_ID_L_REG                     (0xFEU)  /*Device ID LSB Register */
#define HDC2080_DEV_ID_H_REG                     (0xFFU)  /*Device ID MSB Register */

// Soft Reset and Interrupt Configuration bit definitions
#define HDC2080_RST_DRDY_INT_CONF_SOFT_RES      (1<<7)
#define HDC2080_RST_DRDY_INT_CONF_AMM           (1<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_1_120     (1<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_1_60      (2<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_1_10      (3<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_1_5       (4<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_1         (5<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_2         (6<<4)
#define HDC2080_RST_DRDY_INT_CONF_AMM_5         (7<<4)
#define HDC2080_RST_DRDY_INT_CONF_DRDY_EN       (1<<2)
#define HDC2080_RST_DRDY_INT_CONF_INT_POL       (1<<1)
#define HDC2080_MEAS_CONFIG_START_MEAS          (1<<0)

#define RH_PER_LSB           0.0015259F /* Relative Humidity celsius per LSB  */
#define CELSIUS_PER_LSB      0.0025177F /* Degrees celsius per LSB            */

/****************************************************/
/**\name    REGISTER ADDRESS DEFINITIONS (TMP102)   */
/****************************************************/

#define TMP_TEMPERATURE_REG 0
#define TMP_CONFIG_REG 1

#define TMP_CFG_SD      (1<<0)
#define TMP_CFG_TM      (1<<1)
#define TMP_CFG_POL     (1<<2)
#define TMP_CFG_F0      (1<<3)
#define TMP_CFG_F1      (1<<4)
#define TMP_CFG_R0      (1<<5)
#define TMP_CFG_R1      (1<<6)
#define TMP_CFG_OS      (1<<7)

#define TMP_CFG_EM      (1<<4)  //set high for 13bit range
#define TMP_CFG_AL      (1<<5)
#define TMP_CFG_CR0     (1<<6)
#define TMP_CFG_CR1     (1<<7)
#define TMP_CFG_CR_025Hz    (0)
#define TMP_CFG_CR_1Hz      (TMP_CFG_CR0)
#define TMP_CFG_CR_4Hz      (TMP_CFG_CR1)
#define TMP_CFG_CR_8Hz      (TMP_CFG_CR0 | TMP_CFG_CR1)


#endif
