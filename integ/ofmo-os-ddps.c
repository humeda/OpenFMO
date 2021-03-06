#include <stdio.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include "ofmo-twoint.h"
#include "fmt-m.h"

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#define HALF    0.5e0
#define ONE     1.e0
#define ZERO    0.e0


#define OFMO_EBUF_FULL          1
#define OFMO_EBUF_NOFULL        0

#define EPS_PS_PAIR     1.e-32
#define EPS_CS_PAIR2    1.e-30
#define MAXNPSPAIR 100

extern double* ofmo_os_getadd_vrr( const int mythread );
extern double* ofmo_os_getadd_hrr( const int mythread );
extern double* ofmo_os_getadd_eri( const int mythread );

extern void fmt( double F[],
        const int m, const double T, const double cssss );
extern int ofmo_integ_add_fock( const int nao, const size_t nstored_eri,
        const double eri_val[], const short int eri_ind4[],
        const double D[], double G[] );

extern double* ofmo_getadd_dfact();
static double *DFACT = NULL;

static void ofmo_hrr_clear_ddps( double *eh ) {
    int i;
    // (DS|PS)
    for ( i=0; i<(0+18); i++ ) eh[i] = 0.e0;
    // (FS|PS)
    for ( i=18; i<(18+30); i++ ) eh[i] = 0.e0;
    // (GS|PS)
    for ( i=48; i<(48+45); i++ ) eh[i] = 0.e0;
}

