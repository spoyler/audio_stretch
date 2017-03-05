
Linux Cross Reference
Free Electrons
Embedded Linux Experts

 • source navigation  • diff markup  • identifier search  • freetext search  • 

Version:  2.0.40 2.2.26 2.4.37 3.11 3.12 3.13 3.14 3.15 3.16 3.17 3.18 3.19 4.0 4.1 4.2 4.3 4.4 4.5 4.6 4.7 4.8
Linux/sound/soc/codecs/max98088.h

  1 /*
  2  * max98088.h -- MAX98088 ALSA SoC Audio driver
  3  *
  4  * Copyright 2010 Maxim Integrated Products
  5  *
  6  * This program is free software; you can redistribute it and/or modify
  7  * it under the terms of the GNU General Public License version 2 as
  8  * published by the Free Software Foundation.
  9  */
 10 
 11 #ifndef _MAX98088_H
 12 #define _MAX98088_H
 13 
 14 /*
 15  * MAX98088 Registers Definition
 16  */
 17 #define M98088_REG_00_IRQ_STATUS            0x00
 18 #define M98088_REG_01_MIC_STATUS            0x01
 19 #define M98088_REG_02_JACK_STATUS           0x02
 20 #define M98088_REG_03_BATTERY_VOLTAGE       0x03
 21 #define M98088_REG_0F_IRQ_ENABLE            0x0F
 22 #define M98088_REG_10_SYS_CLK               0x10
 23 #define M98088_REG_11_DAI1_CLKMODE          0x11
 24 #define M98088_REG_12_DAI1_CLKCFG_HI        0x12
 25 #define M98088_REG_13_DAI1_CLKCFG_LO        0x13
 26 #define M98088_REG_14_DAI1_FORMAT           0x14
 27 #define M98088_REG_15_DAI1_CLOCK            0x15
 28 #define M98088_REG_16_DAI1_IOCFG            0x16
 29 #define M98088_REG_17_DAI1_TDM              0x17
 30 #define M98088_REG_18_DAI1_FILTERS          0x18
 31 #define M98088_REG_19_DAI2_CLKMODE          0x19
 32 #define M98088_REG_1A_DAI2_CLKCFG_HI        0x1A
 33 #define M98088_REG_1B_DAI2_CLKCFG_LO        0x1B
 34 #define M98088_REG_1C_DAI2_FORMAT           0x1C
 35 #define M98088_REG_1D_DAI2_CLOCK            0x1D
 36 #define M98088_REG_1E_DAI2_IOCFG            0x1E
 37 #define M98088_REG_1F_DAI2_TDM              0x1F
 38 #define M98088_REG_20_DAI2_FILTERS          0x20
 39 #define M98088_REG_21_SRC                   0x21
 40 #define M98088_REG_22_MIX_DAC               0x22
 41 #define M98088_REG_23_MIX_ADC_LEFT          0x23
 42 #define M98088_REG_24_MIX_ADC_RIGHT         0x24
 43 #define M98088_REG_25_MIX_HP_LEFT           0x25
 44 #define M98088_REG_26_MIX_HP_RIGHT          0x26
 45 #define M98088_REG_27_MIX_HP_CNTL           0x27
 46 #define M98088_REG_28_MIX_REC_LEFT          0x28
 47 #define M98088_REG_29_MIX_REC_RIGHT         0x29
 48 #define M98088_REG_2A_MIC_REC_CNTL          0x2A
 49 #define M98088_REG_2B_MIX_SPK_LEFT          0x2B
 50 #define M98088_REG_2C_MIX_SPK_RIGHT         0x2C
 51 #define M98088_REG_2D_MIX_SPK_CNTL          0x2D
 52 #define M98088_REG_2E_LVL_SIDETONE          0x2E
 53 #define M98088_REG_2F_LVL_DAI1_PLAY         0x2F
 54 #define M98088_REG_30_LVL_DAI1_PLAY_EQ      0x30
 55 #define M98088_REG_31_LVL_DAI2_PLAY         0x31
 56 #define M98088_REG_32_LVL_DAI2_PLAY_EQ      0x32
 57 #define M98088_REG_33_LVL_ADC_L             0x33
 58 #define M98088_REG_34_LVL_ADC_R             0x34
 59 #define M98088_REG_35_LVL_MIC1              0x35
 60 #define M98088_REG_36_LVL_MIC2              0x36
 61 #define M98088_REG_37_LVL_INA               0x37
 62 #define M98088_REG_38_LVL_INB               0x38
 63 #define M98088_REG_39_LVL_HP_L              0x39
 64 #define M98088_REG_3A_LVL_HP_R              0x3A
 65 #define M98088_REG_3B_LVL_REC_L             0x3B
 66 #define M98088_REG_3C_LVL_REC_R             0x3C
 67 #define M98088_REG_3D_LVL_SPK_L             0x3D
 68 #define M98088_REG_3E_LVL_SPK_R             0x3E
 69 #define M98088_REG_3F_MICAGC_CFG            0x3F
 70 #define M98088_REG_40_MICAGC_THRESH         0x40
 71 #define M98088_REG_41_SPKDHP                0x41
 72 #define M98088_REG_42_SPKDHP_THRESH         0x42
 73 #define M98088_REG_43_SPKALC_COMP           0x43
 74 #define M98088_REG_44_PWRLMT_CFG            0x44
 75 #define M98088_REG_45_PWRLMT_TIME           0x45
 76 #define M98088_REG_46_THDLMT_CFG            0x46
 77 #define M98088_REG_47_CFG_AUDIO_IN          0x47
 78 #define M98088_REG_48_CFG_MIC               0x48
 79 #define M98088_REG_49_CFG_LEVEL             0x49
 80 #define M98088_REG_4A_CFG_BYPASS            0x4A
 81 #define M98088_REG_4B_CFG_JACKDET           0x4B
 82 #define M98088_REG_4C_PWR_EN_IN             0x4C
 83 #define M98088_REG_4D_PWR_EN_OUT            0x4D
 84 #define M98088_REG_4E_BIAS_CNTL             0x4E
 85 #define M98088_REG_4F_DAC_BIAS1             0x4F
 86 #define M98088_REG_50_DAC_BIAS2             0x50
 87 #define M98088_REG_51_PWR_SYS               0x51
 88 #define M98088_REG_52_DAI1_EQ_BASE          0x52
 89 #define M98088_REG_84_DAI2_EQ_BASE          0x84
 90 #define M98088_REG_B6_DAI1_BIQUAD_BASE      0xB6
 91 #define M98088_REG_C0_DAI2_BIQUAD_BASE      0xC0
 92 #define M98088_REG_FF_REV_ID                0xFF
 93 
 94 #define M98088_REG_CNT                      (0xFF+1)
 95 
 96 /* MAX98088 Registers Bit Fields */
 97 
 98 /* M98088_REG_11_DAI1_CLKMODE, M98088_REG_19_DAI2_CLKMODE */
 99        #define M98088_CLKMODE_MASK             0xFF
