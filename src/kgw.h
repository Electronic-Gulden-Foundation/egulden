// Copyright (c) 2015-2015 The e-Gulden developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef EGULDEN_KGW_H
#define EGULDEN_KGW_H

#include <stdint.h>

class CBlockIndex;

unsigned int KimotoGravityWell(const CBlockIndex* pindexLast, uint64_t TargetBlockSpacingSeconds, uint64_t PastBlocksMin, uint64_t PastBlocksMax, const Consensus::Params& params);

#endif