static void ofmo_hrr_calc_ddps(
        const double BA[3], const double DC[3], double *eh ) {
    // (DP,PS)
    eh[  93] = eh[  18] - BA[0]*eh[   0];
    eh[  94] = eh[  19] - BA[0]*eh[   1];
    eh[  95] = eh[  20] - BA[0]*eh[   2];
    eh[  96] = eh[  27] - BA[1]*eh[   0];
    eh[  97] = eh[  28] - BA[1]*eh[   1];
    eh[  98] = eh[  29] - BA[1]*eh[   2];
    eh[  99] = eh[  30] - BA[2]*eh[   0];
    eh[ 100] = eh[  31] - BA[2]*eh[   1];
    eh[ 101] = eh[  32] - BA[2]*eh[   2];
    eh[ 102] = eh[  33] - BA[0]*eh[   3];
    eh[ 103] = eh[  34] - BA[0]*eh[   4];
    eh[ 104] = eh[  35] - BA[0]*eh[   5];
    eh[ 105] = eh[  21] - BA[1]*eh[   3];
    eh[ 106] = eh[  22] - BA[1]*eh[   4];
    eh[ 107] = eh[  23] - BA[1]*eh[   5];
    eh[ 108] = eh[  42] - BA[2]*eh[   3];
    eh[ 109] = eh[  43] - BA[2]*eh[   4];
    eh[ 110] = eh[  44] - BA[2]*eh[   5];
    eh[ 111] = eh[  36] - BA[0]*eh[   6];
    eh[ 112] = eh[  37] - BA[0]*eh[   7];
    eh[ 113] = eh[  38] - BA[0]*eh[   8];
    eh[ 114] = eh[  45] - BA[1]*eh[   6];
    eh[ 115] = eh[  46] - BA[1]*eh[   7];
    eh[ 116] = eh[  47] - BA[1]*eh[   8];
    eh[ 117] = eh[  24] - BA[2]*eh[   6];
    eh[ 118] = eh[  25] - BA[2]*eh[   7];
    eh[ 119] = eh[  26] - BA[2]*eh[   8];
    eh[ 120] = eh[  27] - BA[0]*eh[   9];
    eh[ 121] = eh[  28] - BA[0]*eh[  10];
    eh[ 122] = eh[  29] - BA[0]*eh[  11];
    eh[ 123] = eh[  33] - BA[1]*eh[   9];
    eh[ 124] = eh[  34] - BA[1]*eh[  10];
    eh[ 125] = eh[  35] - BA[1]*eh[  11];
    eh[ 126] = eh[  39] - BA[2]*eh[   9];
    eh[ 127] = eh[  40] - BA[2]*eh[  10];
    eh[ 128] = eh[  41] - BA[2]*eh[  11];
    eh[ 129] = eh[  30] - BA[0]*eh[  12];
    eh[ 130] = eh[  31] - BA[0]*eh[  13];
    eh[ 131] = eh[  32] - BA[0]*eh[  14];
    eh[ 132] = eh[  39] - BA[1]*eh[  12];
    eh[ 133] = eh[  40] - BA[1]*eh[  13];
    eh[ 134] = eh[  41] - BA[1]*eh[  14];
    eh[ 135] = eh[  36] - BA[2]*eh[  12];
    eh[ 136] = eh[  37] - BA[2]*eh[  13];
    eh[ 137] = eh[  38] - BA[2]*eh[  14];
    eh[ 138] = eh[  39] - BA[0]*eh[  15];
    eh[ 139] = eh[  40] - BA[0]*eh[  16];
    eh[ 140] = eh[  41] - BA[0]*eh[  17];
    eh[ 141] = eh[  42] - BA[1]*eh[  15];
    eh[ 142] = eh[  43] - BA[1]*eh[  16];
    eh[ 143] = eh[  44] - BA[1]*eh[  17];
    eh[ 144] = eh[  45] - BA[2]*eh[  15];
    eh[ 145] = eh[  46] - BA[2]*eh[  16];
    eh[ 146] = eh[  47] - BA[2]*eh[  17];
    // (FP,PS)
    eh[ 147] = eh[  48] - BA[0]*eh[  18];
    eh[ 148] = eh[  49] - BA[0]*eh[  19];
    eh[ 149] = eh[  50] - BA[0]*eh[  20];
    eh[ 150] = eh[  57] - BA[1]*eh[  18];
    eh[ 151] = eh[  58] - BA[1]*eh[  19];
    eh[ 152] = eh[  59] - BA[1]*eh[  20];
    eh[ 153] = eh[  60] - BA[2]*eh[  18];
    eh[ 154] = eh[  61] - BA[2]*eh[  19];
    eh[ 155] = eh[  62] - BA[2]*eh[  20];
    eh[ 156] = eh[  72] - BA[0]*eh[  21];
    eh[ 157] = eh[  73] - BA[0]*eh[  22];
    eh[ 158] = eh[  74] - BA[0]*eh[  23];
    eh[ 159] = eh[  51] - BA[1]*eh[  21];
    eh[ 160] = eh[  52] - BA[1]*eh[  22];
    eh[ 161] = eh[  53] - BA[1]*eh[  23];
    eh[ 162] = eh[  84] - BA[2]*eh[  21];
    eh[ 163] = eh[  85] - BA[2]*eh[  22];
    eh[ 164] = eh[  86] - BA[2]*eh[  23];
    eh[ 165] = eh[  75] - BA[0]*eh[  24];
    eh[ 166] = eh[  76] - BA[0]*eh[  25];
    eh[ 167] = eh[  77] - BA[0]*eh[  26];
    eh[ 168] = eh[  87] - BA[1]*eh[  24];
    eh[ 169] = eh[  88] - BA[1]*eh[  25];
    eh[ 170] = eh[  89] - BA[1]*eh[  26];
    eh[ 171] = eh[  54] - BA[2]*eh[  24];
    eh[ 172] = eh[  55] - BA[2]*eh[  25];
    eh[ 173] = eh[  56] - BA[2]*eh[  26];
    eh[ 174] = eh[  57] - BA[0]*eh[  27];
    eh[ 175] = eh[  58] - BA[0]*eh[  28];
    eh[ 176] = eh[  59] - BA[0]*eh[  29];
    eh[ 177] = eh[  63] - BA[1]*eh[  27];
    eh[ 178] = eh[  64] - BA[1]*eh[  28];
    eh[ 179] = eh[  65] - BA[1]*eh[  29];
    eh[ 180] = eh[  69] - BA[2]*eh[  27];
    eh[ 181] = eh[  70] - BA[2]*eh[  28];
    eh[ 182] = eh[  71] - BA[2]*eh[  29];
    eh[ 183] = eh[  60] - BA[0]*eh[  30];
    eh[ 184] = eh[  61] - BA[0]*eh[  31];
    eh[ 185] = eh[  62] - BA[0]*eh[  32];
    eh[ 186] = eh[  69] - BA[1]*eh[  30];
    eh[ 187] = eh[  70] - BA[1]*eh[  31];
    eh[ 188] = eh[  71] - BA[1]*eh[  32];
    eh[ 189] = eh[  66] - BA[2]*eh[  30];
    eh[ 190] = eh[  67] - BA[2]*eh[  31];
    eh[ 191] = eh[  68] - BA[2]*eh[  32];
    eh[ 192] = eh[  63] - BA[0]*eh[  33];
    eh[ 193] = eh[  64] - BA[0]*eh[  34];
    eh[ 194] = eh[  65] - BA[0]*eh[  35];
    eh[ 195] = eh[  72] - BA[1]*eh[  33];
    eh[ 196] = eh[  73] - BA[1]*eh[  34];
    eh[ 197] = eh[  74] - BA[1]*eh[  35];
    eh[ 198] = eh[  78] - BA[2]*eh[  33];
    eh[ 199] = eh[  79] - BA[2]*eh[  34];
    eh[ 200] = eh[  80] - BA[2]*eh[  35];
    eh[ 201] = eh[  66] - BA[0]*eh[  36];
    eh[ 202] = eh[  67] - BA[0]*eh[  37];
    eh[ 203] = eh[  68] - BA[0]*eh[  38];
    eh[ 204] = eh[  81] - BA[1]*eh[  36];
    eh[ 205] = eh[  82] - BA[1]*eh[  37];
    eh[ 206] = eh[  83] - BA[1]*eh[  38];
    eh[ 207] = eh[  75] - BA[2]*eh[  36];
    eh[ 208] = eh[  76] - BA[2]*eh[  37];
    eh[ 209] = eh[  77] - BA[2]*eh[  38];
    eh[ 210] = eh[  69] - BA[0]*eh[  39];
    eh[ 211] = eh[  70] - BA[0]*eh[  40];
    eh[ 212] = eh[  71] - BA[0]*eh[  41];
    eh[ 213] = eh[  78] - BA[1]*eh[  39];
    eh[ 214] = eh[  79] - BA[1]*eh[  40];
    eh[ 215] = eh[  80] - BA[1]*eh[  41];
    eh[ 216] = eh[  81] - BA[2]*eh[  39];
    eh[ 217] = eh[  82] - BA[2]*eh[  40];
    eh[ 218] = eh[  83] - BA[2]*eh[  41];
    eh[ 219] = eh[  78] - BA[0]*eh[  42];
    eh[ 220] = eh[  79] - BA[0]*eh[  43];
    eh[ 221] = eh[  80] - BA[0]*eh[  44];
    eh[ 222] = eh[  84] - BA[1]*eh[  42];
    eh[ 223] = eh[  85] - BA[1]*eh[  43];
    eh[ 224] = eh[  86] - BA[1]*eh[  44];
    eh[ 225] = eh[  90] - BA[2]*eh[  42];
    eh[ 226] = eh[  91] - BA[2]*eh[  43];
    eh[ 227] = eh[  92] - BA[2]*eh[  44];
    eh[ 228] = eh[  81] - BA[0]*eh[  45];
    eh[ 229] = eh[  82] - BA[0]*eh[  46];
    eh[ 230] = eh[  83] - BA[0]*eh[  47];
    eh[ 231] = eh[  90] - BA[1]*eh[  45];
    eh[ 232] = eh[  91] - BA[1]*eh[  46];
    eh[ 233] = eh[  92] - BA[1]*eh[  47];
    eh[ 234] = eh[  87] - BA[2]*eh[  45];
    eh[ 235] = eh[  88] - BA[2]*eh[  46];
    eh[ 236] = eh[  89] - BA[2]*eh[  47];
    // (DD,PS)
    eh[ 237] = eh[ 147] - BA[0]*eh[  93];
    eh[ 238] = eh[ 148] - BA[0]*eh[  94];
    eh[ 239] = eh[ 149] - BA[0]*eh[  95];
    eh[ 240] = eh[ 177] - BA[1]*eh[  96];
    eh[ 241] = eh[ 178] - BA[1]*eh[  97];
    eh[ 242] = eh[ 179] - BA[1]*eh[  98];
    eh[ 243] = eh[ 189] - BA[2]*eh[  99];
    eh[ 244] = eh[ 190] - BA[2]*eh[ 100];
    eh[ 245] = eh[ 191] - BA[2]*eh[ 101];
    eh[ 246] = eh[ 150] - BA[0]*eh[  96];
    eh[ 247] = eh[ 151] - BA[0]*eh[  97];
    eh[ 248] = eh[ 152] - BA[0]*eh[  98];
    eh[ 249] = eh[ 153] - BA[0]*eh[  99];
    eh[ 250] = eh[ 154] - BA[0]*eh[ 100];
    eh[ 251] = eh[ 155] - BA[0]*eh[ 101];
    eh[ 252] = eh[ 180] - BA[1]*eh[  99];
    eh[ 253] = eh[ 181] - BA[1]*eh[ 100];
    eh[ 254] = eh[ 182] - BA[1]*eh[ 101];
    eh[ 255] = eh[ 192] - BA[0]*eh[ 102];
    eh[ 256] = eh[ 193] - BA[0]*eh[ 103];
    eh[ 257] = eh[ 194] - BA[0]*eh[ 104];
    eh[ 258] = eh[ 159] - BA[1]*eh[ 105];
    eh[ 259] = eh[ 160] - BA[1]*eh[ 106];
    eh[ 260] = eh[ 161] - BA[1]*eh[ 107];
    eh[ 261] = eh[ 225] - BA[2]*eh[ 108];
    eh[ 262] = eh[ 226] - BA[2]*eh[ 109];
    eh[ 263] = eh[ 227] - BA[2]*eh[ 110];
    eh[ 264] = eh[ 195] - BA[0]*eh[ 105];
    eh[ 265] = eh[ 196] - BA[0]*eh[ 106];
    eh[ 266] = eh[ 197] - BA[0]*eh[ 107];
    eh[ 267] = eh[ 198] - BA[0]*eh[ 108];
    eh[ 268] = eh[ 199] - BA[0]*eh[ 109];
    eh[ 269] = eh[ 200] - BA[0]*eh[ 110];
    eh[ 270] = eh[ 162] - BA[1]*eh[ 108];
    eh[ 271] = eh[ 163] - BA[1]*eh[ 109];
    eh[ 272] = eh[ 164] - BA[1]*eh[ 110];
    eh[ 273] = eh[ 201] - BA[0]*eh[ 111];
    eh[ 274] = eh[ 202] - BA[0]*eh[ 112];
    eh[ 275] = eh[ 203] - BA[0]*eh[ 113];
    eh[ 276] = eh[ 231] - BA[1]*eh[ 114];
    eh[ 277] = eh[ 232] - BA[1]*eh[ 115];
    eh[ 278] = eh[ 233] - BA[1]*eh[ 116];
    eh[ 279] = eh[ 171] - BA[2]*eh[ 117];
    eh[ 280] = eh[ 172] - BA[2]*eh[ 118];
    eh[ 281] = eh[ 173] - BA[2]*eh[ 119];
    eh[ 282] = eh[ 204] - BA[0]*eh[ 114];
    eh[ 283] = eh[ 205] - BA[0]*eh[ 115];
    eh[ 284] = eh[ 206] - BA[0]*eh[ 116];
    eh[ 285] = eh[ 207] - BA[0]*eh[ 117];
    eh[ 286] = eh[ 208] - BA[0]*eh[ 118];
    eh[ 287] = eh[ 209] - BA[0]*eh[ 119];
    eh[ 288] = eh[ 234] - BA[1]*eh[ 117];
    eh[ 289] = eh[ 235] - BA[1]*eh[ 118];
    eh[ 290] = eh[ 236] - BA[1]*eh[ 119];
    eh[ 291] = eh[ 174] - BA[0]*eh[ 120];
    eh[ 292] = eh[ 175] - BA[0]*eh[ 121];
    eh[ 293] = eh[ 176] - BA[0]*eh[ 122];
    eh[ 294] = eh[ 195] - BA[1]*eh[ 123];
    eh[ 295] = eh[ 196] - BA[1]*eh[ 124];
    eh[ 296] = eh[ 197] - BA[1]*eh[ 125];
    eh[ 297] = eh[ 216] - BA[2]*eh[ 126];
    eh[ 298] = eh[ 217] - BA[2]*eh[ 127];
    eh[ 299] = eh[ 218] - BA[2]*eh[ 128];
    eh[ 300] = eh[ 177] - BA[0]*eh[ 123];
    eh[ 301] = eh[ 178] - BA[0]*eh[ 124];
    eh[ 302] = eh[ 179] - BA[0]*eh[ 125];
    eh[ 303] = eh[ 180] - BA[0]*eh[ 126];
    eh[ 304] = eh[ 181] - BA[0]*eh[ 127];
    eh[ 305] = eh[ 182] - BA[0]*eh[ 128];
    eh[ 306] = eh[ 198] - BA[1]*eh[ 126];
    eh[ 307] = eh[ 199] - BA[1]*eh[ 127];
    eh[ 308] = eh[ 200] - BA[1]*eh[ 128];
    eh[ 309] = eh[ 183] - BA[0]*eh[ 129];
    eh[ 310] = eh[ 184] - BA[0]*eh[ 130];
    eh[ 311] = eh[ 185] - BA[0]*eh[ 131];
    eh[ 312] = eh[ 213] - BA[1]*eh[ 132];
    eh[ 313] = eh[ 214] - BA[1]*eh[ 133];
    eh[ 314] = eh[ 215] - BA[1]*eh[ 134];
    eh[ 315] = eh[ 207] - BA[2]*eh[ 135];
    eh[ 316] = eh[ 208] - BA[2]*eh[ 136];
    eh[ 317] = eh[ 209] - BA[2]*eh[ 137];
    eh[ 318] = eh[ 186] - BA[0]*eh[ 132];
    eh[ 319] = eh[ 187] - BA[0]*eh[ 133];
    eh[ 320] = eh[ 188] - BA[0]*eh[ 134];
    eh[ 321] = eh[ 189] - BA[0]*eh[ 135];
    eh[ 322] = eh[ 190] - BA[0]*eh[ 136];
    eh[ 323] = eh[ 191] - BA[0]*eh[ 137];
    eh[ 324] = eh[ 216] - BA[1]*eh[ 135];
    eh[ 325] = eh[ 217] - BA[1]*eh[ 136];
    eh[ 326] = eh[ 218] - BA[1]*eh[ 137];
    eh[ 327] = eh[ 210] - BA[0]*eh[ 138];
    eh[ 328] = eh[ 211] - BA[0]*eh[ 139];
    eh[ 329] = eh[ 212] - BA[0]*eh[ 140];
    eh[ 330] = eh[ 222] - BA[1]*eh[ 141];
    eh[ 331] = eh[ 223] - BA[1]*eh[ 142];
    eh[ 332] = eh[ 224] - BA[1]*eh[ 143];
    eh[ 333] = eh[ 234] - BA[2]*eh[ 144];
    eh[ 334] = eh[ 235] - BA[2]*eh[ 145];
    eh[ 335] = eh[ 236] - BA[2]*eh[ 146];
    eh[ 336] = eh[ 213] - BA[0]*eh[ 141];
    eh[ 337] = eh[ 214] - BA[0]*eh[ 142];
    eh[ 338] = eh[ 215] - BA[0]*eh[ 143];
    eh[ 339] = eh[ 216] - BA[0]*eh[ 144];
    eh[ 340] = eh[ 217] - BA[0]*eh[ 145];
    eh[ 341] = eh[ 218] - BA[0]*eh[ 146];
    eh[ 342] = eh[ 225] - BA[1]*eh[ 144];
    eh[ 343] = eh[ 226] - BA[1]*eh[ 145];
    eh[ 344] = eh[ 227] - BA[1]*eh[ 146];
    // HRR for (XX|XX)-type integral (center CD)
}

