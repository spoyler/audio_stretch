/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : Sin_Table.c
 *    Description : Defines the Sine wave constant Table
 *
 *    History :
 *    1. Date        : 31.3.2008 
 *       Author      : Stoyan Choynev
 *       Description : inital revision
 *
 *    $Revision: #2 $
 **************************************************************************/

/** include files **/
#include "Sin_Table.h"
/** local definitions **/

/** default settings **/

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/
//Define user section 
//Used to put the Sine Table in the RAM for faster execution
#pragma section =".sinetable"
const short Sin_Table[ SAMPLES_NUM ] @ ".sinetable" =
{
    0,       6,      12,      18,      25,      31,      37,      43,      50,      56,      62,      68,      75,      81,      87,      94,
  100,     106,     112,     118,     125,     131,     137,     143,     150,     156,     162,     168,     174,     181,     187,     193,
  199,     205,     211,     218,     224,     230,     236,     242,     248,     254,     260,     266,     272,     278,     284,     290,
  296,     302,     308,     314,     320,     326,     332,     338,     344,     350,     356,     362,     368,     374,     379,     385,
  391,     397,     403,     408,     414,     420,     426,     431,     437,     443,     448,     454,     459,     465,     471,     476,
  482,     487,     493,     498,     504,     509,     515,     520,     525,     531,     536,     541,     547,     552,     557,     563,
  568,     573,     578,     583,     589,     594,     599,     604,     609,     614,     619,     624,     629,     634,     639,     644,
  648,     653,     658,     663,     668,     672,     677,     682,     687,     691,     696,     700,     705,     709,     714,     718,
  723,     727,     732,     736,     740,     745,     749,     753,     757,     762,     766,     770,     774,     778,     782,     786,
  790,     794,     798,     802,     806,     810,     814,     817,     821,     825,     829,     832,     836,     839,     843,     847,
  850,     854,     857,     860,     864,     867,     870,     874,     877,     880,     883,     886,     890,     893,     896,     899,
  902,     905,     908,     910,     913,     916,     919,     922,     924,     927,     930,     932,     935,     937,     940,     942,
  945,     947,     949,     952,     954,     956,     958,     961,     963,     965,     967,     969,     971,     973,     975,     977,
  978,     980,     982,     984,     985,     987,     989,     990,     992,     993,     995,     996,     998,     999,    1000,    1002,
 1003,    1004,    1005,    1006,    1007,    1008,    1010,    1010,    1011,    1012,    1013,    1014,    1015,    1016,    1016,    1017,
 1018,    1018,    1019,    1019,    1020,    1020,    1021,    1021,    1021,    1022,    1022,    1022,    1022,    1022,    1022,    1022,
 1023,    1022,    1022,    1022,    1022,    1022,    1022,    1022,    1021,    1021,    1021,    1020,    1020,    1019,    1019,    1018,
 1018,    1017,    1016,    1016,    1015,    1014,    1013,    1012,    1011,    1010,    1010,    1008,    1007,    1006,    1005,    1004,
 1003,    1002,    1000,     999,     998,     996,     995,     993,     992,     990,     989,     987,     985,     984,     982,     980,
  978,     977,     975,     973,     971,     969,     967,     965,     963,     961,     958,     956,     954,     952,     949,     947,
  945,     942,     940,     937,     935,     932,     930,     927,     924,     922,     919,     916,     913,     910,     908,     905,
  902,     899,     896,     893,     890,     886,     883,     880,     877,     874,     870,     867,     864,     860,     857,     854,
  850,     847,     843,     839,     836,     832,     829,     825,     821,     817,     814,     810,     806,     802,     798,     794,
  790,     786,     782,     778,     774,     770,     766,     762,     757,     753,     749,     745,     740,     736,     732,     727,
  723,     718,     714,     709,     705,     700,     696,     691,     687,     682,     677,     672,     668,     663,     658,     653,
  648,     644,     639,     634,     629,     624,     619,     614,     609,     604,     599,     594,     589,     583,     578,     573,
  568,     563,     557,     552,     547,     541,     536,     531,     525,     520,     515,     509,     504,     498,     493,     487,
  482,     476,     471,     465,     459,     454,     448,     443,     437,     431,     426,     420,     414,     408,     403,     397,
  391,     385,     379,     374,     368,     362,     356,     350,     344,     338,     332,     326,     320,     314,     308,     302,
  296,     290,     284,     278,     272,     266,     260,     254,     248,     242,     236,     230,     224,     218,     211,     205,
  199,     193,     187,     181,     174,     168,     162,     156,     150,     143,     137,     131,     125,     118,     112,     106,
  100,      94,      87,      81,      75,      68,      62,      56,      50,      43,      37,      31,      25,      18,      12,       6,
    0,      -7,     -13,     -19,     -26,     -32,     -38,     -44,     -51,     -57,     -63,     -69,     -76,     -82,     -88,     -95,
 -101,    -107,    -113,    -119,    -126,    -132,    -138,    -144,    -151,    -157,    -163,    -169,    -175,    -182,    -188,    -194,
 -200,    -206,    -212,    -219,    -225,    -231,    -237,    -243,    -249,    -255,    -261,    -267,    -273,    -279,    -285,    -291,
 -297,    -303,    -309,    -315,    -321,    -327,    -333,    -339,    -345,    -351,    -357,    -363,    -369,    -375,    -380,    -386,
 -392,    -398,    -404,    -409,    -415,    -421,    -427,    -432,    -438,    -444,    -449,    -455,    -460,    -466,    -472,    -477,
 -483,    -488,    -494,    -499,    -505,    -510,    -516,    -521,    -526,    -532,    -537,    -542,    -548,    -553,    -558,    -564,
 -569,    -574,    -579,    -584,    -590,    -595,    -600,    -605,    -610,    -615,    -620,    -625,    -630,    -635,    -640,    -645,
 -649,    -654,    -659,    -664,    -669,    -673,    -678,    -683,    -688,    -692,    -697,    -701,    -706,    -710,    -715,    -719,
 -724,    -728,    -733,    -737,    -741,    -746,    -750,    -754,    -758,    -763,    -767,    -771,    -775,    -779,    -783,    -787,
 -791,    -795,    -799,    -803,    -807,    -811,    -815,    -818,    -822,    -826,    -830,    -833,    -837,    -840,    -844,    -848,
 -851,    -855,    -858,    -861,    -865,    -868,    -871,    -875,    -878,    -881,    -884,    -887,    -891,    -894,    -897,    -900,
 -903,    -906,    -909,    -911,    -914,    -917,    -920,    -923,    -925,    -928,    -931,    -933,    -936,    -938,    -941,    -943,
 -946,    -948,    -950,    -953,    -955,    -957,    -959,    -962,    -964,    -966,    -968,    -970,    -972,    -974,    -976,    -978,
 -979,    -981,    -983,    -985,    -986,    -988,    -990,    -991,    -993,    -994,    -996,    -997,    -999,   -1000,   -1001,   -1003,
-1004,   -1005,   -1006,   -1007,   -1008,   -1009,   -1011,   -1011,   -1012,   -1013,   -1014,   -1015,   -1016,   -1017,   -1017,   -1018,
-1019,   -1019,   -1020,   -1020,   -1021,   -1021,   -1022,   -1022,   -1022,   -1023,   -1023,   -1023,   -1023,   -1023,   -1023,   -1023,
-1024,   -1023,   -1023,   -1023,   -1023,   -1023,   -1023,   -1023,   -1022,   -1022,   -1022,   -1021,   -1021,   -1020,   -1020,   -1019,
-1019,   -1018,   -1017,   -1017,   -1016,   -1015,   -1014,   -1013,   -1012,   -1011,   -1011,   -1009,   -1008,   -1007,   -1006,   -1005,
-1004,   -1003,   -1001,   -1000,    -999,    -997,    -996,    -994,    -993,    -991,    -990,    -988,    -986,    -985,    -983,    -981,
 -979,    -978,    -976,    -974,    -972,    -970,    -968,    -966,    -964,    -962,    -959,    -957,    -955,    -953,    -950,    -948,
 -946,    -943,    -941,    -938,    -936,    -933,    -931,    -928,    -925,    -923,    -920,    -917,    -914,    -911,    -909,    -906,
 -903,    -900,    -897,    -894,    -891,    -887,    -884,    -881,    -878,    -875,    -871,    -868,    -865,    -861,    -858,    -855,
 -851,    -848,    -844,    -840,    -837,    -833,    -830,    -826,    -822,    -818,    -815,    -811,    -807,    -803,    -799,    -795,
 -791,    -787,    -783,    -779,    -775,    -771,    -767,    -763,    -758,    -754,    -750,    -746,    -741,    -737,    -733,    -728,
 -724,    -719,    -715,    -710,    -706,    -701,    -697,    -692,    -688,    -683,    -678,    -673,    -669,    -664,    -659,    -654,
 -649,    -645,    -640,    -635,    -630,    -625,    -620,    -615,    -610,    -605,    -600,    -595,    -590,    -584,    -579,    -574,
 -569,    -564,    -558,    -553,    -548,    -542,    -537,    -532,    -526,    -521,    -516,    -510,    -505,    -499,    -494,    -488,
 -483,    -477,    -472,    -466,    -460,    -455,    -449,    -444,    -438,    -432,    -427,    -421,    -415,    -409,    -404,    -398,
 -392,    -386,    -380,    -375,    -369,    -363,    -357,    -351,    -345,    -339,    -333,    -327,    -321,    -315,    -309,    -303,
 -297,    -291,    -285,    -279,    -273,    -267,    -261,    -255,    -249,    -243,    -237,    -231,    -225,    -219,    -212,    -206,
 -200,    -194,    -188,    -182,    -175,    -169,    -163,    -157,    -151,    -144,    -138,    -132,    -126,    -119,    -113,    -107,
 -101,     -95,     -88,     -82,     -76,     -69,     -63,     -57,     -51,     -44,     -38,     -32,     -26,     -19,     -13,      -7,
};
/** private data **/

/** public functions **/

/** private functions **/

