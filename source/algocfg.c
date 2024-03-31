/*
 * SMART: string matching algorithms research tool.
 * Copyright (C) 2024 Reini Urban
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sets.h"

/*
 * This program is used for the build-system to provide proper cmdline args per algo.
 * For now for make verify. make sanitize asan.lst maybe also.
 * cbmc can be much faster if given the proper depth and unwind args. best no depth at all
 * for sound proofs, which should terminate.
 */

enum verify_status {
  VFY_PASS,
  UNSATISFIABLE, // but VERIFICATION SUCCESSFUL
  VFY_FAIL,
  VFY_TIMEOUT,
};

/* UNSATIFIABLE means cbmc it ran into --depth limit. redo with higher --depth,
   maybe enlarge the timeout also. or use the builtin memcmp/memset with CBMC.
 */

struct algocfg {
  const enum algo_id id;
  const int good; // passes all tests
  const int asan; // passes asan tests
  const enum verify_status vfy;
  const int minlen;
  const int maxlen;
  const int depth;
  const int unwind;
};

#define GOOD 1
#define ASAN 1
#define FAIL 0

const struct algocfg ALGOCFGS[] = {
    // clang-format off
  // Comparison based Algorithms
  [_BF] = {_BF, GOOD, ASAN, UNSATISFIABLE, 0, 0, 0, 256}, // 7m with 1024 but still UNSAT,
          // needs memcmp for SAT as in main.h
  [_MP] = {_MP, GOOD, ASAN, VFY_PASS, 0, 0, 512, 0}, // 3m
  [_KMP] = {_KMP, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_BM] = {_BM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_HOR] = {_HOR, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_GS] = {_GS, FAIL, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_AG] = {_AG, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_KR] = {_KR, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_ZT] = {_ZT, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_AC] = {_AC, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_TW] = {_TW, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_OM] = {_OM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_MS] = {_MS, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_QS] = {_QS, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TunBM] = {_TunBM, FAIL, FAIL, UNSATISFIABLE, 0, 0, 256, 256},
  [_NSN] = {_NSN, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_TBM] = {_TBM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_Colussi] = {_Colussi, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_Smith] = {_Smith, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_GG] = {_GG, FAIL, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_Raita] = {_Raita, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_SMOA] = {_SMOA, GOOD, ASAN, VFY_FAIL, 0, 0, 512, 0},
  [_RColussi] = {_RColussi, FAIL, FAIL, VFY_PASS, 0, 0, 256, 256},
  [_Skip] = {_Skip, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_KMPSkip] = {_KMPSkip, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_ASkip] = {_ASkip, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_BR] = {_BR, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_AKC] = {_AKC, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_FS] = {_FS, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_FFS] = {_FFS, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_BFS] = {_BFS, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_TS] = {_TS, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SSABS] = {_SSABS, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_TVSBS] = {_TVSBS, GOOD, FAIL, VFY_PASS, 0, 0, 256, 256},
  [_PBMH] = {_PBMH, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_FJS] = {_FJS, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_BLOCK] = {_BLOCK, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_HASH3] = {_HASH3, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_HASH5] = {_HASH5, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_HASH8] = {_HASH8, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_TSW] = {_TSW, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256}, // m < n - 3
  //[_BMH2] = {_BMH2, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  //[_BMH4] = {_BMH4, GOOD, ASAN, VFY_PASS, 4, 0, 256, 256},
  [_GRASPm] = {_GRASPm, FAIL, FAIL, UNSATISFIABLE, 0, 0, 256, 256},
  [_SSEF] = {_SSEF, X64_ONLY, FAIL, UNSATISFIABLE, 32, 0, 256, 256},
  [_AUT] = {_AUT, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_RF] = {_RF, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TRF] = {_TRF, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_Simon] = {_Simon, FAIL, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_FDM] = {_FDM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_BOM] = {_BOM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_BOM2] = {_BOM2, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_DFDM] = {_DFDM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_WW] = {_WW, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_LDM] = {_LDM, FAIL, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_ILDM1] = {_ILDM1, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_ILDM2] = {_ILDM2, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_EBOM] = {_EBOM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_FBOM] = {_FBOM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_SEBOM] = {_SEBOM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_SFBOM] = {_SFBOM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  //[_SBDM] = {_SBDM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  //[_BSOM] = {_BSOM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SKIP2] = {_SKIP2, GOOD, ASAN, VFY_TIMEOUT, 2, 0, 256, 256},
  [_SKIP3] = {_SKIP3, GOOD, ASAN, VFY_TIMEOUT, 3, 0, 256, 256},
  [_SKIP4] = {_SKIP4, GOOD, ASAN, VFY_TIMEOUT, 4, 0, 256, 256},
  [_SKIP5] = {_SKIP5, GOOD, ASAN, UNSATISFIABLE, 5, 0, 256, 256},
  [_SKIP6] = {_SKIP6, GOOD, ASAN, UNSATISFIABLE, 6, 0, 256, 256},
  [_SKIP7] = {_SKIP7, GOOD, ASAN, UNSATISFIABLE, 7, 0, 256, 256},
  [_SKIP8] = {_SKIP8, GOOD, ASAN, UNSATISFIABLE, 8, 0, 256, 256},
  [_SO]    = {_SO, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_SA]    = {_SA, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_BNDM]  = {_BNDM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_BNDML] = {_BNDML, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_SBNDM] = {_SBNDM, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_TNDM]  = {_TNDM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_TNDMa] = {_TNDMa, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LBNDM] = {_LBNDM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_SVM0]  = {_SVM0, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_SVM1]  = {_SVM1, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SVM2]  = {_SVM2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SVM3]  = {_SVM3, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_SVM4]  = {_SVM4, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_SBNDM2] = {_SBNDM2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SBNDM_BMH] = {_SBNDM_BMH, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_BMH_SBNDM] = {_BMH_SBNDM, FAIL, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_FNDM]  = {_FNDM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_BWW]   = {_BWW, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_FAOSO2] = {_FAOSO2, GOOD, ASAN, VFY_FAIL, 3, 0, 256, 256},
  [_FAOSO4] = {_FAOSO4, FAIL, FAIL, VFY_FAIL, 5, 0, 256, 256},
  [_FAOSO6] = {_FAOSO6, FAIL, FAIL, VFY_PASS, 7, 0, 256, 256},
  [_AOSO2] = {_AOSO2, GOOD, ASAN, UNSATISFIABLE, 3, 0, 256, 256},
  [_AOSO4] = {_AOSO4, GOOD, ASAN, UNSATISFIABLE, 5, 0, 256, 256},
  [_AOSO6] = {_AOSO6, GOOD, ASAN, UNSATISFIABLE, 7, 0, 256, 256},
  [_BLIM]  = {_BLIM, FAIL, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_FSBNDM] = {_FSBNDM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_BNDMq2] = {_BNDMq2, GOOD, ASAN, UNSATISFIABLE, 2, 0, 256, 256},
  [_BNDMq4] = {_BNDMq4, GOOD, FAIL, UNSATISFIABLE, 4, 0, 256, 256},
  [_BNDMq6] = {_BNDMq6, GOOD, FAIL, UNSATISFIABLE, 6, 0, 256, 256},
  [_SBNDMq2] = {_SBNDMq2, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_SBNDMq4] = {_SBNDMq4, GOOD, ASAN, VFY_PASS, 4, 0, 256, 256},
  [_SBNDMq6] = {_SBNDMq6, GOOD, ASAN, VFY_PASS, 6, 0, 256, 256},
  [_SBNDMq8] = {_SBNDMq8, GOOD, ASAN, VFY_PASS, 8, 0, 256, 256},
  [_UFNDMq2] = {_UFNDMq2, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_UFNDMq4] = {_UFNDMq4, GOOD, ASAN, VFY_FAIL, 4, 0, 256, 256},
  [_UFNDMq6] = {_UFNDMq6, GOOD, ASAN, VFY_FAIL, 6, 0, 256, 256},
  [_UFNDMq8] = {_UFNDMq8, GOOD, ASAN, VFY_FAIL, 8, 0, 256, 256},
  [_SABP]  = {_SABP, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_DBWW]  = {_DBWW, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_DBWW2] = {_DBWW2, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_KSA]   = {_KSA, FAIL, ASAN, VFY_FAIL, 0, 0, 256, 256}, // occ FAIL
  [_KBNDM] = {_KBNDM, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256},
  [_BSDM]  = {_BSDM, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_BSDM2] = {_BSDM2, GOOD, ASAN, VFY_FAIL, 2, 0, 256, 256},
  [_BSDM3] = {_BSDM3, GOOD, ASAN, VFY_TIMEOUT, 3, 0, 256, 256},
  [_BSDM4] = {_BSDM4, FAIL, FAIL, VFY_TIMEOUT, 4, 0, 256, 256},
  [_BSDM5] = {_BSDM5, GOOD, ASAN, VFY_TIMEOUT, 5, 0, 256, 256},
  [_BSDM6] = {_BSDM6, GOOD, ASAN, UNSATISFIABLE, 6, 0, 256, 256},
  [_BSDM7] = {_BSDM7, GOOD, ASAN, VFY_TIMEOUT, 7, 0, 256, 256},
  [_BSDM8] = {_BSDM8, GOOD, ASAN, UNSATISFIABLE, 8, 0, 256, 256},
  [_BXS]  = {_BXS, FAIL, FAIL, VFY_TIMEOUT, 0, 0, 256, 256},
  [_BXS1] = {_BXS1, GOOD, ASAN, VFY_PASS, 1, 0, 256, 256},
  [_BXS2] = {_BXS2, GOOD, ASAN, VFY_PASS, 2, 0, 256, 256},
  [_BXS3] = {_BXS3, GOOD, ASAN, VFY_PASS, 3, 0, 256, 256},
  [_BXS4] = {_BXS4, GOOD, ASAN, VFY_PASS, 4, 0, 256, 256},
  [_BXS6] = {_BXS6, GOOD, ASAN, VFY_PASS, 6, 0, 256, 256},
  [_BXS8] = {_BXS8, GOOD, ASAN, VFY_PASS, 8, 0, 256, 256},
  [_FS_W1] = {_FS_W1, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_FS_W2] = {_FS_W2, FAIL, FAIL, VFY_PASS, 0, 0, 256, 256},
  [_FS_W4] = {_FS_W4, FAIL, FAIL, VFY_FAIL, 0, 0, 256, 256}, // n>=6
  [_FS_W6] = {_FS_W6, GOOD, FAIL, VFY_FAIL, 0, 0, 256, 256}, // n>=8
  [_FS_W8] = {_FS_W8, GOOD, FAIL, VFY_PASS, 0, 0, 256, 256},
  [_FSBNDM_W1] = {_FSBNDM_W1, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_FSBNDM_W2] = {_FSBNDM_W2, RNDCRASH, RNDCRASH, VFY_PASS, 2, 0, 256, 256},
  [_FSBNDM_W4] = {_FSBNDM_W4, RNDCRASH, RNDCRASH, VFY_PASS, 0, 0, 256, 256},
  [_FSBNDM_W6] = {_FSBNDM_W6, RNDCRASH, RNDCRASH, VFY_PASS, 0, 0, 256, 256},
  [_FSBNDM_W8] = {_FSBNDM_W8, GOOD, FAIL, UNSATISFIABLE, 11, 0, 256, 256},
  [_FSBNDMQ20] = {_FSBNDMQ20, GOOD, ASAN, VFY_TIMEOUT, 2, 32, 256, 256},
  [_FSBNDMQ21] = {_FSBNDMQ21, GOOD, ASAN, VFY_TIMEOUT, 2, 31, 256, 256},
  [_FSBNDMQ31] = {_FSBNDMQ31, GOOD, ASAN, VFY_TIMEOUT, 3, 31, 256, 256},
  [_FSBNDMQ32] = {_FSBNDMQ32, FAIL, FAIL, VFY_TIMEOUT, 3, 30, 256, 256},
  [_FSBNDMQ41] = {_FSBNDMQ41, GOOD, ASAN, VFY_TIMEOUT, 4, 31, 256, 256},
  [_FSBNDMQ42] = {_FSBNDMQ42, FAIL, FAIL, VFY_TIMEOUT, 4, 30, 256, 256},
  [_FSBNDMQ43] = {_FSBNDMQ43, FAIL, FAIL, VFY_TIMEOUT, 4, 29, 256, 256},
  [_FSBNDMQ61] = {_FSBNDMQ61, GOOD, ASAN, VFY_TIMEOUT, 4, 0, 256, 256},
  [_FSBNDMQ62] = {_FSBNDMQ62, FAIL, FAIL, VFY_TIMEOUT, 6, 0, 256, 256},
  [_FSBNDMQ64] = {_FSBNDMQ64, FAIL, FAIL, VFY_TIMEOUT, 6, 0, 256, 256},
  [_FSBNDMQ81] = {_FSBNDMQ81, GOOD, ASAN, VFY_TIMEOUT, 8, 0, 256, 256},
  [_FSBNDMQ82] = {_FSBNDMQ82, FAIL, FAIL, VFY_TIMEOUT, 8, 0, 256, 256},
  [_FSBNDMQ84] = {_FSBNDMQ84, FAIL, FAIL, VFY_TIMEOUT, 8, 0, 256, 256},
  [_FSBNDMQ86] = {_FSBNDMQ86, FAIL, FAIL, VFY_TIMEOUT, 8, 0, 256, 256},
  [_IOM]   = {_IOM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_JOM]   = {_JOM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR2] = {_LWFR2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR3] = {_LWFR3, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR4] = {_LWFR4, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR5] = {_LWFR5, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR6] = {_LWFR6, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR7] = {_LWFR7, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_LWFR8] = {_LWFR8, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_QF23] = {_QF23, GOOD, ASAN, VFY_PASS, 3, 0, 256, 256},
  [_QF24] = {_QF24, GOOD, ASAN, VFY_PASS, 3, 0, 256, 256},
  [_QF26] = {_QF26, FAIL, FAIL, VFY_PASS, 3, 0, 256, 256},
  [_QF28] = {_QF28, GOOD, ASAN, VFY_PASS, 3, 0, 256, 256},
  [_QF33] = {_QF33, GOOD, ASAN, VFY_PASS, 4, 0, 256, 256},
  [_QF34] = {_QF34, GOOD, ASAN, VFY_PASS, 4, 0, 256, 256},
  [_QF36] = {_QF36, GOOD, ASAN, VFY_PASS, 4, 0, 256, 256},
  [_QF42] = {_QF42, GOOD, ASAN, VFY_PASS, 5, 0, 256, 256},
  [_QF43] = {_QF43, GOOD, ASAN, VFY_PASS, 5, 0, 256, 256},
  [_QF44] = {_QF44, GOOD, ASAN, VFY_PASS, 5, 0, 256, 256},
  [_QF62] = {_QF62, GOOD, ASAN, VFY_PASS, 7, 0, 256, 256},
  [_QF63] = {_QF63, GOOD, ASAN, VFY_PASS, 7, 0, 256, 256},
  [_QLQS] = {_QLQS, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SBNDM_W2] = {_SBNDM_W2, FAIL, FAIL, VFY_PASS, 2, 0, 256, 256},
  [_SBNDM_W4] = {_SBNDM_W4, FAIL, FAIL, VFY_PASS, 4, 0, 256, 256},
  [_SBNDM_W6] = {_SBNDM_W6, GOOD, FAIL, VFY_PASS, 6, 0, 256, 256},
  [_SSM]    = {_SSM, GOOD, ASAN, UNSATISFIABLE, 0, 0, 256, 256},
  [_TSA]    = {_TSA, FAIL, ASAN, VFY_TIMEOUT, 2, 0, 256, 256},
  [_TSA_Q2] = {_TSA_Q2, FAIL, ASAN, VFY_TIMEOUT, 2, 64, 256, 256},
  [_TSO5]   = {_TSO5, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256},
  [_TUNEDBM] = {_TUNEDBM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TVSBS_W2] = {_TVSBS_W2, GOOD, FAIL, VFY_PASS, 0, 0, 256, 256},
  [_TVSBS_W4] = {_TVSBS_W4, FAIL, FAIL, VFY_PASS, 2, 0, 256, 256},
  [_TVSBS_W6] = {_TVSBS_W6, FAIL, FAIL, VFY_PASS, 2, 0, 256, 256},
  [_TVSBS_W8] = {_TVSBS_W8, FAIL, FAIL, VFY_PASS, 2, 0, 256, 256},
  [_WFR]  = {_WFR, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR2] = {_WFR2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR3] = {_WFR3, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR4] = {_WFR4, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR5] = {_WFR5, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR6] = {_WFR6, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR7] = {_WFR7, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFR8] = {_WFR8, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ2] = {_WFRQ2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ3] = {_WFRQ3, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ4] = {_WFRQ4, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ5] = {_WFRQ5, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ6] = {_WFRQ6, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ7] = {_WFRQ7, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WFRQ8] = {_WFRQ8, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR]  = {_TWFR, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR2] = {_TWFR2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR3] = {_TWFR3, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR4] = {_TWFR4, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR5] = {_TWFR5, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR6] = {_TWFR6, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR7] = {_TWFR7, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFR8] = {_TWFR8, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ2] = {_TWFRQ2, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ3] = {_TWFRQ3, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ4] = {_TWFRQ4, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ5] = {_TWFRQ5, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ6] = {_TWFRQ6, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ7] = {_TWFRQ7, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_TWFRQ8] = {_TWFRQ8, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WC]  = {_WC, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_WOM] = {_WOM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_DOUBLEHASH] = {_DOUBLEHASH, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_FT3]  = {_FT3, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  //[_HPBM] = {_HPBM, GOOD, ASAN, VFY_PASS, 0, 0, 256, 256},
  [_SSECP] = {_SSECP, FAIL, ASAN, VFY_FAIL, 0, 0, 256, 256}, // no cbmc simd support yet
  [_EPSM]  = {_EPSM, GOOD, ASAN, VFY_TIMEOUT, 0, 0, 256, 256}, // no cbmc simd support yet
  [_LIBC]  = {_LIBC, FAIL, FAIL, VFY_FAIL, 0, 0, 256, 256}, // no \0
  [_LIBC1] = {_LIBC1, RNDCRASH, RNDCRASH, VFY_FAIL, 0, 0, 256, 256},
  [_MUSL]  = {_MUSL, GOOD, ASAN, VFY_FAIL, 0, 0, 256, 256}, // no \0
  [_MUSL1] = {_MUSL1, GOOD, RNDCRASH, VFY_PASS, 0, 0, 256, 256},
  [_SIMDKR] = {_SIMDKR, RNDCRASH, RNDCRASH, VFY_FAIL, 0, 0, 256, 256}, // no cbmc simd support yet
    // clang-format on
};

int main(int argc, char **argv) {
  if (argc < 1 || argc > 3) {
    fprintf(stderr, "Usage: ./algocfg [algo] [cfg [value]]\n");
    return 1;
  }
  char *algo = argv[1];
  getAlgo(ALGO_NAME, EXECUTE);
  int id = search_ALGO(ALGO_NAME, algo);
  if (id < 0) { // not an algo, so a cfg
    const char *cfg = algo;
    if (strcmp(cfg, "good") == 0) {
      int value = (argc == 3) ? atoi(argv[2]) : 1;
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].good == value)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "asan") == 0) {
      int value = (argc == 3) ? atoi(argv[2]) : 1;
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].asan == value)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "VFY_PASS") == 0) {
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].vfy == VFY_PASS)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "VFY_FAIL") == 0) {
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].vfy == VFY_FAIL)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "VFY_TIMEOUT") == 0) {
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].vfy == VFY_TIMEOUT)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "UNSATISFIABLE") == 0) {
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].vfy == UNSATISFIABLE)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "depth") == 0) {
      int value = (argc == 3) ? atoi(argv[2]) : 256;
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].depth == value)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "unwind") == 0) {
      int value = (argc == 3) ? atoi(argv[2]) : 256;
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].unwind == value)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "minlen") == 0) {
      int value = (argc == 3) ? atoi(argv[2]) : 0;
      // TODO which op? == < > ...
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].minlen == value)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else if (strcmp(cfg, "maxlen") == 0) {
      int value = (argc == 3) ? atoi(argv[2]) : 0;
      for (unsigned i = 0; i < NumAlgo; i++)
        if (ALGOS[i].id == i && ALGOCFGS[i].maxlen == value)
          printf("%s ", ALGOS[i].name);
      printf("\n");
    } else {
      printf("Unknown cfg or algo %s\n", cfg);
      exit(1);
    }
  } else {
    const char *cfg = argv[2];
    if (strcmp(cfg, "good") == 0)
      printf("%d\n", ALGOCFGS[id].good);
    else if (strcmp(cfg, "asan") == 0)
      printf("%d\n", ALGOCFGS[id].asan);
    else if (strcmp(cfg, "vfy") == 0)
      printf("%s\n", ALGOCFGS[id].vfy == VFY_PASS        ? "VFY_PASS"
                     : ALGOCFGS[id].vfy == VFY_FAIL      ? "VFY_FAIL"
                     : ALGOCFGS[id].vfy == VFY_TIMEOUT   ? "VFY_TIMEOUT"
                     : ALGOCFGS[id].vfy == UNSATISFIABLE ? "UNSATISFIABLE"
                                                         : "?");
    else if (strcmp(cfg, "VFY_PASS") == 0)
      printf("%d\n", ALGOCFGS[id].vfy == VFY_PASS);
    else if (strcmp(cfg, "VFY_FAIL") == 0)
      printf("%d\n", ALGOCFGS[id].vfy == VFY_FAIL);
    else if (strcmp(cfg, "VFY_TIMEOUT") == 0)
      printf("%d\n", ALGOCFGS[id].vfy == VFY_TIMEOUT);
    else if (strcmp(cfg, "UNSATISFIABLE") == 0)
      printf("%d\n", ALGOCFGS[id].vfy == UNSATISFIABLE);
    else if (strcmp(cfg, "depth") == 0)
      printf("%d\n", ALGOCFGS[id].depth);
    else if (strcmp(cfg, "unwind") == 0)
      printf("%d\n", ALGOCFGS[id].unwind);
    else if (strcmp(cfg, "cbmc") == 0) {
      if (ALGOCFGS[id].depth)
        printf("--depth %d ", ALGOCFGS[id].depth);
      if (ALGOCFGS[id].unwind)
        printf("--unwind %d ", ALGOCFGS[id].unwind);
      printf("\n");
    }
    else if (strcmp(cfg, "minlen") == 0)
      printf("%d\n", ALGOCFGS[id].minlen);
    else if (strcmp(cfg, "maxlen") == 0)
      printf("%d\n", ALGOCFGS[id].maxlen);
    else {
      printf("Unknown cfg %s\n", cfg);
      exit(1);
    }
    return 0;
  }
}