100 
101 /* M98088_REG_14_DAI1_FORMAT, M98088_REG_1C_DAI2_FORMAT */
102        #define M98088_DAI_MAS                  (1<<7)
103        #define M98088_DAI_WCI                  (1<<6)
104        #define M98088_DAI_BCI                  (1<<5)
105        #define M98088_DAI_DLY                  (1<<4)
106        #define M98088_DAI_TDM                  (1<<2)
107        #define M98088_DAI_FSW                  (1<<1)
108        #define M98088_DAI_WS                   (1<<0)
109 
110 /* M98088_REG_15_DAI1_CLOCK, M98088_REG_1D_DAI2_CLOCK */
111        #define M98088_DAI_BSEL64               (1<<0)
112        #define M98088_DAI_OSR64                (1<<6)
113 
114 /* M98088_REG_16_DAI1_IOCFG, M98088_REG_1E_DAI2_IOCFG */
115        #define M98088_S1NORMAL                 (1<<6)
116        #define M98088_S2NORMAL                 (2<<6)
117        #define M98088_SDATA                    (3<<0)
118 
119 /* M98088_REG_18_DAI1_FILTERS, M98088_REG_20_DAI2_FILTERS */
120        #define M98088_DAI_DHF                  (1<<3)
121 
122 /* M98088_REG_22_MIX_DAC */
123        #define M98088_DAI1L_TO_DACL            (1<<7)
124        #define M98088_DAI1R_TO_DACL            (1<<6)
125        #define M98088_DAI2L_TO_DACL            (1<<5)
126        #define M98088_DAI2R_TO_DACL            (1<<4)
127        #define M98088_DAI1L_TO_DACR            (1<<3)
128        #define M98088_DAI1R_TO_DACR            (1<<2)
129        #define M98088_DAI2L_TO_DACR            (1<<1)
130        #define M98088_DAI2R_TO_DACR            (1<<0)
131 
132 /* M98088_REG_2A_MIC_REC_CNTL */
133        #define M98088_REC_LINEMODE             (1<<7)
134        #define M98088_REC_LINEMODE_MASK        (1<<7)
135 
136 /* M98088_REG_2D_MIX_SPK_CNTL */
137        #define M98088_MIX_SPKR_GAIN_MASK       (3<<2)
138        #define M98088_MIX_SPKR_GAIN_SHIFT      2
139        #define M98088_MIX_SPKL_GAIN_MASK       (3<<0)
140        #define M98088_MIX_SPKL_GAIN_SHIFT      0
141 
142 /* M98088_REG_2F_LVL_DAI1_PLAY, M98088_REG_31_LVL_DAI2_PLAY */
143        #define M98088_DAI_MUTE                 (1<<7)
144        #define M98088_DAI_MUTE_MASK            (1<<7)
145        #define M98088_DAI_VOICE_GAIN_MASK      (3<<4)
146        #define M98088_DAI_ATTENUATION_MASK     (0xF<<0)
147        #define M98088_DAI_ATTENUATION_SHIFT    0
148 
149 /* M98088_REG_35_LVL_MIC1, M98088_REG_36_LVL_MIC2 */
150        #define M98088_MICPRE_MASK              (3<<5)
151        #define M98088_MICPRE_SHIFT             5
152 
153 /* M98088_REG_3A_LVL_HP_R */
154        #define M98088_HP_MUTE                  (1<<7)
155 
156 /* M98088_REG_3C_LVL_REC_R */
157        #define M98088_REC_MUTE                 (1<<7)
158 
159 /* M98088_REG_3E_LVL_SPK_R */
160        #define M98088_SP_MUTE                  (1<<7)
161 
162 /* M98088_REG_48_CFG_MIC */
163        #define M98088_EXTMIC_MASK              (3<<0)
164        #define M98088_DIGMIC_L                 (1<<5)
165        #define M98088_DIGMIC_R                 (1<<4)
166 
167 /* M98088_REG_49_CFG_LEVEL */
168        #define M98088_VSEN                     (1<<6)
169        #define M98088_ZDEN                     (1<<5)
170        #define M98088_EQ2EN                    (1<<1)
171        #define M98088_EQ1EN                    (1<<0)
172 
173 /* M98088_REG_4C_PWR_EN_IN */
174        #define M98088_INAEN                    (1<<7)
175        #define M98088_INBEN                    (1<<6)
176        #define M98088_MBEN                     (1<<3)
177        #define M98088_ADLEN                    (1<<1)
178        #define M98088_ADREN                    (1<<0)
179 
180 /* M98088_REG_4D_PWR_EN_OUT */
181        #define M98088_HPLEN                    (1<<7)
182        #define M98088_HPREN                    (1<<6)
183        #define M98088_HPEN                     ((1<<7)|(1<<6))
184        #define M98088_SPLEN                    (1<<5)
185        #define M98088_SPREN                    (1<<4)
186        #define M98088_RECEN                    (1<<3)
187        #define M98088_DALEN                    (1<<1)
188        #define M98088_DAREN                    (1<<0)
189 
190 /* M98088_REG_51_PWR_SYS */
191        #define M98088_SHDNRUN                  (1<<7)
192        #define M98088_PERFMODE                 (1<<3)
193        #define M98088_HPPLYBACK                (1<<2)
194        #define M98088_PWRSV8K                  (1<<1)
195        #define M98088_PWRSV                    (1<<0)
196 
197 /* Line inputs */
198 #define LINE_INA  0
199 #define LINE_INB  1
200 
201 #define M98088_COEFS_PER_BAND               5
202 
203 #define M98088_BYTE1(w) ((w >> 8) & 0xff)
204 #define M98088_BYTE0(w) (w & 0xff)
205 
206 #endif
207 

This page was automatically generated by LXR 0.3.1 (source).  •  Linux is a registered trademark of Linus Torvalds  •  Contact us

    Home
    Development
    Services
    Training
    Docs
    Community
    Company
    Blog
