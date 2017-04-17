// Copyright (c) 2015-2015 The e-Gulden developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <math.h>

#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"

unsigned int KimotoGravityWell(const CBlockIndex* pindexLast, uint64_t TargetBlockSpacingSeconds, uint64_t PastBlocksMin, uint64_t PastBlocksMax, const Consensus::Params& params)
{
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading    = pindexLast;

    uint64_t PastBlocksMass          = 0;
    int64_t  PastRateActualSeconds   = 0;
    int64_t  PastRateTargetSeconds   = 0;
    double   PastRateAdjustmentRatio = double(1);

    arith_uint256 PastDifficultyAverage, PastDifficultyAveragePrev;
    double        EventHorizonDeviation, EventHorizonDeviationFast, EventHorizonDeviationSlow;

    if(BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64_t) BlockLastSolved->nHeight < PastBlocksMin)
        return UintToArith256(params.powLimit).GetCompact();

    for(unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++)
    {
        if(PastBlocksMax > 0 && i > PastBlocksMax) break;
        PastBlocksMass++;

        PastDifficultyAverage.SetCompact(BlockReading->nBits);

        if(i > 1) {
            if(PastDifficultyAverage >= PastDifficultyAveragePrev) {
                PastDifficultyAverage = ((PastDifficultyAverage - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;
            } else {
                PastDifficultyAverage = PastDifficultyAveragePrev - ((PastDifficultyAveragePrev - PastDifficultyAverage) / i);
            }
        }

        PastDifficultyAveragePrev = PastDifficultyAverage;

        PastRateActualSeconds = BlockLastSolved->GetBlockTime() - BlockReading->GetBlockTime();
        PastRateTargetSeconds = TargetBlockSpacingSeconds * PastBlocksMass;

        PastRateActualSeconds = (PastRateActualSeconds < 0) ? 0 : PastRateActualSeconds;
        PastRateAdjustmentRatio = (PastRateActualSeconds != 0 && PastRateTargetSeconds != 0)
            ? double(PastRateTargetSeconds) / double(PastRateActualSeconds)
            : double(1);

        EventHorizonDeviation     = 1 + (0.7084 * pow((double(PastBlocksMass) / double(144)), -1.228));
        EventHorizonDeviationFast = EventHorizonDeviation;
        EventHorizonDeviationSlow = 1 / EventHorizonDeviation;

        if((PastBlocksMass >= PastBlocksMin
                    && (PastRateAdjustmentRatio <= EventHorizonDeviationSlow || PastRateAdjustmentRatio >= EventHorizonDeviationFast))
                || (BlockReading->pprev == NULL)) {
            assert(BlockReading);
            break;
        }

        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);
    if(PastRateActualSeconds != 0 && PastRateTargetSeconds != 0)
    {
        bnNew *= PastRateActualSeconds;
        bnNew /= PastRateTargetSeconds;
    }

    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    if(bnNew > bnPowLimit) {
        bnNew = bnPowLimit;
    }

    // Debug
    /*
       LogPrintf("Difficulty Retarget - Kimoto Gravity Well [Block: %d]\n", pindexLast->nHeight);
       LogPrintf("PastRateAdjustmentRatio = %g\n", PastRateAdjustmentRatio);
       LogPrintf("Before: %08x %s\n", pindexLast->nBits, uint256().SetCompact(pindexLast->nBits).ToString().c_str());
       LogPrintf("After:  %08x %s\n", bnNew.GetCompact(), bnNew.ToString().c_str());
       */

    return bnNew.GetCompact();
}
