/*
 * Copyright (c) 2023 Espressif Systems (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_INCLUDE_DT_BINDINGS_PINCTRL_ESP32C6_GPIO_SIGMAP_H_
#define ZEPHYR_INCLUDE_DT_BINDINGS_PINCTRL_ESP32C6_GPIO_SIGMAP_H_

#define ESP_NOSIG                       ESP_SIG_INVAL

#define ESP_EXT_ADC_START             0
#define ESP_LEDC_LS_SIG_OUT0          0
#define ESP_LEDC_LS_SIG_OUT1          1
#define ESP_LEDC_LS_SIG_OUT2          2
#define ESP_LEDC_LS_SIG_OUT3          3
#define ESP_LEDC_LS_SIG_OUT4          4
#define ESP_LEDC_LS_SIG_OUT5          5
#define ESP_U0RXD_IN                  6
#define ESP_U0TXD_OUT                 6
#define ESP_U0CTS_IN                  7
#define ESP_U0RTS_OUT                 7
#define ESP_U0DSR_IN                  8
#define ESP_U0DTR_OUT                 8
#define ESP_U1RXD_IN                  9
#define ESP_U1TXD_OUT                 9
#define ESP_U1CTS_IN                  10
#define ESP_U1RTS_OUT                 10
#define ESP_U1DSR_IN                  11
#define ESP_U1DTR_OUT                 11
#define ESP_I2S_MCLK_IN               12
#define ESP_I2S_MCLK_OUT              12
#define ESP_I2SO_BCK_IN               13
#define ESP_I2SO_BCK_OUT              13
#define ESP_I2SO_WS_IN                14
#define ESP_I2SO_WS_OUT               14
#define ESP_I2SI_SD_IN                15
#define ESP_I2SO_SD_OUT               15
#define ESP_I2SI_BCK_IN               16
#define ESP_I2SI_BCK_OUT              16
#define ESP_I2SI_WS_IN                17
#define ESP_I2SI_WS_OUT               17
#define ESP_I2SO_SD1_OUT              18
#define ESP_USB_JTAG_TDO_BRIDGE       19
#define ESP_USB_JTAG_TRST             19
#define ESP_CPU_TESTBUS0              20
#define ESP_CPU_TESTBUS1              21
#define ESP_CPU_TESTBUS2              22
#define ESP_CPU_TESTBUS3              23
#define ESP_CPU_TESTBUS4              24
#define ESP_CPU_TESTBUS5              25
#define ESP_CPU_TESTBUS6              26
#define ESP_CPU_TESTBUS7              27
#define ESP_CPU_GPIO_IN0              28
#define ESP_CPU_GPIO_OUT0             28
#define ESP_CPU_GPIO_IN1              29
#define ESP_CPU_GPIO_OUT1             29
#define ESP_CPU_GPIO_IN2              30
#define ESP_CPU_GPIO_OUT2             30
#define ESP_CPU_GPIO_IN3              31
#define ESP_CPU_GPIO_OUT3             31
#define ESP_CPU_GPIO_IN4              32
#define ESP_CPU_GPIO_OUT4             32
#define ESP_CPU_GPIO_IN5              33
#define ESP_CPU_GPIO_OUT5             33
#define ESP_CPU_GPIO_IN6              34
#define ESP_CPU_GPIO_OUT6             34
#define ESP_CPU_GPIO_IN7              35
#define ESP_CPU_GPIO_OUT7             35
#define ESP_USB_JTAG_TCK              36
#define ESP_USB_JTAG_TMS              37
#define ESP_USB_JTAG_TDI              38
#define ESP_USB_JTAG_TDO              39
#define ESP_USB_EXTPHY_VP             40
#define ESP_USB_EXTPHY_OEN            40
#define ESP_USB_EXTPHY_VM             41
#define ESP_USB_EXTPHY_SPEED          41
#define ESP_USB_EXTPHY_RCV            42
#define ESP_USB_EXTPHY_VPO            42
#define ESP_USB_EXTPHY_VMO            43
#define ESP_USB_EXTPHY_SUSPND         44
#define ESP_I2CEXT0_SCL_IN            45
#define ESP_I2CEXT0_SCL_OUT           45
#define ESP_I2CEXT0_SDA_IN            46
#define ESP_I2CEXT0_SDA_OUT           46
#define ESP_PARL_RX_DATA0             47
#define ESP_PARL_TX_DATA0             47
#define ESP_PARL_RX_DATA1             48
#define ESP_PARL_TX_DATA1             48
#define ESP_PARL_RX_DATA2             49
#define ESP_PARL_TX_DATA2             49
#define ESP_PARL_RX_DATA3             50
#define ESP_PARL_TX_DATA3             50
#define ESP_PARL_RX_DATA4             51
#define ESP_PARL_TX_DATA4             51
#define ESP_PARL_RX_DATA5             52
#define ESP_PARL_TX_DATA5             52
#define ESP_PARL_RX_DATA6             53
#define ESP_PARL_TX_DATA6             53
#define ESP_PARL_RX_DATA7             54
#define ESP_PARL_TX_DATA7             54
#define ESP_PARL_RX_DATA8             55
#define ESP_PARL_TX_DATA8             55
#define ESP_PARL_RX_DATA9             56
#define ESP_PARL_TX_DATA9             56
#define ESP_PARL_RX_DATA10            57
#define ESP_PARL_TX_DATA10            57
#define ESP_PARL_RX_DATA11            58
#define ESP_PARL_TX_DATA11            58
#define ESP_PARL_RX_DATA12            59
#define ESP_PARL_TX_DATA12            59
#define ESP_PARL_RX_DATA13            60
#define ESP_PARL_TX_DATA13            60
#define ESP_PARL_RX_DATA14            61
#define ESP_PARL_TX_DATA14            61
#define ESP_PARL_RX_DATA15            62
#define ESP_PARL_TX_DATA15            62
#define ESP_FSPICLK_IN                63
#define ESP_FSPICLK_OUT               63
#define ESP_FSPIQ_IN                  64
#define ESP_FSPIQ_OUT                 64
#define ESP_FSPID_IN                  65
#define ESP_FSPID_OUT                 65
#define ESP_FSPIHD_IN                 66
#define ESP_FSPIHD_OUT                66
#define ESP_FSPIWP_IN                 67
#define ESP_FSPIWP_OUT                67
#define ESP_FSPICS0_IN                68
#define ESP_FSPICS0_OUT               68
#define ESP_PARL_RX_CLK_IN            69
#define ESP_SDIO_TOHOST_INT_OUT       69
#define ESP_PARL_TX_CLK_IN            70
#define ESP_PARL_TX_CLK_OUT           70
#define ESP_RMT_SIG_IN0               71
#define ESP_RMT_SIG_OUT0              71
#define ESP_MODEM_DIAG0               71
#define ESP_RMT_SIG_IN1               72
#define ESP_RMT_SIG_OUT1              72
#define ESP_MODEM_DIAG1               72
#define ESP_TWAI0_RX                  73
#define ESP_TWAI0_TX                  73
#define ESP_MODEM_DIAG2               73
#define ESP_TWAI0_BUS_OFF_ON          74
#define ESP_MODEM_DIAG3               74
#define ESP_TWAI0_CLKOUT              75
#define ESP_MODEM_DIAG4               75
#define ESP_TWAI0_STANDBY             76
#define ESP_MODEM_DIAG5               76
#define ESP_TWAI1_RX                  77
#define ESP_TWAI1_TX                  77
#define ESP_MODEM_DIAG6               77
#define ESP_TWAI1_BUS_OFF_ON          78
#define ESP_MODEM_DIAG7               78
#define ESP_TWAI1_CLKOUT              79
#define ESP_MODEM_DIAG8               79
#define ESP_TWAI1_STANDBY             80
#define ESP_MODEM_DIAG9               80
#define ESP_EXTERN_PRIORITY_I         81
#define ESP_EXTERN_PRIORITY_O         81
#define ESP_EXTERN_ACTIVE_I           82
#define ESP_EXTERN_ACTIVE_O           82
#define ESP_GPIO_SD0_OUT              83
#define ESP_GPIO_SD1_OUT              84
#define ESP_GPIO_SD2_OUT              85
#define ESP_GPIO_SD3_OUT              86
#define ESP_PWM0_SYNC0_IN             87
#define ESP_PWM0_OUT0A                87
#define ESP_MODEM_DIAG10              87
#define ESP_PWM0_SYNC1_IN             88
#define ESP_PWM0_OUT0B                88
#define ESP_MODEM_DIAG11              88
#define ESP_PWM0_SYNC2_IN             89
#define ESP_PWM0_OUT1A                89
#define ESP_MODEM_DIAG12              89
#define ESP_PWM0_F0_IN                90
#define ESP_PWM0_OUT1B                90
#define ESP_MODEM_DIAG13              90
#define ESP_PWM0_F1_IN                91
#define ESP_PWM0_OUT2A                91
#define ESP_MODEM_DIAG14              91
#define ESP_PWM0_F2_IN                92
#define ESP_PWM0_OUT2B                92
#define ESP_MODEM_DIAG15              92
#define ESP_PWM0_CAP0_IN              93
#define ESP_ANT_SEL0                  93
#define ESP_PWM0_CAP1_IN              94
#define ESP_ANT_SEL1                  94
#define ESP_PWM0_CAP2_IN              95
#define ESP_ANT_SEL2                  95
#define ESP_ANT_SEL3                  96
#define ESP_SIG_IN_FUNC_97            97
#define ESP_SIG_IN_FUNC97             97
#define ESP_SIG_IN_FUNC_98            98
#define ESP_SIG_IN_FUNC98             98
#define ESP_SIG_IN_FUNC_99            99
#define ESP_SIG_IN_FUNC99             99
#define ESP_SIG_IN_FUNC_100           100
#define ESP_SIG_IN_FUNC100            100
#define ESP_PCNT_SIG_CH0_IN0          101
#define ESP_FSPICS1_OUT               101
#define ESP_MODEM_DIAG16              101
#define ESP_PCNT_SIG_CH1_IN0          102
#define ESP_FSPICS2_OUT               102
#define ESP_MODEM_DIAG17              102
#define ESP_PCNT_CTRL_CH0_IN0         103
#define ESP_FSPICS3_OUT               103
#define ESP_MODEM_DIAG18              103
#define ESP_PCNT_CTRL_CH1_IN0         104
#define ESP_FSPICS4_OUT               104
#define ESP_MODEM_DIAG19              104
#define ESP_PCNT_SIG_CH0_IN1          105
#define ESP_FSPICS5_OUT               105
#define ESP_MODEM_DIAG20              105
#define ESP_PCNT_SIG_CH1_IN1          106
#define ESP_MODEM_DIAG21              106
#define ESP_PCNT_CTRL_CH0_IN1         107
#define ESP_MODEM_DIAG22              107
#define ESP_PCNT_CTRL_CH1_IN1         108
#define ESP_MODEM_DIAG23              108
#define ESP_PCNT_SIG_CH0_IN2          109
#define ESP_MODEM_DIAG24              109
#define ESP_PCNT_SIG_CH1_IN2          110
#define ESP_MODEM_DIAG25              110
#define ESP_PCNT_CTRL_CH0_IN2         111
#define ESP_MODEM_DIAG26              111
#define ESP_PCNT_CTRL_CH1_IN2         112
#define ESP_MODEM_DIAG27              112
#define ESP_PCNT_SIG_CH0_IN3          113
#define ESP_MODEM_DIAG28              113
#define ESP_PCNT_SIG_CH1_IN3          114
#define ESP_SPICLK_OUT                114
#define ESP_MODEM_DIAG29              114
#define ESP_PCNT_CTRL_CH0_IN3         115
#define ESP_SPICS0_OUT                115
#define ESP_MODEM_DIAG30              115
#define ESP_PCNT_CTRL_CH1_IN3         116
#define ESP_SPICS1_OUT                116
#define ESP_MODEM_DIAG31              116
#define ESP_GPIO_EVENT_MATRIX_IN0     117
#define ESP_GPIO_TASK_MATRIX_OUT0     117
#define ESP_GPIO_EVENT_MATRIX_IN1     118
#define ESP_GPIO_TASK_MATRIX_OUT1     118
#define ESP_GPIO_EVENT_MATRIX_IN2     119
#define ESP_GPIO_TASK_MATRIX_OUT2     119
#define ESP_GPIO_EVENT_MATRIX_IN3     120
#define ESP_GPIO_TASK_MATRIX_OUT3     120
#define ESP_SPIQ_IN                   121
#define ESP_SPIQ_OUT                  121
#define ESP_SPID_IN                   122
#define ESP_SPID_OUT                  122
#define ESP_SPIHD_IN                  123
#define ESP_SPIHD_OUT                 123
#define ESP_SPIWP_IN                  124
#define ESP_SPIWP_OUT                 124
#define ESP_CLK_OUT_OUT1              125
#define ESP_CLK_OUT_OUT2              126
#define ESP_CLK_OUT_OUT3              127
#define ESP_SIG_GPIO_OUT              128
#define ESP_GPIO_MAP_DATE             0x2201120

#endif  /* ZEPHYR_INCLUDE_DT_BINDINGS_PINCTRL_ESP32C6_GPIO_SIGMAP_H_ */