static void ofmo_hrr_coef_ddps(
        const double *eh, double *DINT ) {
    int i, j, k, l, iao, jao, kao, lao, ix, iy;
    double coef_a, coef_ab, coef_abc;
    ix = 237;
    iy = 0;

    for ( i=0, iao=4; i<6; i++, iao++ ) {
        coef_a = DFACT[iao];
        for ( j=0, jao=4; j<6; j++, jao++ ) {
            coef_ab = coef_a * DFACT[jao];
            for ( k=0, kao=1; k<3; k++, kao++ ) {
                coef_abc = coef_ab * DFACT[kao];
                DINT[iy] = coef_abc * eh[ix];
                iy++;
                ix++;
            }
        }
    }
}

static void ofmo_vrr_calc_ddps(
        const double T, const double cssss,
        const double zeta2, const double eta2, const double ze2,
        const double rz, const double re,
        const double PA[3], const double WP[3],
        const double QC[3], const double WQ[3],
        double *ev ) {
    // (ss|ss) m=0,5
    //fmt( &ev[0], 5, T, cssss );
    OFMO_FMT( &ev[0], 5, T, cssss );
    // (ps|ss) m=0,4
    ev[ 6]=PA[0]*ev[0]+WP[0]*ev[1];
    ev[ 7]=PA[1]*ev[0]+WP[1]*ev[1];
    ev[ 8]=PA[2]*ev[0]+WP[2]*ev[1];
    ev[ 9]=PA[0]*ev[1]+WP[0]*ev[2];
    ev[10]=PA[1]*ev[1]+WP[1]*ev[2];
    ev[11]=PA[2]*ev[1]+WP[2]*ev[2];
    ev[12]=PA[0]*ev[2]+WP[0]*ev[3];
    ev[13]=PA[1]*ev[2]+WP[1]*ev[3];
    ev[14]=PA[2]*ev[2]+WP[2]*ev[3];
    ev[15]=PA[0]*ev[3]+WP[0]*ev[4];
    ev[16]=PA[1]*ev[3]+WP[1]*ev[4];
    ev[17]=PA[2]*ev[3]+WP[2]*ev[4];
    ev[18]=PA[0]*ev[4]+WP[0]*ev[5];
    ev[19]=PA[1]*ev[4]+WP[1]*ev[5];
    ev[20]=PA[2]*ev[4]+WP[2]*ev[5];
    // (ds|ss) m=0,3
    ev[21]=PA[0]*ev[ 6]+WP[0]*ev[ 9]+zeta2*(ev[0]-rz*ev[1]);
    ev[22]=PA[1]*ev[ 7]+WP[1]*ev[10]+zeta2*(ev[0]-rz*ev[1]);
    ev[23]=PA[2]*ev[ 8]+WP[2]*ev[11]+zeta2*(ev[0]-rz*ev[1]);
    ev[24]=PA[0]*ev[ 7]+WP[0]*ev[10];
    ev[25]=PA[0]*ev[ 8]+WP[0]*ev[11];
    ev[26]=PA[1]*ev[ 8]+WP[1]*ev[11];
    ev[27]=PA[0]*ev[ 9]+WP[0]*ev[12]+zeta2*(ev[1]-rz*ev[2]);
    ev[28]=PA[1]*ev[10]+WP[1]*ev[13]+zeta2*(ev[1]-rz*ev[2]);
    ev[29]=PA[2]*ev[11]+WP[2]*ev[14]+zeta2*(ev[1]-rz*ev[2]);
    ev[30]=PA[0]*ev[10]+WP[0]*ev[13];
    ev[31]=PA[0]*ev[11]+WP[0]*ev[14];
    ev[32]=PA[1]*ev[11]+WP[1]*ev[14];
    ev[33]=PA[0]*ev[12]+WP[0]*ev[15]+zeta2*(ev[2]-rz*ev[3]);
    ev[34]=PA[1]*ev[13]+WP[1]*ev[16]+zeta2*(ev[2]-rz*ev[3]);
    ev[35]=PA[2]*ev[14]+WP[2]*ev[17]+zeta2*(ev[2]-rz*ev[3]);
    ev[36]=PA[0]*ev[13]+WP[0]*ev[16];
    ev[37]=PA[0]*ev[14]+WP[0]*ev[17];
    ev[38]=PA[1]*ev[14]+WP[1]*ev[17];
    ev[39]=PA[0]*ev[15]+WP[0]*ev[18]+zeta2*(ev[3]-rz*ev[4]);
    ev[40]=PA[1]*ev[16]+WP[1]*ev[19]+zeta2*(ev[3]-rz*ev[4]);
    ev[41]=PA[2]*ev[17]+WP[2]*ev[20]+zeta2*(ev[3]-rz*ev[4]);
    ev[42]=PA[0]*ev[16]+WP[0]*ev[19];
    ev[43]=PA[0]*ev[17]+WP[0]*ev[20];
    ev[44]=PA[1]*ev[17]+WP[1]*ev[20];
    // (fs|ss) m=0,2
    ev[45]=PA[0]*ev[21]+WP[0]*ev[27]+2.e0*zeta2*(ev[ 6]-rz*ev[ 9]);
    ev[46]=PA[1]*ev[22]+WP[1]*ev[28]+2.e0*zeta2*(ev[ 7]-rz*ev[10]);
    ev[47]=PA[2]*ev[23]+WP[2]*ev[29]+2.e0*zeta2*(ev[ 8]-rz*ev[11]);
    ev[48]=PA[1]*ev[21]+WP[1]*ev[27];
    ev[49]=PA[2]*ev[21]+WP[2]*ev[27];
    ev[50]=PA[0]*ev[22]+WP[0]*ev[28];
    ev[51]=PA[0]*ev[23]+WP[0]*ev[29];
    ev[52]=PA[0]*ev[26]+WP[0]*ev[32];
    ev[53]=PA[2]*ev[22]+WP[2]*ev[28];
    ev[54]=PA[1]*ev[23]+WP[1]*ev[29];
    ev[55]=PA[0]*ev[27]+WP[0]*ev[33]+2.e0*zeta2*(ev[ 9]-rz*ev[12]);
    ev[56]=PA[1]*ev[28]+WP[1]*ev[34]+2.e0*zeta2*(ev[10]-rz*ev[13]);
    ev[57]=PA[2]*ev[29]+WP[2]*ev[35]+2.e0*zeta2*(ev[11]-rz*ev[14]);
    ev[58]=PA[1]*ev[27]+WP[1]*ev[33];
    ev[59]=PA[2]*ev[27]+WP[2]*ev[33];
    ev[60]=PA[0]*ev[28]+WP[0]*ev[34];
    ev[61]=PA[0]*ev[29]+WP[0]*ev[35];
    ev[62]=PA[0]*ev[32]+WP[0]*ev[38];
    ev[63]=PA[2]*ev[28]+WP[2]*ev[34];
    ev[64]=PA[1]*ev[29]+WP[1]*ev[35];
    ev[65]=PA[0]*ev[33]+WP[0]*ev[39]+2.e0*zeta2*(ev[12]-rz*ev[15]);
    ev[66]=PA[1]*ev[34]+WP[1]*ev[40]+2.e0*zeta2*(ev[13]-rz*ev[16]);
    ev[67]=PA[2]*ev[35]+WP[2]*ev[41]+2.e0*zeta2*(ev[14]-rz*ev[17]);
    ev[68]=PA[1]*ev[33]+WP[1]*ev[39];
    ev[69]=PA[2]*ev[33]+WP[2]*ev[39];
    ev[70]=PA[0]*ev[34]+WP[0]*ev[40];
    ev[71]=PA[0]*ev[35]+WP[0]*ev[41];
    ev[72]=PA[0]*ev[38]+WP[0]*ev[44];
    ev[73]=PA[2]*ev[34]+WP[2]*ev[40];
    ev[74]=PA[1]*ev[35]+WP[1]*ev[41];
    // (gs|ss) m=0,1
    ev[ 75]=PA[0]*ev[45]+WP[0]*ev[55]+3.e0*zeta2*(ev[21]-rz*ev[27]);
    ev[ 76]=PA[1]*ev[46]+WP[1]*ev[56]+3.e0*zeta2*(ev[22]-rz*ev[28]);
    ev[ 77]=PA[2]*ev[47]+WP[2]*ev[57]+3.e0*zeta2*(ev[23]-rz*ev[29]);
    ev[ 78]=PA[1]*ev[45]+WP[1]*ev[55];
    ev[ 79]=PA[2]*ev[45]+WP[2]*ev[55];
    ev[ 80]=PA[0]*ev[50]+WP[0]*ev[60]+     zeta2*(ev[22]-rz*ev[28]);
    ev[ 81]=PA[0]*ev[51]+WP[0]*ev[61]+     zeta2*(ev[23]-rz*ev[29]);
    ev[ 82]=PA[1]*ev[49]+WP[1]*ev[59];
    ev[ 83]=PA[0]*ev[46]+WP[0]*ev[56];
    ev[ 84]=PA[0]*ev[47]+WP[0]*ev[57];
    ev[ 85]=PA[0]*ev[53]+WP[0]*ev[63];
    ev[ 86]=PA[0]*ev[54]+WP[0]*ev[64];
    ev[ 87]=PA[2]*ev[46]+WP[2]*ev[56];
    ev[ 88]=PA[1]*ev[47]+WP[1]*ev[57];
    ev[ 89]=PA[1]*ev[54]+WP[1]*ev[64]+     zeta2*(ev[23]-rz*ev[29]);
    ev[ 90]=PA[0]*ev[55]+WP[0]*ev[65]+3.e0*zeta2*(ev[27]-rz*ev[33]);
    ev[ 91]=PA[1]*ev[56]+WP[1]*ev[66]+3.e0*zeta2*(ev[28]-rz*ev[34]);
    ev[ 92]=PA[2]*ev[57]+WP[2]*ev[67]+3.e0*zeta2*(ev[29]-rz*ev[35]);
    ev[ 93]=PA[1]*ev[55]+WP[1]*ev[65];
    ev[ 94]=PA[2]*ev[55]+WP[2]*ev[65];
    ev[ 95]=PA[0]*ev[60]+WP[0]*ev[70]+     zeta2*(ev[28]-rz*ev[34]);
    ev[ 96]=PA[0]*ev[61]+WP[0]*ev[71]+     zeta2*(ev[29]-rz*ev[35]);
    ev[ 97]=PA[1]*ev[59]+WP[1]*ev[69];
    ev[ 98]=PA[0]*ev[56]+WP[0]*ev[66];
    ev[ 99]=PA[0]*ev[57]+WP[0]*ev[67];
    ev[100]=PA[0]*ev[63]+WP[0]*ev[73];
    ev[101]=PA[0]*ev[64]+WP[0]*ev[74];
    ev[102]=PA[2]*ev[56]+WP[2]*ev[66];
    ev[103]=PA[1]*ev[57]+WP[1]*ev[67];
    ev[104]=PA[1]*ev[64]+WP[1]*ev[74]+     zeta2*(ev[29]-rz*ev[35]);
    // (ds|ps) m=[0,0]
    ev[105]=QC[0]*ev[21]+WQ[0]*ev[27]+2.e0*ze2*ev[ 9];
    ev[106]=QC[1]*ev[21]+WQ[1]*ev[27];
    ev[107]=QC[2]*ev[21]+WQ[2]*ev[27];
    ev[108]=QC[0]*ev[22]+WQ[0]*ev[28];
    ev[109]=QC[1]*ev[22]+WQ[1]*ev[28]+2.e0*ze2*ev[10];
    ev[110]=QC[2]*ev[22]+WQ[2]*ev[28];
    ev[111]=QC[0]*ev[23]+WQ[0]*ev[29];
    ev[112]=QC[1]*ev[23]+WQ[1]*ev[29];
    ev[113]=QC[2]*ev[23]+WQ[2]*ev[29]+2.e0*ze2*ev[11];
    ev[114]=QC[0]*ev[24]+WQ[0]*ev[30]+     ze2*ev[10];
    ev[115]=QC[1]*ev[24]+WQ[1]*ev[30]+     ze2*ev[ 9];
    ev[116]=QC[2]*ev[24]+WQ[2]*ev[30];
    ev[117]=QC[0]*ev[25]+WQ[0]*ev[31]+     ze2*ev[11];
    ev[118]=QC[1]*ev[25]+WQ[1]*ev[31];
    ev[119]=QC[2]*ev[25]+WQ[2]*ev[31]+     ze2*ev[ 9];
    ev[120]=QC[0]*ev[26]+WQ[0]*ev[32];
    ev[121]=QC[1]*ev[26]+WQ[1]*ev[32]+     ze2*ev[11];
    ev[122]=QC[2]*ev[26]+WQ[2]*ev[32]+     ze2*ev[10];
    // (fs|ps) m=[0,0]
    ev[123]=QC[0]*ev[45]+WQ[0]*ev[55]+3.e0*ze2*ev[27];
    ev[124]=QC[1]*ev[45]+WQ[1]*ev[55];
    ev[125]=QC[2]*ev[45]+WQ[2]*ev[55];
    ev[126]=QC[0]*ev[46]+WQ[0]*ev[56];
    ev[127]=QC[1]*ev[46]+WQ[1]*ev[56]+3.e0*ze2*ev[28];
    ev[128]=QC[2]*ev[46]+WQ[2]*ev[56];
    ev[129]=QC[0]*ev[47]+WQ[0]*ev[57];
    ev[130]=QC[1]*ev[47]+WQ[1]*ev[57];
    ev[131]=QC[2]*ev[47]+WQ[2]*ev[57]+3.e0*ze2*ev[29];
    ev[132]=QC[0]*ev[48]+WQ[0]*ev[58]+2.e0*ze2*ev[30];
    ev[133]=QC[1]*ev[48]+WQ[1]*ev[58]+     ze2*ev[27];
    ev[134]=QC[2]*ev[48]+WQ[2]*ev[58];
    ev[135]=QC[0]*ev[49]+WQ[0]*ev[59]+2.e0*ze2*ev[31];
    ev[136]=QC[1]*ev[49]+WQ[1]*ev[59];
    ev[137]=QC[2]*ev[49]+WQ[2]*ev[59]+     ze2*ev[27];
    ev[138]=QC[0]*ev[50]+WQ[0]*ev[60]+     ze2*ev[28];
    ev[139]=QC[1]*ev[50]+WQ[1]*ev[60]+2.e0*ze2*ev[30];
    ev[140]=QC[2]*ev[50]+WQ[2]*ev[60];
    ev[141]=QC[0]*ev[51]+WQ[0]*ev[61]+     ze2*ev[29];
    ev[142]=QC[1]*ev[51]+WQ[1]*ev[61];
    ev[143]=QC[2]*ev[51]+WQ[2]*ev[61]+2.e0*ze2*ev[31];
    ev[144]=QC[0]*ev[52]+WQ[0]*ev[62]+     ze2*ev[32];
    ev[145]=QC[1]*ev[52]+WQ[1]*ev[62]+     ze2*ev[31];
    ev[146]=QC[2]*ev[52]+WQ[2]*ev[62]+     ze2*ev[30];
    ev[147]=QC[0]*ev[53]+WQ[0]*ev[63];
    ev[148]=QC[1]*ev[53]+WQ[1]*ev[63]+2.e0*ze2*ev[32];
    ev[149]=QC[2]*ev[53]+WQ[2]*ev[63]+     ze2*ev[28];
    ev[150]=QC[0]*ev[54]+WQ[0]*ev[64];
    ev[151]=QC[1]*ev[54]+WQ[1]*ev[64]+     ze2*ev[29];
    ev[152]=QC[2]*ev[54]+WQ[2]*ev[64]+2.e0*ze2*ev[32];
    // (gs|ps) m=[0,0]
    ev[153]=QC[0]*ev[75]+WQ[0]*ev[ 90]+4.e0*ze2*ev[55];
    ev[154]=QC[1]*ev[75]+WQ[1]*ev[ 90];
    ev[155]=QC[2]*ev[75]+WQ[2]*ev[ 90];
    ev[156]=QC[0]*ev[76]+WQ[0]*ev[ 91];
    ev[157]=QC[1]*ev[76]+WQ[1]*ev[ 91]+4.e0*ze2*ev[56];
    ev[158]=QC[2]*ev[76]+WQ[2]*ev[ 91];
    ev[159]=QC[0]*ev[77]+WQ[0]*ev[ 92];
    ev[160]=QC[1]*ev[77]+WQ[1]*ev[ 92];
    ev[161]=QC[2]*ev[77]+WQ[2]*ev[ 92]+4.e0*ze2*ev[57];
    ev[162]=QC[0]*ev[78]+WQ[0]*ev[ 93]+3.e0*ze2*ev[58];
    ev[163]=QC[1]*ev[78]+WQ[1]*ev[ 93]+     ze2*ev[55];
    ev[164]=QC[2]*ev[78]+WQ[2]*ev[ 93];
    ev[165]=QC[0]*ev[79]+WQ[0]*ev[ 94]+3.e0*ze2*ev[59];
    ev[166]=QC[1]*ev[79]+WQ[1]*ev[ 94];
    ev[167]=QC[2]*ev[79]+WQ[2]*ev[ 94]+     ze2*ev[55];
    ev[168]=QC[0]*ev[80]+WQ[0]*ev[ 95]+2.e0*ze2*ev[60];
    ev[169]=QC[1]*ev[80]+WQ[1]*ev[ 95]+2.e0*ze2*ev[58];
    ev[170]=QC[2]*ev[80]+WQ[2]*ev[ 95];
    ev[171]=QC[0]*ev[81]+WQ[0]*ev[ 96]+2.e0*ze2*ev[61];
    ev[172]=QC[1]*ev[81]+WQ[1]*ev[ 96];
    ev[173]=QC[2]*ev[81]+WQ[2]*ev[ 96]+2.e0*ze2*ev[59];
    ev[174]=QC[0]*ev[82]+WQ[0]*ev[ 97]+2.e0*ze2*ev[62];
    ev[175]=QC[1]*ev[82]+WQ[1]*ev[ 97]+     ze2*ev[59];
    ev[176]=QC[2]*ev[82]+WQ[2]*ev[ 97]+     ze2*ev[58];
    ev[177]=QC[0]*ev[83]+WQ[0]*ev[ 98]+     ze2*ev[56];
    ev[178]=QC[1]*ev[83]+WQ[1]*ev[ 98]+3.e0*ze2*ev[60];
    ev[179]=QC[2]*ev[83]+WQ[2]*ev[ 98];
    ev[180]=QC[0]*ev[84]+WQ[0]*ev[ 99]+     ze2*ev[57];
    ev[181]=QC[1]*ev[84]+WQ[1]*ev[ 99];
    ev[182]=QC[2]*ev[84]+WQ[2]*ev[ 99]+3.e0*ze2*ev[61];
    ev[183]=QC[0]*ev[85]+WQ[0]*ev[100]+     ze2*ev[63];
    ev[184]=QC[1]*ev[85]+WQ[1]*ev[100]+2.e0*ze2*ev[62];
    ev[185]=QC[2]*ev[85]+WQ[2]*ev[100]+     ze2*ev[60];
    ev[186]=QC[0]*ev[86]+WQ[0]*ev[101]+     ze2*ev[64];
    ev[187]=QC[1]*ev[86]+WQ[1]*ev[101]+     ze2*ev[61];
    ev[188]=QC[2]*ev[86]+WQ[2]*ev[101]+2.e0*ze2*ev[62];
    ev[189]=QC[0]*ev[87]+WQ[0]*ev[102];
    ev[190]=QC[1]*ev[87]+WQ[1]*ev[102]+3.e0*ze2*ev[63];
    ev[191]=QC[2]*ev[87]+WQ[2]*ev[102]+     ze2*ev[56];
    ev[192]=QC[0]*ev[88]+WQ[0]*ev[103];
    ev[193]=QC[1]*ev[88]+WQ[1]*ev[103]+     ze2*ev[57];
    ev[194]=QC[2]*ev[88]+WQ[2]*ev[103]+3.e0*ze2*ev[64];
    ev[195]=QC[0]*ev[89]+WQ[0]*ev[104];
    ev[196]=QC[1]*ev[89]+WQ[1]*ev[104]+2.e0*ze2*ev[64];
    ev[197]=QC[2]*ev[89]+WQ[2]*ev[104]+2.e0*ze2*ev[63];
}

