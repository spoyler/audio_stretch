
Linux Cross Reference
Free Electrons
Embedded Linux Experts

 • source navigation  • diff markup  • identifier search  • freetext search  •

Version:  2.0.40 2.2.26 2.4.37 3.11 3.12 3.13 3.14 3.15 3.16 3.17 3.18 3.19 4.0 4.1 4.2 4.3 4.4 4.5 4.6 4.7 4.8
Linux/sound/soc/codecs/max98088.c

  1 /*
  2  * max98088.c -- MAX98088 ALSA SoC Audio driver
  3  *
  4  * Copyright 2010 Maxim Integrated Products
  5  *
  6  * This program is free software; you can redistribute it and/or modify
  7  * it under the terms of the GNU General Public License version 2 as
  8  * published by the Free Software Foundation.
  9  */
 10
 11 #include <linux/module.h>
 12 #include <linux/moduleparam.h>
 13 #include <linux/kernel.h>
 14 #include <linux/init.h>
 15 #include <linux/delay.h>
 16 #include <linux/pm.h>
 17 #include <linux/i2c.h>
 18 #include <linux/regmap.h>
 19 #include <sound/core.h>
 20 #include <sound/pcm.h>
 21 #include <sound/pcm_params.h>
 22 #include <sound/soc.h>
 23 #include <sound/initval.h>
 24 #include <sound/tlv.h>
 25 #include <linux/slab.h>
 26 #include <asm/div64.h>
 27 #include <sound/max98088.h>
 28 #include "max98088.h"
 29
 30 enum max98088_type {
 31        MAX98088,
 32        MAX98089,
 33 };
 34
 35 struct max98088_cdata {
 36        unsigned int rate;
 37        unsigned int fmt;
 38        int eq_sel;
 39 };
 40
 41 struct max98088_priv {
 42         struct regmap *regmap;
 43         enum max98088_type devtype;
 44         struct max98088_pdata *pdata;
 45         unsigned int sysclk;
 46         struct max98088_cdata dai[2];
 47         int eq_textcnt;
 48         const char **eq_texts;
 49         struct soc_enum eq_enum;
 50         u8 ina_state;
 51         u8 inb_state;
 52         unsigned int ex_mode;
 53         unsigned int digmic;
 54         unsigned int mic1pre;
 55         unsigned int mic2pre;
 56         unsigned int extmic_mode;
 57 };
 58
 59 static const struct reg_default max98088_reg[] = {
 60         {  0xf, 0x00 }, /* 0F interrupt enable */
 61
 62         { 0x10, 0x00 }, /* 10 master clock */
 63         { 0x11, 0x00 }, /* 11 DAI1 clock mode */
 64         { 0x12, 0x00 }, /* 12 DAI1 clock control */
 65         { 0x13, 0x00 }, /* 13 DAI1 clock control */
 66         { 0x14, 0x00 }, /* 14 DAI1 format */
 67         { 0x15, 0x00 }, /* 15 DAI1 clock */
 68         { 0x16, 0x00 }, /* 16 DAI1 config */
 69         { 0x17, 0x00 }, /* 17 DAI1 TDM */
 70         { 0x18, 0x00 }, /* 18 DAI1 filters */
 71         { 0x19, 0x00 }, /* 19 DAI2 clock mode */
 72         { 0x1a, 0x00 }, /* 1A DAI2 clock control */
 73         { 0x1b, 0x00 }, /* 1B DAI2 clock control */
 74         { 0x1c, 0x00 }, /* 1C DAI2 format */
 75         { 0x1d, 0x00 }, /* 1D DAI2 clock */
 76         { 0x1e, 0x00 }, /* 1E DAI2 config */
 77         { 0x1f, 0x00 }, /* 1F DAI2 TDM */
 78
 79         { 0x20, 0x00 }, /* 20 DAI2 filters */
 80         { 0x21, 0x00 }, /* 21 data config */
 81         { 0x22, 0x00 }, /* 22 DAC mixer */
 82         { 0x23, 0x00 }, /* 23 left ADC mixer */
 83         { 0x24, 0x00 }, /* 24 right ADC mixer */
 84         { 0x25, 0x00 }, /* 25 left HP mixer */
 85         { 0x26, 0x00 }, /* 26 right HP mixer */
 86         { 0x27, 0x00 }, /* 27 HP control */
 87         { 0x28, 0x00 }, /* 28 left REC mixer */
 88         { 0x29, 0x00 }, /* 29 right REC mixer */
 89         { 0x2a, 0x00 }, /* 2A REC control */
 90         { 0x2b, 0x00 }, /* 2B left SPK mixer */
 91         { 0x2c, 0x00 }, /* 2C right SPK mixer */
 92         { 0x2d, 0x00 }, /* 2D SPK control */
 93         { 0x2e, 0x00 }, /* 2E sidetone */
 94         { 0x2f, 0x00 }, /* 2F DAI1 playback level */
 95
 96         { 0x30, 0x00 }, /* 30 DAI1 playback level */
 97         { 0x31, 0x00 }, /* 31 DAI2 playback level */
 98         { 0x32, 0x00 }, /* 32 DAI2 playbakc level */
 99         { 0x33, 0x00 }, /* 33 left ADC level */
100         { 0x34, 0x00 }, /* 34 right ADC level */
101         { 0x35, 0x00 }, /* 35 MIC1 level */
102         { 0x36, 0x00 }, /* 36 MIC2 level */
103         { 0x37, 0x00 }, /* 37 INA level */
104         { 0x38, 0x00 }, /* 38 INB level */
105         { 0x39, 0x00 }, /* 39 left HP volume */
106         { 0x3a, 0x00 }, /* 3A right HP volume */
107         { 0x3b, 0x00 }, /* 3B left REC volume */
108         { 0x3c, 0x00 }, /* 3C right REC volume */
109         { 0x3d, 0x00 }, /* 3D left SPK volume */
110         { 0x3e, 0x00 }, /* 3E right SPK volume */
111         { 0x3f, 0x00 }, /* 3F MIC config */
112
113         { 0x40, 0x00 }, /* 40 MIC threshold */
114         { 0x41, 0x00 }, /* 41 excursion limiter filter */
115         { 0x42, 0x00 }, /* 42 excursion limiter threshold */
116         { 0x43, 0x00 }, /* 43 ALC */
117         { 0x44, 0x00 }, /* 44 power limiter threshold */
118         { 0x45, 0x00 }, /* 45 power limiter config */
119         { 0x46, 0x00 }, /* 46 distortion limiter config */
120         { 0x47, 0x00 }, /* 47 audio input */
121         { 0x48, 0x00 }, /* 48 microphone */
122         { 0x49, 0x00 }, /* 49 level control */
123         { 0x4a, 0x00 }, /* 4A bypass switches */
124         { 0x4b, 0x00 }, /* 4B jack detect */
125         { 0x4c, 0x00 }, /* 4C input enable */
126         { 0x4d, 0x00 }, /* 4D output enable */
127         { 0x4e, 0xF0 }, /* 4E bias control */
128         { 0x4f, 0x00 }, /* 4F DAC power */
129
130         { 0x50, 0x0F }, /* 50 DAC power */
131         { 0x51, 0x00 }, /* 51 system */
132         { 0x52, 0x00 }, /* 52 DAI1 EQ1 */
133         { 0x53, 0x00 }, /* 53 DAI1 EQ1 */
134         { 0x54, 0x00 }, /* 54 DAI1 EQ1 */
135         { 0x55, 0x00 }, /* 55 DAI1 EQ1 */
136         { 0x56, 0x00 }, /* 56 DAI1 EQ1 */
137         { 0x57, 0x00 }, /* 57 DAI1 EQ1 */
138         { 0x58, 0x00 }, /* 58 DAI1 EQ1 */
139         { 0x59, 0x00 }, /* 59 DAI1 EQ1 */
140         { 0x5a, 0x00 }, /* 5A DAI1 EQ1 */
141         { 0x5b, 0x00 }, /* 5B DAI1 EQ1 */
142         { 0x5c, 0x00 }, /* 5C DAI1 EQ2 */
143         { 0x5d, 0x00 }, /* 5D DAI1 EQ2 */
144         { 0x5e, 0x00 }, /* 5E DAI1 EQ2 */
145         { 0x5f, 0x00 }, /* 5F DAI1 EQ2 */
146
147         { 0x60, 0x00 }, /* 60 DAI1 EQ2 */
148         { 0x61, 0x00 }, /* 61 DAI1 EQ2 */
149         { 0x62, 0x00 }, /* 62 DAI1 EQ2 */
150         { 0x63, 0x00 }, /* 63 DAI1 EQ2 */
151         { 0x64, 0x00 }, /* 64 DAI1 EQ2 */
152         { 0x65, 0x00 }, /* 65 DAI1 EQ2 */
153         { 0x66, 0x00 }, /* 66 DAI1 EQ3 */
154         { 0x67, 0x00 }, /* 67 DAI1 EQ3 */
155         { 0x68, 0x00 }, /* 68 DAI1 EQ3 */
156         { 0x69, 0x00 }, /* 69 DAI1 EQ3 */
157         { 0x6a, 0x00 }, /* 6A DAI1 EQ3 */
158         { 0x6b, 0x00 }, /* 6B DAI1 EQ3 */
159         { 0x6c, 0x00 }, /* 6C DAI1 EQ3 */
160         { 0x6d, 0x00 }, /* 6D DAI1 EQ3 */
161         { 0x6e, 0x00 }, /* 6E DAI1 EQ3 */
162         { 0x6f, 0x00 }, /* 6F DAI1 EQ3 */
163
164         { 0x70, 0x00 }, /* 70 DAI1 EQ4 */
165         { 0x71, 0x00 }, /* 71 DAI1 EQ4 */
166         { 0x72, 0x00 }, /* 72 DAI1 EQ4 */
167         { 0x73, 0x00 }, /* 73 DAI1 EQ4 */
168         { 0x74, 0x00 }, /* 74 DAI1 EQ4 */
169         { 0x75, 0x00 }, /* 75 DAI1 EQ4 */
170         { 0x76, 0x00 }, /* 76 DAI1 EQ4 */
171         { 0x77, 0x00 }, /* 77 DAI1 EQ4 */
172         { 0x78, 0x00 }, /* 78 DAI1 EQ4 */
173         { 0x79, 0x00 }, /* 79 DAI1 EQ4 */
174         { 0x7a, 0x00 }, /* 7A DAI1 EQ5 */
175         { 0x7b, 0x00 }, /* 7B DAI1 EQ5 */
176         { 0x7c, 0x00 }, /* 7C DAI1 EQ5 */
177         { 0x7d, 0x00 }, /* 7D DAI1 EQ5 */
178         { 0x7e, 0x00 }, /* 7E DAI1 EQ5 */
179         { 0x7f, 0x00 }, /* 7F DAI1 EQ5 */
180
181         { 0x80, 0x00 }, /* 80 DAI1 EQ5 */
182         { 0x81, 0x00 }, /* 81 DAI1 EQ5 */
183         { 0x82, 0x00 }, /* 82 DAI1 EQ5 */
184         { 0x83, 0x00 }, /* 83 DAI1 EQ5 */
185         { 0x84, 0x00 }, /* 84 DAI2 EQ1 */
186         { 0x85, 0x00 }, /* 85 DAI2 EQ1 */
187         { 0x86, 0x00 }, /* 86 DAI2 EQ1 */
188         { 0x87, 0x00 }, /* 87 DAI2 EQ1 */
189         { 0x88, 0x00 }, /* 88 DAI2 EQ1 */
190         { 0x89, 0x00 }, /* 89 DAI2 EQ1 */
191         { 0x8a, 0x00 }, /* 8A DAI2 EQ1 */
192         { 0x8b, 0x00 }, /* 8B DAI2 EQ1 */
193         { 0x8c, 0x00 }, /* 8C DAI2 EQ1 */
194         { 0x8d, 0x00 }, /* 8D DAI2 EQ1 */
195         { 0x8e, 0x00 }, /* 8E DAI2 EQ2 */
196         { 0x8f, 0x00 }, /* 8F DAI2 EQ2 */
197
198         { 0x90, 0x00 }, /* 90 DAI2 EQ2 */
199         { 0x91, 0x00 }, /* 91 DAI2 EQ2 */
200         { 0x92, 0x00 }, /* 92 DAI2 EQ2 */
201         { 0x93, 0x00 }, /* 93 DAI2 EQ2 */
202         { 0x94, 0x00 }, /* 94 DAI2 EQ2 */
203         { 0x95, 0x00 }, /* 95 DAI2 EQ2 */
204         { 0x96, 0x00 }, /* 96 DAI2 EQ2 */
205         { 0x97, 0x00 }, /* 97 DAI2 EQ2 */
206         { 0x98, 0x00 }, /* 98 DAI2 EQ3 */
207         { 0x99, 0x00 }, /* 99 DAI2 EQ3 */
208         { 0x9a, 0x00 }, /* 9A DAI2 EQ3 */
209         { 0x9b, 0x00 }, /* 9B DAI2 EQ3 */
210         { 0x9c, 0x00 }, /* 9C DAI2 EQ3 */
211         { 0x9d, 0x00 }, /* 9D DAI2 EQ3 */
212         { 0x9e, 0x00 }, /* 9E DAI2 EQ3 */
213         { 0x9f, 0x00 }, /* 9F DAI2 EQ3 */
214
215         { 0xa0, 0x00 }, /* A0 DAI2 EQ3 */
216         { 0xa1, 0x00 }, /* A1 DAI2 EQ3 */
217         { 0xa2, 0x00 }, /* A2 DAI2 EQ4 */
218         { 0xa3, 0x00 }, /* A3 DAI2 EQ4 */
219         { 0xa4, 0x00 }, /* A4 DAI2 EQ4 */
220         { 0xa5, 0x00 }, /* A5 DAI2 EQ4 */
221         { 0xa6, 0x00 }, /* A6 DAI2 EQ4 */
222         { 0xa7, 0x00 }, /* A7 DAI2 EQ4 */
223         { 0xa8, 0x00 }, /* A8 DAI2 EQ4 */
224         { 0xa9, 0x00 }, /* A9 DAI2 EQ4 */
225         { 0xaa, 0x00 }, /* AA DAI2 EQ4 */
226         { 0xab, 0x00 }, /* AB DAI2 EQ4 */
227         { 0xac, 0x00 }, /* AC DAI2 EQ5 */
228         { 0xad, 0x00 }, /* AD DAI2 EQ5 */
229         { 0xae, 0x00 }, /* AE DAI2 EQ5 */
230         { 0xaf, 0x00 }, /* AF DAI2 EQ5 */
231
232         { 0xb0, 0x00 }, /* B0 DAI2 EQ5 */
233         { 0xb1, 0x00 }, /* B1 DAI2 EQ5 */
234         { 0xb2, 0x00 }, /* B2 DAI2 EQ5 */
235         { 0xb3, 0x00 }, /* B3 DAI2 EQ5 */
236         { 0xb4, 0x00 }, /* B4 DAI2 EQ5 */
237         { 0xb5, 0x00 }, /* B5 DAI2 EQ5 */
238         { 0xb6, 0x00 }, /* B6 DAI1 biquad */
239         { 0xb7, 0x00 }, /* B7 DAI1 biquad */
240         { 0xb8 ,0x00 }, /* B8 DAI1 biquad */
241         { 0xb9, 0x00 }, /* B9 DAI1 biquad */
242         { 0xba, 0x00 }, /* BA DAI1 biquad */
243         { 0xbb, 0x00 }, /* BB DAI1 biquad */
244         { 0xbc, 0x00 }, /* BC DAI1 biquad */
245         { 0xbd, 0x00 }, /* BD DAI1 biquad */
246         { 0xbe, 0x00 }, /* BE DAI1 biquad */
247         { 0xbf, 0x00 }, /* BF DAI1 biquad */
248
249         { 0xc0, 0x00 }, /* C0 DAI2 biquad */
250         { 0xc1, 0x00 }, /* C1 DAI2 biquad */
251         { 0xc2, 0x00 }, /* C2 DAI2 biquad */
252         { 0xc3, 0x00 }, /* C3 DAI2 biquad */
253         { 0xc4, 0x00 }, /* C4 DAI2 biquad */
254         { 0xc5, 0x00 }, /* C5 DAI2 biquad */
255         { 0xc6, 0x00 }, /* C6 DAI2 biquad */
256         { 0xc7, 0x00 }, /* C7 DAI2 biquad */
257         { 0xc8, 0x00 }, /* C8 DAI2 biquad */
258         { 0xc9, 0x00 }, /* C9 DAI2 biquad */
259 };
260
261 static bool max98088_readable_register(struct device *dev, unsigned int reg)
262 {
263         switch (reg) {
264         case M98088_REG_00_IRQ_STATUS ... 0xC9:
265         case M98088_REG_FF_REV_ID:
266                 return true;
267         default:
268                 return false;
269         }
270 }
271
272 static bool max98088_writeable_register(struct device *dev, unsigned int reg)
273 {
274         switch (reg) {
275         case M98088_REG_03_BATTERY_VOLTAGE ... 0xC9:
276                 return true;
277         default:
278                 return false;
279         }
280 }
281
282 static bool max98088_volatile_register(struct device *dev, unsigned int reg)
283 {
284         switch (reg) {
285         case M98088_REG_00_IRQ_STATUS ... M98088_REG_03_BATTERY_VOLTAGE:
286         case M98088_REG_FF_REV_ID:
287                 return true;
288         default:
289                 return false;
290         }
291 }
292
293 static const struct regmap_config max98088_regmap = {
294         .reg_bits = 8,
295         .val_bits = 8,
296
297         .readable_reg = max98088_readable_register,
298         .writeable_reg = max98088_writeable_register,
299         .volatile_reg = max98088_volatile_register,
300         .max_register = 0xff,
301
302         .reg_defaults = max98088_reg,
303         .num_reg_defaults = ARRAY_SIZE(max98088_reg),
304         .cache_type = REGCACHE_RBTREE,
305 };
306
307 /*
308  * Load equalizer DSP coefficient configurations registers
309  */
310 static void m98088_eq_band(struct snd_soc_codec *codec, unsigned int dai,
311                    unsigned int band, u16 *coefs)
312 {
313        unsigned int eq_reg;
314        unsigned int i;
315
316         if (WARN_ON(band > 4) ||
317             WARN_ON(dai > 1))
318                 return;
319
320        /* Load the base register address */
321        eq_reg = dai ? M98088_REG_84_DAI2_EQ_BASE : M98088_REG_52_DAI1_EQ_BASE;
322
323        /* Add the band address offset, note adjustment for word address */
324        eq_reg += band * (M98088_COEFS_PER_BAND << 1);
325
326        /* Step through the registers and coefs */
327        for (i = 0; i < M98088_COEFS_PER_BAND; i++) {
328                snd_soc_write(codec, eq_reg++, M98088_BYTE1(coefs[i]));
329                snd_soc_write(codec, eq_reg++, M98088_BYTE0(coefs[i]));
330        }
331 }
332
333 /*
334  * Excursion limiter modes
335  */
336 static const char *max98088_exmode_texts[] = {
337        "Off", "100Hz", "400Hz", "600Hz", "800Hz", "1000Hz", "200-400Hz",
338        "400-600Hz", "400-800Hz",
339 };
340
341 static const unsigned int max98088_exmode_values[] = {
342        0x00, 0x43, 0x10, 0x20, 0x30, 0x40, 0x11, 0x22, 0x32
343 };
344
345 static SOC_VALUE_ENUM_SINGLE_DECL(max98088_exmode_enum,
346                                   M98088_REG_41_SPKDHP, 0, 127,
347                                   max98088_exmode_texts,
348                                   max98088_exmode_values);
349
350 static const char *max98088_ex_thresh[] = { /* volts PP */
351        "0.6", "1.2", "1.8", "2.4", "3.0", "3.6", "4.2", "4.8"};
352 static SOC_ENUM_SINGLE_DECL(max98088_ex_thresh_enum,
353                             M98088_REG_42_SPKDHP_THRESH, 0,
354                             max98088_ex_thresh);
355
356 static const char *max98088_fltr_mode[] = {"Voice", "Music" };
357 static SOC_ENUM_SINGLE_DECL(max98088_filter_mode_enum,
358                             M98088_REG_18_DAI1_FILTERS, 7,
359                             max98088_fltr_mode);
360
361 static const char *max98088_extmic_text[] = { "None", "MIC1", "MIC2" };
362
363 static SOC_ENUM_SINGLE_DECL(max98088_extmic_enum,
364                             M98088_REG_48_CFG_MIC, 0,
365                             max98088_extmic_text);
366
367 static const struct snd_kcontrol_new max98088_extmic_mux =
368        SOC_DAPM_ENUM("External MIC Mux", max98088_extmic_enum);
369
370 static const char *max98088_dai1_fltr[] = {
371        "Off", "fc=258/fs=16k", "fc=500/fs=16k",
372        "fc=258/fs=8k", "fc=500/fs=8k", "fc=200"};
373 static SOC_ENUM_SINGLE_DECL(max98088_dai1_dac_filter_enum,
374                             M98088_REG_18_DAI1_FILTERS, 0,
375                             max98088_dai1_fltr);
376 static SOC_ENUM_SINGLE_DECL(max98088_dai1_adc_filter_enum,
377                             M98088_REG_18_DAI1_FILTERS, 4,
378                             max98088_dai1_fltr);
379
380 static int max98088_mic1pre_set(struct snd_kcontrol *kcontrol,
381                                struct snd_ctl_elem_value *ucontrol)
382 {
383        struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
384        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
385        unsigned int sel = ucontrol->value.integer.value[0];
386
387        max98088->mic1pre = sel;
388        snd_soc_update_bits(codec, M98088_REG_35_LVL_MIC1, M98088_MICPRE_MASK,
389                (1+sel)<<M98088_MICPRE_SHIFT);
390
391        return 0;
392 }
393
394 static int max98088_mic1pre_get(struct snd_kcontrol *kcontrol,
395                                struct snd_ctl_elem_value *ucontrol)
396 {
397        struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
398        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
399
400        ucontrol->value.integer.value[0] = max98088->mic1pre;
401        return 0;
402 }
403
404 static int max98088_mic2pre_set(struct snd_kcontrol *kcontrol,
405                                struct snd_ctl_elem_value *ucontrol)
406 {
407        struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
408        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
409        unsigned int sel = ucontrol->value.integer.value[0];
410
411        max98088->mic2pre = sel;
412        snd_soc_update_bits(codec, M98088_REG_36_LVL_MIC2, M98088_MICPRE_MASK,
413                (1+sel)<<M98088_MICPRE_SHIFT);
414
415        return 0;
416 }
417
418 static int max98088_mic2pre_get(struct snd_kcontrol *kcontrol,
419                                struct snd_ctl_elem_value *ucontrol)
420 {
421        struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
422        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
423
424        ucontrol->value.integer.value[0] = max98088->mic2pre;
425        return 0;
426 }
427
428 static const DECLARE_TLV_DB_RANGE(max98088_micboost_tlv,
429         0, 1, TLV_DB_SCALE_ITEM(0, 2000, 0),
430         2, 2, TLV_DB_SCALE_ITEM(3000, 0, 0)
431 );
432
433 static const DECLARE_TLV_DB_RANGE(max98088_hp_tlv,
434         0, 6, TLV_DB_SCALE_ITEM(-6700, 400, 0),
435         7, 14, TLV_DB_SCALE_ITEM(-4000, 300, 0),
436         15, 21, TLV_DB_SCALE_ITEM(-1700, 200, 0),
437         22, 27, TLV_DB_SCALE_ITEM(-400, 100, 0),
438         28, 31, TLV_DB_SCALE_ITEM(150, 50, 0)
439 );
440
441 static const DECLARE_TLV_DB_RANGE(max98088_spk_tlv,
442         0, 6, TLV_DB_SCALE_ITEM(-6200, 400, 0),
443         7, 14, TLV_DB_SCALE_ITEM(-3500, 300, 0),
444         15, 21, TLV_DB_SCALE_ITEM(-1200, 200, 0),
445         22, 27, TLV_DB_SCALE_ITEM(100, 100, 0),
446         28, 31, TLV_DB_SCALE_ITEM(650, 50, 0)
447 );
448
449 static const struct snd_kcontrol_new max98088_snd_controls[] = {
450
451         SOC_DOUBLE_R_TLV("Headphone Volume", M98088_REG_39_LVL_HP_L,
452                          M98088_REG_3A_LVL_HP_R, 0, 31, 0, max98088_hp_tlv),
453         SOC_DOUBLE_R_TLV("Speaker Volume", M98088_REG_3D_LVL_SPK_L,
454                          M98088_REG_3E_LVL_SPK_R, 0, 31, 0, max98088_spk_tlv),
455         SOC_DOUBLE_R_TLV("Receiver Volume", M98088_REG_3B_LVL_REC_L,
456                          M98088_REG_3C_LVL_REC_R, 0, 31, 0, max98088_spk_tlv),
457
458        SOC_DOUBLE_R("Headphone Switch", M98088_REG_39_LVL_HP_L,
459                M98088_REG_3A_LVL_HP_R, 7, 1, 1),
460        SOC_DOUBLE_R("Speaker Switch", M98088_REG_3D_LVL_SPK_L,
461                M98088_REG_3E_LVL_SPK_R, 7, 1, 1),
462        SOC_DOUBLE_R("Receiver Switch", M98088_REG_3B_LVL_REC_L,
463                M98088_REG_3C_LVL_REC_R, 7, 1, 1),
464
465        SOC_SINGLE("MIC1 Volume", M98088_REG_35_LVL_MIC1, 0, 31, 1),
466        SOC_SINGLE("MIC2 Volume", M98088_REG_36_LVL_MIC2, 0, 31, 1),
467
468        SOC_SINGLE_EXT_TLV("MIC1 Boost Volume",
469                        M98088_REG_35_LVL_MIC1, 5, 2, 0,
470                        max98088_mic1pre_get, max98088_mic1pre_set,
471                        max98088_micboost_tlv),
472        SOC_SINGLE_EXT_TLV("MIC2 Boost Volume",
473                        M98088_REG_36_LVL_MIC2, 5, 2, 0,
474                        max98088_mic2pre_get, max98088_mic2pre_set,
475                        max98088_micboost_tlv),
476
477        SOC_SINGLE("INA Volume", M98088_REG_37_LVL_INA, 0, 7, 1),
478        SOC_SINGLE("INB Volume", M98088_REG_38_LVL_INB, 0, 7, 1),
479
480        SOC_SINGLE("ADCL Volume", M98088_REG_33_LVL_ADC_L, 0, 15, 0),
481        SOC_SINGLE("ADCR Volume", M98088_REG_34_LVL_ADC_R, 0, 15, 0),
482
483        SOC_SINGLE("ADCL Boost Volume", M98088_REG_33_LVL_ADC_L, 4, 3, 0),
484        SOC_SINGLE("ADCR Boost Volume", M98088_REG_34_LVL_ADC_R, 4, 3, 0),
485
486        SOC_SINGLE("EQ1 Switch", M98088_REG_49_CFG_LEVEL, 0, 1, 0),
487        SOC_SINGLE("EQ2 Switch", M98088_REG_49_CFG_LEVEL, 1, 1, 0),
488
489        SOC_ENUM("EX Limiter Mode", max98088_exmode_enum),
490        SOC_ENUM("EX Limiter Threshold", max98088_ex_thresh_enum),
491
492        SOC_ENUM("DAI1 Filter Mode", max98088_filter_mode_enum),
493        SOC_ENUM("DAI1 DAC Filter", max98088_dai1_dac_filter_enum),
494        SOC_ENUM("DAI1 ADC Filter", max98088_dai1_adc_filter_enum),
495        SOC_SINGLE("DAI2 DC Block Switch", M98088_REG_20_DAI2_FILTERS,
496                0, 1, 0),
497
498        SOC_SINGLE("ALC Switch", M98088_REG_43_SPKALC_COMP, 7, 1, 0),
499        SOC_SINGLE("ALC Threshold", M98088_REG_43_SPKALC_COMP, 0, 7, 0),
500        SOC_SINGLE("ALC Multiband", M98088_REG_43_SPKALC_COMP, 3, 1, 0),
501        SOC_SINGLE("ALC Release Time", M98088_REG_43_SPKALC_COMP, 4, 7, 0),
502
503        SOC_SINGLE("PWR Limiter Threshold", M98088_REG_44_PWRLMT_CFG,
504                4, 15, 0),
505        SOC_SINGLE("PWR Limiter Weight", M98088_REG_44_PWRLMT_CFG, 0, 7, 0),
506        SOC_SINGLE("PWR Limiter Time1", M98088_REG_45_PWRLMT_TIME, 0, 15, 0),
507        SOC_SINGLE("PWR Limiter Time2", M98088_REG_45_PWRLMT_TIME, 4, 15, 0),
508
509        SOC_SINGLE("THD Limiter Threshold", M98088_REG_46_THDLMT_CFG, 4, 15, 0),
510        SOC_SINGLE("THD Limiter Time", M98088_REG_46_THDLMT_CFG, 0, 7, 0),
511 };
512
513 /* Left speaker mixer switch */
514 static const struct snd_kcontrol_new max98088_left_speaker_mixer_controls[] = {
515        SOC_DAPM_SINGLE("Left DAC1 Switch", M98088_REG_2B_MIX_SPK_LEFT, 0, 1, 0),
516        SOC_DAPM_SINGLE("Right DAC1 Switch", M98088_REG_2B_MIX_SPK_LEFT, 7, 1, 0),
517        SOC_DAPM_SINGLE("Left DAC2 Switch", M98088_REG_2B_MIX_SPK_LEFT, 0, 1, 0),
518        SOC_DAPM_SINGLE("Right DAC2 Switch", M98088_REG_2B_MIX_SPK_LEFT, 7, 1, 0),
519        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_2B_MIX_SPK_LEFT, 5, 1, 0),
520        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_2B_MIX_SPK_LEFT, 6, 1, 0),
521        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_2B_MIX_SPK_LEFT, 1, 1, 0),
522        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_2B_MIX_SPK_LEFT, 2, 1, 0),
523        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_2B_MIX_SPK_LEFT, 3, 1, 0),
524        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_2B_MIX_SPK_LEFT, 4, 1, 0),
525 };
526
527 /* Right speaker mixer switch */
528 static const struct snd_kcontrol_new max98088_right_speaker_mixer_controls[] = {
529        SOC_DAPM_SINGLE("Left DAC1 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 7, 1, 0),
530        SOC_DAPM_SINGLE("Right DAC1 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 0, 1, 0),
531        SOC_DAPM_SINGLE("Left DAC2 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 7, 1, 0),
532        SOC_DAPM_SINGLE("Right DAC2 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 0, 1, 0),
533        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 5, 1, 0),
534        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 6, 1, 0),
535        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 1, 1, 0),
536        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 2, 1, 0),
537        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 3, 1, 0),
538        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_2C_MIX_SPK_RIGHT, 4, 1, 0),
539 };
540
541 /* Left headphone mixer switch */
542 static const struct snd_kcontrol_new max98088_left_hp_mixer_controls[] = {
543        SOC_DAPM_SINGLE("Left DAC1 Switch", M98088_REG_25_MIX_HP_LEFT, 0, 1, 0),
544        SOC_DAPM_SINGLE("Right DAC1 Switch", M98088_REG_25_MIX_HP_LEFT, 7, 1, 0),
545        SOC_DAPM_SINGLE("Left DAC2 Switch", M98088_REG_25_MIX_HP_LEFT, 0, 1, 0),
546        SOC_DAPM_SINGLE("Right DAC2 Switch", M98088_REG_25_MIX_HP_LEFT, 7, 1, 0),
547        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_25_MIX_HP_LEFT, 5, 1, 0),
548        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_25_MIX_HP_LEFT, 6, 1, 0),
549        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_25_MIX_HP_LEFT, 1, 1, 0),
550        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_25_MIX_HP_LEFT, 2, 1, 0),
551        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_25_MIX_HP_LEFT, 3, 1, 0),
552        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_25_MIX_HP_LEFT, 4, 1, 0),
553 };
554
555 /* Right headphone mixer switch */
556 static const struct snd_kcontrol_new max98088_right_hp_mixer_controls[] = {
557        SOC_DAPM_SINGLE("Left DAC1 Switch", M98088_REG_26_MIX_HP_RIGHT, 7, 1, 0),
558        SOC_DAPM_SINGLE("Right DAC1 Switch", M98088_REG_26_MIX_HP_RIGHT, 0, 1, 0),
559        SOC_DAPM_SINGLE("Left DAC2 Switch", M98088_REG_26_MIX_HP_RIGHT, 7, 1, 0),
560        SOC_DAPM_SINGLE("Right DAC2 Switch", M98088_REG_26_MIX_HP_RIGHT, 0, 1, 0),
561        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_26_MIX_HP_RIGHT, 5, 1, 0),
562        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_26_MIX_HP_RIGHT, 6, 1, 0),
563        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_26_MIX_HP_RIGHT, 1, 1, 0),
564        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_26_MIX_HP_RIGHT, 2, 1, 0),
565        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_26_MIX_HP_RIGHT, 3, 1, 0),
566        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_26_MIX_HP_RIGHT, 4, 1, 0),
567 };
568
569 /* Left earpiece/receiver mixer switch */
570 static const struct snd_kcontrol_new max98088_left_rec_mixer_controls[] = {
571        SOC_DAPM_SINGLE("Left DAC1 Switch", M98088_REG_28_MIX_REC_LEFT, 0, 1, 0),
572        SOC_DAPM_SINGLE("Right DAC1 Switch", M98088_REG_28_MIX_REC_LEFT, 7, 1, 0),
573        SOC_DAPM_SINGLE("Left DAC2 Switch", M98088_REG_28_MIX_REC_LEFT, 0, 1, 0),
574        SOC_DAPM_SINGLE("Right DAC2 Switch", M98088_REG_28_MIX_REC_LEFT, 7, 1, 0),
575        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_28_MIX_REC_LEFT, 5, 1, 0),
576        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_28_MIX_REC_LEFT, 6, 1, 0),
577        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_28_MIX_REC_LEFT, 1, 1, 0),
578        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_28_MIX_REC_LEFT, 2, 1, 0),
579        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_28_MIX_REC_LEFT, 3, 1, 0),
580        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_28_MIX_REC_LEFT, 4, 1, 0),
581 };
582
583 /* Right earpiece/receiver mixer switch */
584 static const struct snd_kcontrol_new max98088_right_rec_mixer_controls[] = {
585        SOC_DAPM_SINGLE("Left DAC1 Switch", M98088_REG_29_MIX_REC_RIGHT, 7, 1, 0),
586        SOC_DAPM_SINGLE("Right DAC1 Switch", M98088_REG_29_MIX_REC_RIGHT, 0, 1, 0),
587        SOC_DAPM_SINGLE("Left DAC2 Switch", M98088_REG_29_MIX_REC_RIGHT, 7, 1, 0),
588        SOC_DAPM_SINGLE("Right DAC2 Switch", M98088_REG_29_MIX_REC_RIGHT, 0, 1, 0),
589        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_29_MIX_REC_RIGHT, 5, 1, 0),
590        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_29_MIX_REC_RIGHT, 6, 1, 0),
591        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_29_MIX_REC_RIGHT, 1, 1, 0),
592        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_29_MIX_REC_RIGHT, 2, 1, 0),
593        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_29_MIX_REC_RIGHT, 3, 1, 0),
594        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_29_MIX_REC_RIGHT, 4, 1, 0),
595 };
596
597 /* Left ADC mixer switch */
598 static const struct snd_kcontrol_new max98088_left_ADC_mixer_controls[] = {
599        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_23_MIX_ADC_LEFT, 7, 1, 0),
600        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_23_MIX_ADC_LEFT, 6, 1, 0),
601        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_23_MIX_ADC_LEFT, 3, 1, 0),
602        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_23_MIX_ADC_LEFT, 2, 1, 0),
603        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_23_MIX_ADC_LEFT, 1, 1, 0),
604        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_23_MIX_ADC_LEFT, 0, 1, 0),
605 };
606
607 /* Right ADC mixer switch */
608 static const struct snd_kcontrol_new max98088_right_ADC_mixer_controls[] = {
609        SOC_DAPM_SINGLE("MIC1 Switch", M98088_REG_24_MIX_ADC_RIGHT, 7, 1, 0),
610        SOC_DAPM_SINGLE("MIC2 Switch", M98088_REG_24_MIX_ADC_RIGHT, 6, 1, 0),
611        SOC_DAPM_SINGLE("INA1 Switch", M98088_REG_24_MIX_ADC_RIGHT, 3, 1, 0),
612        SOC_DAPM_SINGLE("INA2 Switch", M98088_REG_24_MIX_ADC_RIGHT, 2, 1, 0),
613        SOC_DAPM_SINGLE("INB1 Switch", M98088_REG_24_MIX_ADC_RIGHT, 1, 1, 0),
614        SOC_DAPM_SINGLE("INB2 Switch", M98088_REG_24_MIX_ADC_RIGHT, 0, 1, 0),
615 };
616
617 static int max98088_mic_event(struct snd_soc_dapm_widget *w,
618                             struct snd_kcontrol *kcontrol, int event)
619 {
620        struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
621        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
622
623        switch (event) {
624        case SND_SOC_DAPM_POST_PMU:
625                if (w->reg == M98088_REG_35_LVL_MIC1) {
626                        snd_soc_update_bits(codec, w->reg, M98088_MICPRE_MASK,
627                                (1+max98088->mic1pre)<<M98088_MICPRE_SHIFT);
628                } else {
629                        snd_soc_update_bits(codec, w->reg, M98088_MICPRE_MASK,
630                                (1+max98088->mic2pre)<<M98088_MICPRE_SHIFT);
631                }
632                break;
633        case SND_SOC_DAPM_POST_PMD:
634                snd_soc_update_bits(codec, w->reg, M98088_MICPRE_MASK, 0);
635                break;
636        default:
637                return -EINVAL;
638        }
639
640        return 0;
641 }
642
643 /*
644  * The line inputs are 2-channel stereo inputs with the left
645  * and right channels sharing a common PGA power control signal.
646  */
647 static int max98088_line_pga(struct snd_soc_dapm_widget *w,
648                             int event, int line, u8 channel)
649 {
650        struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
651        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
652        u8 *state;
653
654         if (WARN_ON(!(channel == 1 || channel == 2)))
655                 return -EINVAL;
656
657        switch (line) {
658        case LINE_INA:
659                state = &max98088->ina_state;
660                break;
661        case LINE_INB:
662                state = &max98088->inb_state;
663                break;
664        default:
665                return -EINVAL;
666        }
667
668        switch (event) {
669        case SND_SOC_DAPM_POST_PMU:
670                *state |= channel;
671                snd_soc_update_bits(codec, w->reg,
672                        (1 << w->shift), (1 << w->shift));
673                break;
674        case SND_SOC_DAPM_POST_PMD:
675                *state &= ~channel;
676                if (*state == 0) {
677                        snd_soc_update_bits(codec, w->reg,
678                                (1 << w->shift), 0);
679                }
680                break;
681        default:
682                return -EINVAL;
683        }
684
685        return 0;
686 }
687
688 static int max98088_pga_ina1_event(struct snd_soc_dapm_widget *w,
689                                   struct snd_kcontrol *k, int event)
690 {
691        return max98088_line_pga(w, event, LINE_INA, 1);
692 }
693
694 static int max98088_pga_ina2_event(struct snd_soc_dapm_widget *w,
695                                   struct snd_kcontrol *k, int event)
696 {
697        return max98088_line_pga(w, event, LINE_INA, 2);
698 }
699
700 static int max98088_pga_inb1_event(struct snd_soc_dapm_widget *w,
701                                   struct snd_kcontrol *k, int event)
702 {
703        return max98088_line_pga(w, event, LINE_INB, 1);
704 }
705
706 static int max98088_pga_inb2_event(struct snd_soc_dapm_widget *w,
707                                   struct snd_kcontrol *k, int event)
708 {
709        return max98088_line_pga(w, event, LINE_INB, 2);
710 }
711
712 static const struct snd_soc_dapm_widget max98088_dapm_widgets[] = {
713
714        SND_SOC_DAPM_ADC("ADCL", "HiFi Capture", M98088_REG_4C_PWR_EN_IN, 1, 0),
715        SND_SOC_DAPM_ADC("ADCR", "HiFi Capture", M98088_REG_4C_PWR_EN_IN, 0, 0),
716
717        SND_SOC_DAPM_DAC("DACL1", "HiFi Playback",
718                M98088_REG_4D_PWR_EN_OUT, 1, 0),
719        SND_SOC_DAPM_DAC("DACR1", "HiFi Playback",
720                M98088_REG_4D_PWR_EN_OUT, 0, 0),
721        SND_SOC_DAPM_DAC("DACL2", "Aux Playback",
722                M98088_REG_4D_PWR_EN_OUT, 1, 0),
723        SND_SOC_DAPM_DAC("DACR2", "Aux Playback",
724                M98088_REG_4D_PWR_EN_OUT, 0, 0),
725
726        SND_SOC_DAPM_PGA("HP Left Out", M98088_REG_4D_PWR_EN_OUT,
727                7, 0, NULL, 0),
728        SND_SOC_DAPM_PGA("HP Right Out", M98088_REG_4D_PWR_EN_OUT,
729                6, 0, NULL, 0),
730
731        SND_SOC_DAPM_PGA("SPK Left Out", M98088_REG_4D_PWR_EN_OUT,
732                5, 0, NULL, 0),
733        SND_SOC_DAPM_PGA("SPK Right Out", M98088_REG_4D_PWR_EN_OUT,
734                4, 0, NULL, 0),
735
736        SND_SOC_DAPM_PGA("REC Left Out", M98088_REG_4D_PWR_EN_OUT,
737                3, 0, NULL, 0),
738        SND_SOC_DAPM_PGA("REC Right Out", M98088_REG_4D_PWR_EN_OUT,
739                2, 0, NULL, 0),
740
741        SND_SOC_DAPM_MUX("External MIC", SND_SOC_NOPM, 0, 0,
742                &max98088_extmic_mux),
743
744        SND_SOC_DAPM_MIXER("Left HP Mixer", SND_SOC_NOPM, 0, 0,
745                &max98088_left_hp_mixer_controls[0],
746                ARRAY_SIZE(max98088_left_hp_mixer_controls)),
747
748        SND_SOC_DAPM_MIXER("Right HP Mixer", SND_SOC_NOPM, 0, 0,
749                &max98088_right_hp_mixer_controls[0],
750                ARRAY_SIZE(max98088_right_hp_mixer_controls)),
751
752        SND_SOC_DAPM_MIXER("Left SPK Mixer", SND_SOC_NOPM, 0, 0,
753                &max98088_left_speaker_mixer_controls[0],
754                ARRAY_SIZE(max98088_left_speaker_mixer_controls)),
755
756        SND_SOC_DAPM_MIXER("Right SPK Mixer", SND_SOC_NOPM, 0, 0,
757                &max98088_right_speaker_mixer_controls[0],
758                ARRAY_SIZE(max98088_right_speaker_mixer_controls)),
759
760        SND_SOC_DAPM_MIXER("Left REC Mixer", SND_SOC_NOPM, 0, 0,
761          &max98088_left_rec_mixer_controls[0],
762                ARRAY_SIZE(max98088_left_rec_mixer_controls)),
763
764        SND_SOC_DAPM_MIXER("Right REC Mixer", SND_SOC_NOPM, 0, 0,
765          &max98088_right_rec_mixer_controls[0],
766                ARRAY_SIZE(max98088_right_rec_mixer_controls)),
767
768        SND_SOC_DAPM_MIXER("Left ADC Mixer", SND_SOC_NOPM, 0, 0,
769                &max98088_left_ADC_mixer_controls[0],
770                ARRAY_SIZE(max98088_left_ADC_mixer_controls)),
771
772        SND_SOC_DAPM_MIXER("Right ADC Mixer", SND_SOC_NOPM, 0, 0,
773                &max98088_right_ADC_mixer_controls[0],
774                ARRAY_SIZE(max98088_right_ADC_mixer_controls)),
775
776        SND_SOC_DAPM_PGA_E("MIC1 Input", M98088_REG_35_LVL_MIC1,
777                5, 0, NULL, 0, max98088_mic_event,
778                SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
779
780        SND_SOC_DAPM_PGA_E("MIC2 Input", M98088_REG_36_LVL_MIC2,
781                5, 0, NULL, 0, max98088_mic_event,
782                SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
783
784        SND_SOC_DAPM_PGA_E("INA1 Input", M98088_REG_4C_PWR_EN_IN,
785                7, 0, NULL, 0, max98088_pga_ina1_event,
786                SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
787
788        SND_SOC_DAPM_PGA_E("INA2 Input", M98088_REG_4C_PWR_EN_IN,
789                7, 0, NULL, 0, max98088_pga_ina2_event,
790                SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
791
792        SND_SOC_DAPM_PGA_E("INB1 Input", M98088_REG_4C_PWR_EN_IN,
793                6, 0, NULL, 0, max98088_pga_inb1_event,
794                SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
795
796        SND_SOC_DAPM_PGA_E("INB2 Input", M98088_REG_4C_PWR_EN_IN,
797                6, 0, NULL, 0, max98088_pga_inb2_event,
798                SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_POST_PMD),
799
800        SND_SOC_DAPM_MICBIAS("MICBIAS", M98088_REG_4C_PWR_EN_IN, 3, 0),
801
802        SND_SOC_DAPM_OUTPUT("HPL"),
803        SND_SOC_DAPM_OUTPUT("HPR"),
804        SND_SOC_DAPM_OUTPUT("SPKL"),
805        SND_SOC_DAPM_OUTPUT("SPKR"),
806        SND_SOC_DAPM_OUTPUT("RECL"),
807        SND_SOC_DAPM_OUTPUT("RECR"),
808
809        SND_SOC_DAPM_INPUT("MIC1"),
810        SND_SOC_DAPM_INPUT("MIC2"),
811        SND_SOC_DAPM_INPUT("INA1"),
812        SND_SOC_DAPM_INPUT("INA2"),
813        SND_SOC_DAPM_INPUT("INB1"),
814        SND_SOC_DAPM_INPUT("INB2"),
815 };
816
817 static const struct snd_soc_dapm_route max98088_audio_map[] = {
818        /* Left headphone output mixer */
819        {"Left HP Mixer", "Left DAC1 Switch", "DACL1"},
820        {"Left HP Mixer", "Left DAC2 Switch", "DACL2"},
821        {"Left HP Mixer", "Right DAC1 Switch", "DACR1"},
822        {"Left HP Mixer", "Right DAC2 Switch", "DACR2"},
823        {"Left HP Mixer", "MIC1 Switch", "MIC1 Input"},
824        {"Left HP Mixer", "MIC2 Switch", "MIC2 Input"},
825        {"Left HP Mixer", "INA1 Switch", "INA1 Input"},
826        {"Left HP Mixer", "INA2 Switch", "INA2 Input"},
827        {"Left HP Mixer", "INB1 Switch", "INB1 Input"},
828        {"Left HP Mixer", "INB2 Switch", "INB2 Input"},
829
830        /* Right headphone output mixer */
831        {"Right HP Mixer", "Left DAC1 Switch", "DACL1"},
832        {"Right HP Mixer", "Left DAC2 Switch", "DACL2"  },
833        {"Right HP Mixer", "Right DAC1 Switch", "DACR1"},
834        {"Right HP Mixer", "Right DAC2 Switch", "DACR2"},
835        {"Right HP Mixer", "MIC1 Switch", "MIC1 Input"},
836        {"Right HP Mixer", "MIC2 Switch", "MIC2 Input"},
837        {"Right HP Mixer", "INA1 Switch", "INA1 Input"},
838        {"Right HP Mixer", "INA2 Switch", "INA2 Input"},
839        {"Right HP Mixer", "INB1 Switch", "INB1 Input"},
840        {"Right HP Mixer", "INB2 Switch", "INB2 Input"},
841
842        /* Left speaker output mixer */
843        {"Left SPK Mixer", "Left DAC1 Switch", "DACL1"},
844        {"Left SPK Mixer", "Left DAC2 Switch", "DACL2"},
845        {"Left SPK Mixer", "Right DAC1 Switch", "DACR1"},
846        {"Left SPK Mixer", "Right DAC2 Switch", "DACR2"},
847        {"Left SPK Mixer", "MIC1 Switch", "MIC1 Input"},
848        {"Left SPK Mixer", "MIC2 Switch", "MIC2 Input"},
849        {"Left SPK Mixer", "INA1 Switch", "INA1 Input"},
850        {"Left SPK Mixer", "INA2 Switch", "INA2 Input"},
851        {"Left SPK Mixer", "INB1 Switch", "INB1 Input"},
852        {"Left SPK Mixer", "INB2 Switch", "INB2 Input"},
853
854        /* Right speaker output mixer */
855        {"Right SPK Mixer", "Left DAC1 Switch", "DACL1"},
856        {"Right SPK Mixer", "Left DAC2 Switch", "DACL2"},
857        {"Right SPK Mixer", "Right DAC1 Switch", "DACR1"},
858        {"Right SPK Mixer", "Right DAC2 Switch", "DACR2"},
859        {"Right SPK Mixer", "MIC1 Switch", "MIC1 Input"},
860        {"Right SPK Mixer", "MIC2 Switch", "MIC2 Input"},
861        {"Right SPK Mixer", "INA1 Switch", "INA1 Input"},
862        {"Right SPK Mixer", "INA2 Switch", "INA2 Input"},
863        {"Right SPK Mixer", "INB1 Switch", "INB1 Input"},
864        {"Right SPK Mixer", "INB2 Switch", "INB2 Input"},
865
866        /* Earpiece/Receiver output mixer */
867        {"Left REC Mixer", "Left DAC1 Switch", "DACL1"},
868        {"Left REC Mixer", "Left DAC2 Switch", "DACL2"},
869        {"Left REC Mixer", "Right DAC1 Switch", "DACR1"},
870        {"Left REC Mixer", "Right DAC2 Switch", "DACR2"},
871        {"Left REC Mixer", "MIC1 Switch", "MIC1 Input"},
872        {"Left REC Mixer", "MIC2 Switch", "MIC2 Input"},
873        {"Left REC Mixer", "INA1 Switch", "INA1 Input"},
874        {"Left REC Mixer", "INA2 Switch", "INA2 Input"},
875        {"Left REC Mixer", "INB1 Switch", "INB1 Input"},
876        {"Left REC Mixer", "INB2 Switch", "INB2 Input"},
877
878        /* Earpiece/Receiver output mixer */
879        {"Right REC Mixer", "Left DAC1 Switch", "DACL1"},
880        {"Right REC Mixer", "Left DAC2 Switch", "DACL2"},
881        {"Right REC Mixer", "Right DAC1 Switch", "DACR1"},
882        {"Right REC Mixer", "Right DAC2 Switch", "DACR2"},
883        {"Right REC Mixer", "MIC1 Switch", "MIC1 Input"},
884        {"Right REC Mixer", "MIC2 Switch", "MIC2 Input"},
885        {"Right REC Mixer", "INA1 Switch", "INA1 Input"},
886        {"Right REC Mixer", "INA2 Switch", "INA2 Input"},
887        {"Right REC Mixer", "INB1 Switch", "INB1 Input"},
888        {"Right REC Mixer", "INB2 Switch", "INB2 Input"},
889
890        {"HP Left Out", NULL, "Left HP Mixer"},
891        {"HP Right Out", NULL, "Right HP Mixer"},
892        {"SPK Left Out", NULL, "Left SPK Mixer"},
893        {"SPK Right Out", NULL, "Right SPK Mixer"},
894        {"REC Left Out", NULL, "Left REC Mixer"},
895        {"REC Right Out", NULL, "Right REC Mixer"},
896
897        {"HPL", NULL, "HP Left Out"},
898        {"HPR", NULL, "HP Right Out"},
899        {"SPKL", NULL, "SPK Left Out"},
900        {"SPKR", NULL, "SPK Right Out"},
901        {"RECL", NULL, "REC Left Out"},
902        {"RECR", NULL, "REC Right Out"},
903
904        /* Left ADC input mixer */
905        {"Left ADC Mixer", "MIC1 Switch", "MIC1 Input"},
906        {"Left ADC Mixer", "MIC2 Switch", "MIC2 Input"},
907        {"Left ADC Mixer", "INA1 Switch", "INA1 Input"},
908        {"Left ADC Mixer", "INA2 Switch", "INA2 Input"},
909        {"Left ADC Mixer", "INB1 Switch", "INB1 Input"},
910        {"Left ADC Mixer", "INB2 Switch", "INB2 Input"},
911
912        /* Right ADC input mixer */
913        {"Right ADC Mixer", "MIC1 Switch", "MIC1 Input"},
914        {"Right ADC Mixer", "MIC2 Switch", "MIC2 Input"},
915        {"Right ADC Mixer", "INA1 Switch", "INA1 Input"},
916        {"Right ADC Mixer", "INA2 Switch", "INA2 Input"},
917        {"Right ADC Mixer", "INB1 Switch", "INB1 Input"},
918        {"Right ADC Mixer", "INB2 Switch", "INB2 Input"},
919
920        /* Inputs */
921        {"ADCL", NULL, "Left ADC Mixer"},
922        {"ADCR", NULL, "Right ADC Mixer"},
923        {"INA1 Input", NULL, "INA1"},
924        {"INA2 Input", NULL, "INA2"},
925        {"INB1 Input", NULL, "INB1"},
926        {"INB2 Input", NULL, "INB2"},
927        {"MIC1 Input", NULL, "MIC1"},
928        {"MIC2 Input", NULL, "MIC2"},
929 };
930
931 /* codec mclk clock divider coefficients */
932 static const struct {
933        u32 rate;
934        u8  sr;
935 } rate_table[] = {
936        {8000,  0x10},
937        {11025, 0x20},
938        {16000, 0x30},
939        {22050, 0x40},
940        {24000, 0x50},
941        {32000, 0x60},
942        {44100, 0x70},
943        {48000, 0x80},
944        {88200, 0x90},
945        {96000, 0xA0},
946 };
947
948 static inline int rate_value(int rate, u8 *value)
949 {
950        int i;
951
952        for (i = 0; i < ARRAY_SIZE(rate_table); i++) {
953                if (rate_table[i].rate >= rate) {
954                        *value = rate_table[i].sr;
955                        return 0;
956                }
957        }
958        *value = rate_table[0].sr;
959        return -EINVAL;
960 }
961
962 static int max98088_dai1_hw_params(struct snd_pcm_substream *substream,
963                                   struct snd_pcm_hw_params *params,
964                                   struct snd_soc_dai *dai)
965 {
966        struct snd_soc_codec *codec = dai->codec;
967        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
968        struct max98088_cdata *cdata;
969        unsigned long long ni;
970        unsigned int rate;
971        u8 regval;
972
973        cdata = &max98088->dai[0];
974
975        rate = params_rate(params);
976
977        switch (params_width(params)) {
978        case 16:
979                snd_soc_update_bits(codec, M98088_REG_14_DAI1_FORMAT,
980                        M98088_DAI_WS, 0);
981                break;
982        case 24:
983                snd_soc_update_bits(codec, M98088_REG_14_DAI1_FORMAT,
984                        M98088_DAI_WS, M98088_DAI_WS);
985                break;
986        default:
987                return -EINVAL;
988        }
989
990        snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS, M98088_SHDNRUN, 0);
991
992        if (rate_value(rate, &regval))
993                return -EINVAL;
994
995        snd_soc_update_bits(codec, M98088_REG_11_DAI1_CLKMODE,
996                M98088_CLKMODE_MASK, regval);
997        cdata->rate = rate;
998
999        /* Configure NI when operating as master */
1000        if (snd_soc_read(codec, M98088_REG_14_DAI1_FORMAT)
1001                & M98088_DAI_MAS) {
1002                if (max98088->sysclk == 0) {
1003                        dev_err(codec->dev, "Invalid system clock frequency\n");
1004                        return -EINVAL;
1005                }
1006                ni = 65536ULL * (rate < 50000 ? 96ULL : 48ULL)
1007                                * (unsigned long long int)rate;
1008                do_div(ni, (unsigned long long int)max98088->sysclk);
1009                snd_soc_write(codec, M98088_REG_12_DAI1_CLKCFG_HI,
1010                        (ni >> 8) & 0x7F);
1011                snd_soc_write(codec, M98088_REG_13_DAI1_CLKCFG_LO,
1012                        ni & 0xFF);
1013        }
1014
1015        /* Update sample rate mode */
1016        if (rate < 50000)
1017                snd_soc_update_bits(codec, M98088_REG_18_DAI1_FILTERS,
1018                        M98088_DAI_DHF, 0);
1019        else
1020                snd_soc_update_bits(codec, M98088_REG_18_DAI1_FILTERS,
1021                        M98088_DAI_DHF, M98088_DAI_DHF);
1022
1023        snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS, M98088_SHDNRUN,
1024                M98088_SHDNRUN);
1025
1026        return 0;
1027 }
1028
1029 static int max98088_dai2_hw_params(struct snd_pcm_substream *substream,
1030                                   struct snd_pcm_hw_params *params,
1031                                   struct snd_soc_dai *dai)
1032 {
1033        struct snd_soc_codec *codec = dai->codec;
1034        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1035        struct max98088_cdata *cdata;
1036        unsigned long long ni;
1037        unsigned int rate;
1038        u8 regval;
1039
1040        cdata = &max98088->dai[1];
1041
1042        rate = params_rate(params);
1043
1044        switch (params_width(params)) {
1045        case 16:
1046                snd_soc_update_bits(codec, M98088_REG_1C_DAI2_FORMAT,
1047                        M98088_DAI_WS, 0);
1048                break;
1049        case 24:
1050                snd_soc_update_bits(codec, M98088_REG_1C_DAI2_FORMAT,
1051                        M98088_DAI_WS, M98088_DAI_WS);
1052                break;
1053        default:
1054                return -EINVAL;
1055        }
1056
1057        snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS, M98088_SHDNRUN, 0);
1058
1059        if (rate_value(rate, &regval))
1060                return -EINVAL;
1061
1062        snd_soc_update_bits(codec, M98088_REG_19_DAI2_CLKMODE,
1063                M98088_CLKMODE_MASK, regval);
1064        cdata->rate = rate;
1065
1066        /* Configure NI when operating as master */
1067        if (snd_soc_read(codec, M98088_REG_1C_DAI2_FORMAT)
1068                & M98088_DAI_MAS) {
1069                if (max98088->sysclk == 0) {
1070                        dev_err(codec->dev, "Invalid system clock frequency\n");
1071                        return -EINVAL;
1072                }
1073                ni = 65536ULL * (rate < 50000 ? 96ULL : 48ULL)
1074                                * (unsigned long long int)rate;
1075                do_div(ni, (unsigned long long int)max98088->sysclk);
1076                snd_soc_write(codec, M98088_REG_1A_DAI2_CLKCFG_HI,
1077                        (ni >> 8) & 0x7F);
1078                snd_soc_write(codec, M98088_REG_1B_DAI2_CLKCFG_LO,
1079                        ni & 0xFF);
1080        }
1081
1082        /* Update sample rate mode */
1083        if (rate < 50000)
1084                snd_soc_update_bits(codec, M98088_REG_20_DAI2_FILTERS,
1085                        M98088_DAI_DHF, 0);
1086        else
1087                snd_soc_update_bits(codec, M98088_REG_20_DAI2_FILTERS,
1088                        M98088_DAI_DHF, M98088_DAI_DHF);
1089
1090        snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS, M98088_SHDNRUN,
1091                M98088_SHDNRUN);
1092
1093        return 0;
1094 }
1095
1096 static int max98088_dai_set_sysclk(struct snd_soc_dai *dai,
1097                                   int clk_id, unsigned int freq, int dir)
1098 {
1099        struct snd_soc_codec *codec = dai->codec;
1100        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1101
1102        /* Requested clock frequency is already setup */
1103        if (freq == max98088->sysclk)
1104                return 0;
1105
1106        /* Setup clocks for slave mode, and using the PLL
1107         * PSCLK = 0x01 (when master clk is 10MHz to 20MHz)
1108         *         0x02 (when master clk is 20MHz to 30MHz)..
1109         */
1110        if ((freq >= 10000000) && (freq < 20000000)) {
1111                snd_soc_write(codec, M98088_REG_10_SYS_CLK, 0x10);
1112        } else if ((freq >= 20000000) && (freq < 30000000)) {
1113                snd_soc_write(codec, M98088_REG_10_SYS_CLK, 0x20);
1114        } else {
1115                dev_err(codec->dev, "Invalid master clock frequency\n");
1116                return -EINVAL;
1117        }
1118
1119        if (snd_soc_read(codec, M98088_REG_51_PWR_SYS)  & M98088_SHDNRUN) {
1120                snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS,
1121                        M98088_SHDNRUN, 0);
1122                snd_soc_update_bits(codec, M98088_REG_51_PWR_SYS,
1123                        M98088_SHDNRUN, M98088_SHDNRUN);
1124        }
1125
1126        dev_dbg(dai->dev, "Clock source is %d at %uHz\n", clk_id, freq);
1127
1128        max98088->sysclk = freq;
1129        return 0;
1130 }
1131
1132 static int max98088_dai1_set_fmt(struct snd_soc_dai *codec_dai,
1133                                 unsigned int fmt)
1134 {
1135        struct snd_soc_codec *codec = codec_dai->codec;
1136        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1137        struct max98088_cdata *cdata;
1138        u8 reg15val;
1139        u8 reg14val = 0;
1140
1141        cdata = &max98088->dai[0];
1142
1143        if (fmt != cdata->fmt) {
1144                cdata->fmt = fmt;
1145
1146                switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
1147                case SND_SOC_DAIFMT_CBS_CFS:
1148                        /* Slave mode PLL */
1149                        snd_soc_write(codec, M98088_REG_12_DAI1_CLKCFG_HI,
1150                                0x80);
1151                        snd_soc_write(codec, M98088_REG_13_DAI1_CLKCFG_LO,
1152                                0x00);
1153                        break;
1154                case SND_SOC_DAIFMT_CBM_CFM:
1155                        /* Set to master mode */
1156                        reg14val |= M98088_DAI_MAS;
1157                        break;
1158                case SND_SOC_DAIFMT_CBS_CFM:
1159                case SND_SOC_DAIFMT_CBM_CFS:
1160                default:
1161                        dev_err(codec->dev, "Clock mode unsupported");
1162                        return -EINVAL;
1163                }
1164
1165                switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
1166                case SND_SOC_DAIFMT_I2S:
1167                        reg14val |= M98088_DAI_DLY;
1168                        break;
1169                case SND_SOC_DAIFMT_LEFT_J:
1170                        break;
1171                default:
1172                        return -EINVAL;
1173                }
1174
1175                switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
1176                case SND_SOC_DAIFMT_NB_NF:
1177                        break;
1178                case SND_SOC_DAIFMT_NB_IF:
1179                        reg14val |= M98088_DAI_WCI;
1180                        break;
1181                case SND_SOC_DAIFMT_IB_NF:
1182                        reg14val |= M98088_DAI_BCI;
1183                        break;
1184                case SND_SOC_DAIFMT_IB_IF:
1185                        reg14val |= M98088_DAI_BCI|M98088_DAI_WCI;
1186                        break;
1187                default:
1188                        return -EINVAL;
1189                }
1190
1191                snd_soc_update_bits(codec, M98088_REG_14_DAI1_FORMAT,
1192                        M98088_DAI_MAS | M98088_DAI_DLY | M98088_DAI_BCI |
1193                        M98088_DAI_WCI, reg14val);
1194
1195                reg15val = M98088_DAI_BSEL64;
1196                if (max98088->digmic)
1197                        reg15val |= M98088_DAI_OSR64;
1198                snd_soc_write(codec, M98088_REG_15_DAI1_CLOCK, reg15val);
1199        }
1200
1201        return 0;
1202 }
1203
1204 static int max98088_dai2_set_fmt(struct snd_soc_dai *codec_dai,
1205                                 unsigned int fmt)
1206 {
1207        struct snd_soc_codec *codec = codec_dai->codec;
1208        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1209        struct max98088_cdata *cdata;
1210        u8 reg1Cval = 0;
1211
1212        cdata = &max98088->dai[1];
1213
1214        if (fmt != cdata->fmt) {
1215                cdata->fmt = fmt;
1216
1217                switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
1218                case SND_SOC_DAIFMT_CBS_CFS:
1219                        /* Slave mode PLL */
1220                        snd_soc_write(codec, M98088_REG_1A_DAI2_CLKCFG_HI,
1221                                0x80);
1222                        snd_soc_write(codec, M98088_REG_1B_DAI2_CLKCFG_LO,
1223                                0x00);
1224                        break;
1225                case SND_SOC_DAIFMT_CBM_CFM:
1226                        /* Set to master mode */
1227                        reg1Cval |= M98088_DAI_MAS;
1228                        break;
1229                case SND_SOC_DAIFMT_CBS_CFM:
1230                case SND_SOC_DAIFMT_CBM_CFS:
1231                default:
1232                        dev_err(codec->dev, "Clock mode unsupported");
1233                        return -EINVAL;
1234                }
1235
1236                switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
1237                case SND_SOC_DAIFMT_I2S:
1238                        reg1Cval |= M98088_DAI_DLY;
1239                        break;
1240                case SND_SOC_DAIFMT_LEFT_J:
1241                        break;
1242                default:
1243                        return -EINVAL;
1244                }
1245
1246                switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
1247                case SND_SOC_DAIFMT_NB_NF:
1248                        break;
1249                case SND_SOC_DAIFMT_NB_IF:
1250                        reg1Cval |= M98088_DAI_WCI;
1251                        break;
1252                case SND_SOC_DAIFMT_IB_NF:
1253                        reg1Cval |= M98088_DAI_BCI;
1254                        break;
1255                case SND_SOC_DAIFMT_IB_IF:
1256                        reg1Cval |= M98088_DAI_BCI|M98088_DAI_WCI;
1257                        break;
1258                default:
1259                        return -EINVAL;
1260                }
1261
1262                snd_soc_update_bits(codec, M98088_REG_1C_DAI2_FORMAT,
1263                        M98088_DAI_MAS | M98088_DAI_DLY | M98088_DAI_BCI |
1264                        M98088_DAI_WCI, reg1Cval);
1265
1266                snd_soc_write(codec, M98088_REG_1D_DAI2_CLOCK,
1267                        M98088_DAI_BSEL64);
1268        }
1269
1270        return 0;
1271 }
1272
1273 static int max98088_dai1_digital_mute(struct snd_soc_dai *codec_dai, int mute)
1274 {
1275        struct snd_soc_codec *codec = codec_dai->codec;
1276        int reg;
1277
1278        if (mute)
1279                reg = M98088_DAI_MUTE;
1280        else
1281                reg = 0;
1282
1283        snd_soc_update_bits(codec, M98088_REG_2F_LVL_DAI1_PLAY,
1284                            M98088_DAI_MUTE_MASK, reg);
1285        return 0;
1286 }
1287
1288 static int max98088_dai2_digital_mute(struct snd_soc_dai *codec_dai, int mute)
1289 {
1290        struct snd_soc_codec *codec = codec_dai->codec;
1291        int reg;
1292
1293        if (mute)
1294                reg = M98088_DAI_MUTE;
1295        else
1296                reg = 0;
1297
1298        snd_soc_update_bits(codec, M98088_REG_31_LVL_DAI2_PLAY,
1299                            M98088_DAI_MUTE_MASK, reg);
1300        return 0;
1301 }
1302
1303 static int max98088_set_bias_level(struct snd_soc_codec *codec,
1304                                   enum snd_soc_bias_level level)
1305 {
1306         struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1307
1308         switch (level) {
1309         case SND_SOC_BIAS_ON:
1310                 break;
1311
1312         case SND_SOC_BIAS_PREPARE:
1313                 break;
1314
1315         case SND_SOC_BIAS_STANDBY:
1316                 if (snd_soc_codec_get_bias_level(codec) == SND_SOC_BIAS_OFF)
1317                         regcache_sync(max98088->regmap);
1318
1319                 snd_soc_update_bits(codec, M98088_REG_4C_PWR_EN_IN,
1320                                    M98088_MBEN, M98088_MBEN);
1321                 break;
1322
1323         case SND_SOC_BIAS_OFF:
1324                 snd_soc_update_bits(codec, M98088_REG_4C_PWR_EN_IN,
1325                                     M98088_MBEN, 0);
1326                 regcache_mark_dirty(max98088->regmap);
1327                 break;
1328         }
1329         return 0;
1330 }
1331
1332 #define MAX98088_RATES SNDRV_PCM_RATE_8000_96000
1333 #define MAX98088_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S24_LE)
1334
1335 static const struct snd_soc_dai_ops max98088_dai1_ops = {
1336        .set_sysclk = max98088_dai_set_sysclk,
1337        .set_fmt = max98088_dai1_set_fmt,
1338        .hw_params = max98088_dai1_hw_params,
1339        .digital_mute = max98088_dai1_digital_mute,
1340 };
1341
1342 static const struct snd_soc_dai_ops max98088_dai2_ops = {
1343        .set_sysclk = max98088_dai_set_sysclk,
1344        .set_fmt = max98088_dai2_set_fmt,
1345        .hw_params = max98088_dai2_hw_params,
1346        .digital_mute = max98088_dai2_digital_mute,
1347 };
1348
1349 static struct snd_soc_dai_driver max98088_dai[] = {
1350 {
1351        .name = "HiFi",
1352        .playback = {
1353                .stream_name = "HiFi Playback",
1354                .channels_min = 1,
1355                .channels_max = 2,
1356                .rates = MAX98088_RATES,
1357                .formats = MAX98088_FORMATS,
1358        },
1359        .capture = {
1360                .stream_name = "HiFi Capture",
1361                .channels_min = 1,
1362                .channels_max = 2,
1363                .rates = MAX98088_RATES,
1364                .formats = MAX98088_FORMATS,
1365        },
1366         .ops = &max98088_dai1_ops,
1367 },
1368 {
1369        .name = "Aux",
1370        .playback = {
1371                .stream_name = "Aux Playback",
1372                .channels_min = 1,
1373                .channels_max = 2,
1374                .rates = MAX98088_RATES,
1375                .formats = MAX98088_FORMATS,
1376        },
1377        .ops = &max98088_dai2_ops,
1378 }
1379 };
1380
1381 static const char *eq_mode_name[] = {"EQ1 Mode", "EQ2 Mode"};
1382
1383 static int max98088_get_channel(struct snd_soc_codec *codec, const char *name)
1384 {
1385         int i;
1386
1387         for (i = 0; i < ARRAY_SIZE(eq_mode_name); i++)
1388                 if (strcmp(name, eq_mode_name[i]) == 0)
1389                         return i;
1390
1391         /* Shouldn't happen */
1392         dev_err(codec->dev, "Bad EQ channel name '%s'\n", name);
1393         return -EINVAL;
1394 }
1395
1396 static void max98088_setup_eq1(struct snd_soc_codec *codec)
1397 {
1398        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1399        struct max98088_pdata *pdata = max98088->pdata;
1400        struct max98088_eq_cfg *coef_set;
1401        int best, best_val, save, i, sel, fs;
1402        struct max98088_cdata *cdata;
1403
1404        cdata = &max98088->dai[0];
1405
1406        if (!pdata || !max98088->eq_textcnt)
1407                return;
1408
1409        /* Find the selected configuration with nearest sample rate */
1410        fs = cdata->rate;
1411        sel = cdata->eq_sel;
1412
1413        best = 0;
1414        best_val = INT_MAX;
1415        for (i = 0; i < pdata->eq_cfgcnt; i++) {
1416                if (strcmp(pdata->eq_cfg[i].name, max98088->eq_texts[sel]) == 0 &&
1417                    abs(pdata->eq_cfg[i].rate - fs) < best_val) {
1418                        best = i;
1419                        best_val = abs(pdata->eq_cfg[i].rate - fs);
1420                }
1421        }
1422
1423        dev_dbg(codec->dev, "Selected %s/%dHz for %dHz sample rate\n",
1424                pdata->eq_cfg[best].name,
1425                pdata->eq_cfg[best].rate, fs);
1426
1427        /* Disable EQ while configuring, and save current on/off state */
1428        save = snd_soc_read(codec, M98088_REG_49_CFG_LEVEL);
1429        snd_soc_update_bits(codec, M98088_REG_49_CFG_LEVEL, M98088_EQ1EN, 0);
1430
1431        coef_set = &pdata->eq_cfg[sel];
1432
1433        m98088_eq_band(codec, 0, 0, coef_set->band1);
1434        m98088_eq_band(codec, 0, 1, coef_set->band2);
1435        m98088_eq_band(codec, 0, 2, coef_set->band3);
1436        m98088_eq_band(codec, 0, 3, coef_set->band4);
1437        m98088_eq_band(codec, 0, 4, coef_set->band5);
1438
1439        /* Restore the original on/off state */
1440        snd_soc_update_bits(codec, M98088_REG_49_CFG_LEVEL, M98088_EQ1EN, save);
1441 }
1442
1443 static void max98088_setup_eq2(struct snd_soc_codec *codec)
1444 {
1445        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1446        struct max98088_pdata *pdata = max98088->pdata;
1447        struct max98088_eq_cfg *coef_set;
1448        int best, best_val, save, i, sel, fs;
1449        struct max98088_cdata *cdata;
1450
1451        cdata = &max98088->dai[1];
1452
1453        if (!pdata || !max98088->eq_textcnt)
1454                return;
1455
1456        /* Find the selected configuration with nearest sample rate */
1457        fs = cdata->rate;
1458
1459        sel = cdata->eq_sel;
1460        best = 0;
1461        best_val = INT_MAX;
1462        for (i = 0; i < pdata->eq_cfgcnt; i++) {
1463                if (strcmp(pdata->eq_cfg[i].name, max98088->eq_texts[sel]) == 0 &&
1464                    abs(pdata->eq_cfg[i].rate - fs) < best_val) {
1465                        best = i;
1466                        best_val = abs(pdata->eq_cfg[i].rate - fs);
1467                }
1468        }
1469
1470        dev_dbg(codec->dev, "Selected %s/%dHz for %dHz sample rate\n",
1471                pdata->eq_cfg[best].name,
1472                pdata->eq_cfg[best].rate, fs);
1473
1474        /* Disable EQ while configuring, and save current on/off state */
1475        save = snd_soc_read(codec, M98088_REG_49_CFG_LEVEL);
1476        snd_soc_update_bits(codec, M98088_REG_49_CFG_LEVEL, M98088_EQ2EN, 0);
1477
1478        coef_set = &pdata->eq_cfg[sel];
1479
1480        m98088_eq_band(codec, 1, 0, coef_set->band1);
1481        m98088_eq_band(codec, 1, 1, coef_set->band2);
1482        m98088_eq_band(codec, 1, 2, coef_set->band3);
1483        m98088_eq_band(codec, 1, 3, coef_set->band4);
1484        m98088_eq_band(codec, 1, 4, coef_set->band5);
1485
1486        /* Restore the original on/off state */
1487        snd_soc_update_bits(codec, M98088_REG_49_CFG_LEVEL, M98088_EQ2EN,
1488                save);
1489 }
1490
1491 static int max98088_put_eq_enum(struct snd_kcontrol *kcontrol,
1492                                 struct snd_ctl_elem_value *ucontrol)
1493 {
1494        struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
1495        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1496        struct max98088_pdata *pdata = max98088->pdata;
1497        int channel = max98088_get_channel(codec, kcontrol->id.name);
1498        struct max98088_cdata *cdata;
1499         int sel = ucontrol->value.enumerated.item[0];
1500
1501        if (channel < 0)
1502                return channel;
1503
1504        cdata = &max98088->dai[channel];
1505
1506        if (sel >= pdata->eq_cfgcnt)
1507                return -EINVAL;
1508
1509        cdata->eq_sel = sel;
1510
1511        switch (channel) {
1512        case 0:
1513                max98088_setup_eq1(codec);
1514                break;
1515        case 1:
1516                max98088_setup_eq2(codec);
1517                break;
1518        }
1519
1520        return 0;
1521 }
1522
1523 static int max98088_get_eq_enum(struct snd_kcontrol *kcontrol,
1524                                 struct snd_ctl_elem_value *ucontrol)
1525 {
1526        struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
1527        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1528        int channel = max98088_get_channel(codec, kcontrol->id.name);
1529        struct max98088_cdata *cdata;
1530
1531        if (channel < 0)
1532                return channel;
1533
1534        cdata = &max98088->dai[channel];
1535        ucontrol->value.enumerated.item[0] = cdata->eq_sel;
1536        return 0;
1537 }
1538
1539 static void max98088_handle_eq_pdata(struct snd_soc_codec *codec)
1540 {
1541        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1542        struct max98088_pdata *pdata = max98088->pdata;
1543        struct max98088_eq_cfg *cfg;
1544        unsigned int cfgcnt;
1545        int i, j;
1546        const char **t;
1547        int ret;
1548        struct snd_kcontrol_new controls[] = {
1549                SOC_ENUM_EXT((char *)eq_mode_name[0],
1550                        max98088->eq_enum,
1551                        max98088_get_eq_enum,
1552                        max98088_put_eq_enum),
1553                SOC_ENUM_EXT((char *)eq_mode_name[1],
1554                        max98088->eq_enum,
1555                        max98088_get_eq_enum,
1556                        max98088_put_eq_enum),
1557        };
1558        BUILD_BUG_ON(ARRAY_SIZE(controls) != ARRAY_SIZE(eq_mode_name));
1559
1560        cfg = pdata->eq_cfg;
1561        cfgcnt = pdata->eq_cfgcnt;
1562
1563        /* Setup an array of texts for the equalizer enum.
1564         * This is based on Mark Brown's equalizer driver code.
1565         */
1566        max98088->eq_textcnt = 0;
1567        max98088->eq_texts = NULL;
1568        for (i = 0; i < cfgcnt; i++) {
1569                for (j = 0; j < max98088->eq_textcnt; j++) {
1570                        if (strcmp(cfg[i].name, max98088->eq_texts[j]) == 0)
1571                                break;
1572                }
1573
1574                if (j != max98088->eq_textcnt)
1575                        continue;
1576
1577                /* Expand the array */
1578                t = krealloc(max98088->eq_texts,
1579                             sizeof(char *) * (max98088->eq_textcnt + 1),
1580                             GFP_KERNEL);
1581                if (t == NULL)
1582                        continue;
1583
1584                /* Store the new entry */
1585                t[max98088->eq_textcnt] = cfg[i].name;
1586                max98088->eq_textcnt++;
1587                max98088->eq_texts = t;
1588        }
1589
1590        /* Now point the soc_enum to .texts array items */
1591        max98088->eq_enum.texts = max98088->eq_texts;
1592        max98088->eq_enum.items = max98088->eq_textcnt;
1593
1594        ret = snd_soc_add_codec_controls(codec, controls, ARRAY_SIZE(controls));
1595        if (ret != 0)
1596                dev_err(codec->dev, "Failed to add EQ control: %d\n", ret);
1597 }
1598
1599 static void max98088_handle_pdata(struct snd_soc_codec *codec)
1600 {
1601        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1602        struct max98088_pdata *pdata = max98088->pdata;
1603        u8 regval = 0;
1604
1605        if (!pdata) {
1606                dev_dbg(codec->dev, "No platform data\n");
1607                return;
1608        }
1609
1610        /* Configure mic for analog/digital mic mode */
1611        if (pdata->digmic_left_mode)
1612                regval |= M98088_DIGMIC_L;
1613
1614        if (pdata->digmic_right_mode)
1615                regval |= M98088_DIGMIC_R;
1616
1617        max98088->digmic = (regval ? 1 : 0);
1618
1619        snd_soc_write(codec, M98088_REG_48_CFG_MIC, regval);
1620
1621        /* Configure receiver output */
1622        regval = ((pdata->receiver_mode) ? M98088_REC_LINEMODE : 0);
1623        snd_soc_update_bits(codec, M98088_REG_2A_MIC_REC_CNTL,
1624                M98088_REC_LINEMODE_MASK, regval);
1625
1626        /* Configure equalizers */
1627        if (pdata->eq_cfgcnt)
1628                max98088_handle_eq_pdata(codec);
1629 }
1630
1631 static int max98088_probe(struct snd_soc_codec *codec)
1632 {
1633        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1634        struct max98088_cdata *cdata;
1635        int ret = 0;
1636
1637        regcache_mark_dirty(max98088->regmap);
1638
1639        /* initialize private data */
1640
1641        max98088->sysclk = (unsigned)-1;
1642        max98088->eq_textcnt = 0;
1643
1644        cdata = &max98088->dai[0];
1645        cdata->rate = (unsigned)-1;
1646        cdata->fmt  = (unsigned)-1;
1647        cdata->eq_sel = 0;
1648
1649        cdata = &max98088->dai[1];
1650        cdata->rate = (unsigned)-1;
1651        cdata->fmt  = (unsigned)-1;
1652        cdata->eq_sel = 0;
1653
1654        max98088->ina_state = 0;
1655        max98088->inb_state = 0;
1656        max98088->ex_mode = 0;
1657        max98088->digmic = 0;
1658        max98088->mic1pre = 0;
1659        max98088->mic2pre = 0;
1660
1661        ret = snd_soc_read(codec, M98088_REG_FF_REV_ID);
1662        if (ret < 0) {
1663                dev_err(codec->dev, "Failed to read device revision: %d\n",
1664                        ret);
1665                goto err_access;
1666        }
1667        dev_info(codec->dev, "revision %c\n", ret - 0x40 + 'A');
1668
1669        snd_soc_write(codec, M98088_REG_51_PWR_SYS, M98088_PWRSV);
1670
1671        snd_soc_write(codec, M98088_REG_0F_IRQ_ENABLE, 0x00);
1672
1673        snd_soc_write(codec, M98088_REG_22_MIX_DAC,
1674                M98088_DAI1L_TO_DACL|M98088_DAI2L_TO_DACL|
1675                M98088_DAI1R_TO_DACR|M98088_DAI2R_TO_DACR);
1676
1677        snd_soc_write(codec, M98088_REG_4E_BIAS_CNTL, 0xF0);
1678        snd_soc_write(codec, M98088_REG_50_DAC_BIAS2, 0x0F);
1679
1680        snd_soc_write(codec, M98088_REG_16_DAI1_IOCFG,
1681                M98088_S1NORMAL|M98088_SDATA);
1682
1683        snd_soc_write(codec, M98088_REG_1E_DAI2_IOCFG,
1684                M98088_S2NORMAL|M98088_SDATA);
1685
1686        max98088_handle_pdata(codec);
1687
1688 err_access:
1689        return ret;
1690 }
1691
1692 static int max98088_remove(struct snd_soc_codec *codec)
1693 {
1694        struct max98088_priv *max98088 = snd_soc_codec_get_drvdata(codec);
1695
1696        kfree(max98088->eq_texts);
1697
1698        return 0;
1699 }
1700
1701 static struct snd_soc_codec_driver soc_codec_dev_max98088 = {
1702         .probe   = max98088_probe,
1703         .remove  = max98088_remove,
1704         .set_bias_level = max98088_set_bias_level,
1705         .suspend_bias_off = true,
1706
1707         .controls = max98088_snd_controls,
1708         .num_controls = ARRAY_SIZE(max98088_snd_controls),
1709         .dapm_widgets = max98088_dapm_widgets,
1710         .num_dapm_widgets = ARRAY_SIZE(max98088_dapm_widgets),
1711         .dapm_routes = max98088_audio_map,
1712         .num_dapm_routes = ARRAY_SIZE(max98088_audio_map),
1713 };
1714
1715 static int max98088_i2c_probe(struct i2c_client *i2c,
1716                               const struct i2c_device_id *id)
1717 {
1718        struct max98088_priv *max98088;
1719        int ret;
1720
1721        max98088 = devm_kzalloc(&i2c->dev, sizeof(struct max98088_priv),
1722                                GFP_KERNEL);
1723        if (max98088 == NULL)
1724                return -ENOMEM;
1725
1726        max98088->regmap = devm_regmap_init_i2c(i2c, &max98088_regmap);
1727        if (IS_ERR(max98088->regmap))
1728                return PTR_ERR(max98088->regmap);
1729
1730        max98088->devtype = id->driver_data;
1731
1732        i2c_set_clientdata(i2c, max98088);
1733        max98088->pdata = i2c->dev.platform_data;
1734
1735        ret = snd_soc_register_codec(&i2c->dev,
1736                        &soc_codec_dev_max98088, &max98088_dai[0], 2);
1737        return ret;
1738 }
1739
1740 static int max98088_i2c_remove(struct i2c_client *client)
1741 {
1742        snd_soc_unregister_codec(&client->dev);
1743        return 0;
1744 }
1745
1746 static const struct i2c_device_id max98088_i2c_id[] = {
1747        { "max98088", MAX98088 },
1748        { "max98089", MAX98089 },
1749        { }
1750 };
1751 MODULE_DEVICE_TABLE(i2c, max98088_i2c_id);
1752
1753 static struct i2c_driver max98088_i2c_driver = {
1754         .driver = {
1755                 .name = "max98088",
1756         },
1757         .probe  = max98088_i2c_probe,
1758         .remove = max98088_i2c_remove,
1759         .id_table = max98088_i2c_id,
1760 };
1761
1762 module_i2c_driver(max98088_i2c_driver);
1763
1764 MODULE_DESCRIPTION("ALSA SoC MAX98088 driver");
1765 MODULE_AUTHOR("Peter Hsiang, Jesse Marroquin");
1766 MODULE_LICENSE("GPL");
1767

This page was automatically generated by LXR 0.3.1 (source).  •  Linux is a registered trademark of Linus Torvalds  •  Contact us

    Home
    Development
    Services
    Training
    Docs
    Community
    Company
    Blog