static void ofmo_vrr_cint_ddps( const double *ev, double *eh ) {
    int La=2, Lb=2, Lc=1, Ld=0;
    int i, ih, iv;
    // (DS|PS)
    for ( i=0, iv=105, ih=0; i<18; i++, iv++, ih++ ) eh[ih]+=ev[iv];
    // (FS|PS)
    for ( i=0, iv=123, ih=18; i<30; i++, iv++, ih++ ) eh[ih]+=ev[iv];
    // (GS|PS)
    for ( i=0, iv=153, ih=48; i<45; i++, iv++, ih++ ) eh[ih]+=ev[iv];
}

void ofmo_twoint_core_os_ddps(
        const int *pLa, const int *pLb, const int *pLc, const int *pLd,
        const int *nijps, const double vzeta[], const double vdkab[],
        const double vxiza[], const double BA[3],
        const int *nklps, const double veta[], const double vdkcd[],
        const double vxizc[], const double DC[3], const double AC[3],
        double *DINT ) {
    int ijps, klps, i;
    double cssss, zeta, dkab, xiza, eta, xizc, dk, T;
    double zeta2, eta2, ze2, rz, re, PA[3], WP[3], QC[3], WQ[3];
    double PQ2, sqrho, rho, PC[3], QP[3];
    double ev[198], eh[345];
    int La=*pLa, Lb=*pLb, Lc=*pLc, Ld=*pLd;

    DFACT = ofmo_getadd_dfact();
    ofmo_hrr_clear_ddps( eh );
    for ( ijps=0; ijps<(*nijps); ijps++ ) {
        zeta  = vzeta[ijps];
        dkab  = vdkab[ijps];
        xiza  = vxiza[ijps];
        zeta2 = HALF * zeta;
        for ( i=0; i<3; i++ ) {
            PC[i] = AC[i] + xiza*BA[i];
            PA[i] = xiza * BA[i];
        }
        for ( klps=0; klps<(*nklps); klps++ ) {
            eta  = veta[klps];
            dk   = dkab * vdkcd[klps];
            xizc = vxizc[klps];
            PQ2  = ZERO;
            for ( i=0; i<3; i++ ) {
                QC[i] = xizc*DC[i];
                QP[i] = xizc*DC[i] - PC[i];
                PQ2  += QP[i]*QP[i];
            }
            sqrho = sqrt(1.e0/(zeta+eta));
            rho   = sqrho*sqrho;
            rz    = rho * zeta;
            ze2 = rz * eta * HALF;
            for ( i=0; i<3; i++ ) {
                WP[i] = rz*QP[i];
                WQ[i] = rz*QP[i] - QP[i];
            }
            T     = rho * PQ2;
            cssss = sqrho * dk;
            ofmo_vrr_calc_ddps(
                    T, cssss, zeta2, eta2, ze2, rz, re, PA, WP, QC, WQ,
                    ev );
            ofmo_vrr_cint_ddps( ev, eh );
        }	// for (klps)
    }	// for (ijps)
    ofmo_hrr_calc_ddps( BA, DC, eh );
    ofmo_hrr_coef_ddps( eh, DINT );
}

int ofmo_twoint_os_ddps(
        const int *pnworkers, const int *pworkerid,
        const int *pLa, const int *pLb, const int *pLc, const int *pLd,
        const int shel_atm[], const int shel_ini[],
        const double atom_x[], const double atom_y[],
        const double atom_z[], const int leading_cs_pair[],
        const double csp_schwarz[],
        const int csp_ics[], const int csp_jcs[],
        const int csp_leading_ps_pair[],
        const double psp_zeta[], const double psp_dkps[],
        const double psp_xiza[],
        // for partially direct SCF
        const long *pebuf_max_nzeri, long *ebuf_non_zero_eri,
        double ebuf_val[], short int ebuf_ind4[],
        int *last_ijcs, int *last_klcs ) {
    int Lab, Lcd, i, j, k, l, ipat, ix;
    int I2, IJ, K2, KL;
    int ijcs, ijcs0, ijcs1;
    int klcs, klcs0, klcs1, max_klcs;
    int ijps0, nijps, klps0, nklps;
    int ics, iat, iao, iao0, jcs, jat, jao, jao0;
    int kcs, kat, kao, kao0, lcs, lat, lao, lao0;
    double A[3], B[3], C[3], D[3], BA[3], DC[3], AC[3];
    double val_ab, val_cd, coe, coe0;
    double DINTEG[6*6*3*1];
    long nzeri, max_nzeri, nzeri4;
    int nworkers=*pnworkers, workerid=*pworkerid;
    int La=*pLa, Lb=*pLb, Lc=*pLc, Ld=*pLd;
    long ebuf_max_nzeri = *pebuf_max_nzeri;
    int mythread;
    float eps_eri = ofmo_twoint_eps_eri(0);
    float eps_ps4 = ofmo_twoint_eps_ps4(0);

    
    mythread = omp_get_thread_num();
    if ( DFACT == NULL ) DFACT = ofmo_getadd_dfact();
    
    Lab = La*(La+1)/2+Lb;
    Lcd = Lc*(Lc+1)/2+Ld;
    ijcs0 = leading_cs_pair[Lab];
    ijcs1 = leading_cs_pair[Lab+1];
    klcs0 = leading_cs_pair[Lcd];
    klcs1 = leading_cs_pair[Lcd+1];
    nzeri     = *ebuf_non_zero_eri;
    max_nzeri = ebuf_max_nzeri - 6*6*3*1;
    nzeri4    = nzeri*4;
    if ( nzeri >= max_nzeri ) {
        *last_ijcs = ijcs0+workerid;
        *last_klcs = klcs0 - 1;
        *ebuf_non_zero_eri = nzeri;
        return OFMO_EBUF_FULL;
    }

    for ( ijcs=ijcs0+workerid; ijcs<ijcs1; ijcs+=nworkers ) {
        val_ab = csp_schwarz[ijcs];
        ics    = csp_ics[ijcs];
        jcs    = csp_jcs[ijcs];
        ijps0  = csp_leading_ps_pair[ijcs];
        nijps  = csp_leading_ps_pair[ijcs+1]-ijps0;
        iat    = shel_atm[ics];
        jat    = shel_atm[jcs];
        iao0   = shel_ini[ics];
        jao0   = shel_ini[jcs];
        A[0]=atom_x[iat]; A[1]=atom_y[iat]; A[2]=atom_z[iat];
        B[0]=atom_x[jat]; B[1]=atom_y[jat]; B[2]=atom_z[jat];
        for ( i=0; i<3; i++ ) BA[i] = B[i] - A[i];
        max_klcs = ( Lab == Lcd ? ijcs+1 : klcs1 );
        for ( klcs=klcs0; klcs<max_klcs; klcs++ ) {
            val_cd = csp_schwarz[klcs];
            if ( val_ab*val_cd < eps_ps4 ) continue;
            kcs    = csp_ics[klcs];
            lcs    = csp_jcs[klcs];
            klps0  = csp_leading_ps_pair[klcs];
            nklps  = csp_leading_ps_pair[klcs+1]-klps0;
            kat    = shel_atm[kcs];
            lat    = shel_atm[lcs];
            kao0   = shel_ini[kcs];
            lao0   = shel_ini[lcs];
            C[0]=atom_x[kat]; C[1]=atom_y[kat]; C[2]=atom_z[kat];
            D[0]=atom_x[lat]; D[1]=atom_y[lat]; D[2]=atom_z[lat];
            for ( i=0; i<3; i++ ) {
                AC[i] = A[i] - C[i];
                DC[i] = D[i] - C[i];
            }
            ofmo_twoint_core_os_ddps(
                    &La, &Lb, &Lc, &Ld,
                    &nijps, &psp_zeta[ijps0], &psp_dkps[ijps0],
                    &psp_xiza[ijps0], BA,
                    &nklps, &psp_zeta[klps0], &psp_dkps[klps0],
                    &psp_xiza[klps0], DC,   AC,      DINTEG );
            ipat=((Lab != Lcd)||(ics==kcs && jcs>lcs) ? true : false );
            for ( i=0, iao=iao0, ix=0; i<6; i++, iao++ ) {
                I2 = (iao*iao+iao)>>1;
                for ( j=0, jao=jao0; j<6; j++, jao++ ) {
                    if ( jao>iao ) { ix+=3*1; continue; }
                    IJ = I2 + jao;
                    coe0 = ( iao==jao ? HALF : ONE );
                    for ( k=0, kao=kao0; k<3; k++, kao++ ) {
                        K2 = (kao*kao+kao)>>1;
                        for ( l=0, lao=lao0; l<1; l++, lao++, ix++ ) {
                            if ( lao>kao ) continue;
                            if ( fabs(DINTEG[ix]) > eps_eri ) {
                                KL = K2 + lao;
                                if ( IJ >= KL ) {
                                    coe = coe0;
                                    if ( kao==lao ) coe *= HALF;
                                    if ( KL == IJ ) coe *= HALF;
                                    ebuf_val[nzeri]     = coe*DINTEG[ix];
                                    ebuf_ind4[nzeri4+0] = (short int)iao;
                                    ebuf_ind4[nzeri4+1] = (short int)jao;
                                    ebuf_ind4[nzeri4+2] = (short int)kao;
                                    ebuf_ind4[nzeri4+3] = (short int)lao;
                                    nzeri++;
                                    nzeri4+=4;
                                } else if ( ipat ) {
                                    coe = coe0;
                                    if ( kao==lao ) coe*=HALF;
                                    ebuf_val[nzeri]     = coe*DINTEG[ix];
                                    ebuf_ind4[nzeri4+0] = (short int)kao;
                                    ebuf_ind4[nzeri4+1] = (short int)lao;
                                    ebuf_ind4[nzeri4+2] = (short int)iao;
                                    ebuf_ind4[nzeri4+3] = (short int)jao;
                                    nzeri++;
                                    nzeri4+=4;
                                }
                            }
                        }	// l
                    }	// k
                }	// j
            }	// i
            if ( nzeri >= max_nzeri ) {
                *last_ijcs = ijcs;
                *last_klcs = klcs;
                *ebuf_non_zero_eri = nzeri;
                return OFMO_EBUF_FULL;
            }
        }	// for (klcs)
    }	// for (ijcs)
    *ebuf_non_zero_eri = nzeri;
    return OFMO_EBUF_NOFULL;
}

int ofmo_twoint_direct_os_ddps(
        const int *pnworkers, const int *pworkerid,
        const int *pLa, const int *pLb, const int *pLc, const int *pLd,
        const int shel_atm[], const int shel_ini[],
        const double atom_x[], const double atom_y[],
        const double atom_z[], const int leading_cs_pair[],
        const double csp_schwarz[],
        const int csp_ics[], const int csp_jcs[],
        const int csp_leading_ps_pair[],
        const double psp_zeta[], const double psp_dkps[],
        const double psp_xiza[],
        // for direct SCF
        const long *petmp_max_nzeri, long *petmp_non_zero_eri,
        double etmp_val[], short int etmp_ind4[],
        const int *plast_ijcs, const int *plast_klcs,
        // density matrix & G-matrix data
        const int *pnao, const double Ds[], double G[] ) {
    int nworkers=*pnworkers, workerid=*pworkerid;
    int La=*pLa, Lb=*pLb, Lc=*pLc, Ld=*pLd;
    int last_ijcs=*plast_ijcs, last_klcs=*plast_klcs, nao=*pnao;
    long max_nzeri=*petmp_max_nzeri;
    long nzeri4, nzeri=*petmp_non_zero_eri;
    //
    int Lab, Lcd, i, j, k, l, ipat, ix;
    int I2, IJ, K2, KL;
    int ijcs, ijcs0, ijcs1;
    int klcs, klcs0, klcs1, max_klcs;
    int ijps0, nijps, klps0, nklps;
    int ics, iat, iao, iao0, jcs, jat, jao, jao0;
    int kcs, kat, kao, kao0, lcs, lat, lao, lao0;
    double A[3], B[3], C[3], D[3], BA[3], DC[3], AC[3];
    double val_ab, val_cd, coe, coe0;
    double DINTEG[6*6*3*1];
    int mythread;
    float eps_eri = ofmo_twoint_eps_eri(0);
    float eps_ps4 = ofmo_twoint_eps_ps4(0);
    float eps_sch = ofmo_twoint_eps_sch(0);

    mythread = omp_get_thread_num();
    if ( DFACT == NULL ) DFACT = ofmo_getadd_dfact();
    
    Lab = La*(La+1)/2+Lb;
    Lcd = Lc*(Lc+1)/2+Ld;
    ijcs1 = leading_cs_pair[Lab+1];
    klcs0 = leading_cs_pair[Lcd];
    klcs1 = leading_cs_pair[Lcd+1];
    if ( last_ijcs != -1 ) { 
        ijcs = last_ijcs;
        klcs = last_klcs+1;
    } else {
        ijcs = leading_cs_pair[Lab] + workerid;
        klcs = klcs0;
    }
    
    max_nzeri -= 6*6*3*1;
    nzeri4    = nzeri*4;
    if ( nzeri >= max_nzeri ) {
        ofmo_integ_add_fock( nao, nzeri, etmp_val, etmp_ind4, Ds, G );
        nzeri = nzeri4 = 0;
    }
    
    for ( ; ijcs<ijcs1; ijcs+=nworkers ) {
        val_ab = csp_schwarz[ijcs];
        ics    = csp_ics[ijcs];
        jcs    = csp_jcs[ijcs];
        ijps0  = csp_leading_ps_pair[ijcs];
        nijps  = csp_leading_ps_pair[ijcs+1]-ijps0;
        iat    = shel_atm[ics];
        jat    = shel_atm[jcs];
        iao0   = shel_ini[ics];
        jao0   = shel_ini[jcs];
        A[0]=atom_x[iat]; A[1]=atom_y[iat]; A[2]=atom_z[iat];
        B[0]=atom_x[jat]; B[1]=atom_y[jat]; B[2]=atom_z[jat];
        for ( i=0; i<3; i++ ) BA[i] = B[i] - A[i];
        max_klcs = ( Lab == Lcd ? ijcs+1 : klcs1 );
        for ( ; klcs<max_klcs; klcs++ ) {
            val_cd = csp_schwarz[klcs];
            if ( val_ab*val_cd < eps_ps4 ) continue;
            kcs    = csp_ics[klcs];
            lcs    = csp_jcs[klcs];
            if ( val_ab*val_cd*ofmo_twoint_dmax6(ics,jcs,kcs,lcs) < eps_sch ) continue;
            klps0  = csp_leading_ps_pair[klcs];
            nklps  = csp_leading_ps_pair[klcs+1]-klps0;
            kat    = shel_atm[kcs];
            lat    = shel_atm[lcs];
            kao0   = shel_ini[kcs];
            lao0   = shel_ini[lcs];
            C[0]=atom_x[kat]; C[1]=atom_y[kat]; C[2]=atom_z[kat];
            D[0]=atom_x[lat]; D[1]=atom_y[lat]; D[2]=atom_z[lat];
            for ( i=0; i<3; i++ ) {
                AC[i] = A[i] - C[i];
                DC[i] = D[i] - C[i];
            }
            ofmo_twoint_core_os_ddps(
                    &La, &Lb, &Lc, &Ld,
                    &nijps, &psp_zeta[ijps0], &psp_dkps[ijps0],
                    &psp_xiza[ijps0], BA,
                    &nklps, &psp_zeta[klps0], &psp_dkps[klps0],
                    &psp_xiza[klps0], DC,   AC,      DINTEG );
            ipat=((Lab != Lcd)||(ics==kcs && jcs>lcs) ? true : false );
            for ( i=0, iao=iao0, ix=0; i<6; i++, iao++ ) {
                I2 = (iao*iao+iao)>>1;
                for ( j=0, jao=jao0; j<6; j++, jao++ ) {
                    if ( jao>iao ) { ix+=3*1; continue; }
                    IJ = I2 + jao;
                    coe0 = ( iao==jao ? HALF : ONE );
                    for ( k=0, kao=kao0; k<3; k++, kao++ ) {
                        K2 = (kao*kao+kao)>>1;
                        for ( l=0, lao=lao0; l<1; l++, lao++, ix++ ) {
                            if ( lao>kao ) continue;
                            if ( fabs(DINTEG[ix]) > eps_eri ) {
                                KL = K2 + lao;
                                if ( IJ >= KL ) {
                                    coe = coe0;
                                    if ( kao==lao ) coe *= HALF;
                                    if ( KL == IJ ) coe *= HALF;
                                    etmp_val[nzeri]     = coe*DINTEG[ix];
                                    etmp_ind4[nzeri4+0] = (short int)iao;
                                    etmp_ind4[nzeri4+1] = (short int)jao;
                                    etmp_ind4[nzeri4+2] = (short int)kao;
                                    etmp_ind4[nzeri4+3] = (short int)lao;
                                    nzeri++;
                                    nzeri4+=4;
                                } else if ( ipat ) {
                                    coe = coe0;
                                    if ( kao==lao ) coe*=HALF;
                                    etmp_val[nzeri]     = coe*DINTEG[ix];
                                    etmp_ind4[nzeri4+0] = (short int)kao;
                                    etmp_ind4[nzeri4+1] = (short int)lao;
                                    etmp_ind4[nzeri4+2] = (short int)iao;
                                    etmp_ind4[nzeri4+3] = (short int)jao;
                                    nzeri++;
                                    nzeri4+=4;
                                }
                            }
                        }	// l
                    }	// k
                }	// j
            }	// i
            if ( nzeri >= max_nzeri ) {
                ofmo_integ_add_fock( nao, nzeri, etmp_val, etmp_ind4,
                        Ds, G );
                nzeri = nzeri4= 0;
            }
        }	// for (klcs)
        klcs = klcs0;
    }	// for (ijcs)
    *petmp_non_zero_eri = nzeri;
    return 0;
}
