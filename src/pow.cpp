// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "chain.h"
#include "chainparams.h"
#include "primitives/block.h"
#include "uint256.h"
#include "util.h"


// TODO: These values are from original source
static int64 nTargetTimespan = 270000; // 3.125 days to difficulty retarget
static int64 nTargetSpacing = 60;       // 1 minute between blocks
static int64 nInterval = nTargetTimespan / nTargetSpacing;

// TODO: This stuff must all be changed to int64_t and so forth
unsigned int static KimotoGravityWell(const CBlockIndex* pindexLast, const CBlockHeader* pblock, uint64 TargetBlockSpacingSeconds, uint64 PastBlocksMin, uint64 PastBlocksMax)
{
	const CBlockIndex *BlockLastSolved = pindexLast;
	const CBlockIndex *BlockReading    = pindexLast;

	uint64 PastBlocksMass          = 0;
	int64  PastRateActualSeconds   = 0;
	int64  PastRateTargetSeconds   = 0;
	double PastRateAdjustmentRatio = double(1);

	CBigNum PastDifficultyAverage, PastDifficultyAveragePrev;
	double  EventHorizonDeviation, EventHorizonDeviationFast, EventHorizonDeviationSlow;

	if(BlockLastSolved == NULL || BlockLastSolved->nHeight == 0 || (uint64) BlockLastSolved->nHeight < PastBlocksMin)
		return bnProofOfWorkLimit.GetCompact();

	for(unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++)
	{
		if(PastBlocksMax > 0 && i > PastBlocksMax)
			break;

		PastBlocksMass++;

		PastDifficultyAverage = (i == 0)
			? PastDifficultyAverage.SetCompact(BlockReading->nBits)
			: PastDifficultyAverage = ((CBigNum().SetCompact(BlockReading->nBits) - PastDifficultyAveragePrev) / i) + PastDifficultyAveragePrev;

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

		if( (PastBlocksMass >= PastBlocksMin && (PastRateAdjustmentRatio <= EventHorizonDeviationSlow || PastRateAdjustmentRatio >= EventHorizonDeviationFast))
			|| (BlockReading->pprev == NULL)) {
			assert(BlockReading);
			break;
		}

		BlockReading = BlockReading->pprev;
	}

	CBigNum bnNew(PastDifficultyAverage);
	if(PastRateActualSeconds != 0 && PastRateTargetSeconds != 0)
	{
		bnNew *= PastRateActualSeconds;
		bnNew /= PastRateTargetSeconds;
	}

	if(bnNew > bnProofOfWorkLimit) { bnNew = bnProofOfWorkLimit; }

	// Debug
	// printf("Difficulty Retarget - Kimoto Gravity Well\n");
	// printf("PastRateAdjustmentRatio = %g\n", PastRateAdjustmentRatio);
	// printf("Before: %08x %s\n", BlockLastSolved->nBits, CBigNum().SetCompact(BlockLastSolved->nBits).getuint256().ToString().c_str());
	// printf("After:  %08x %s\n", bnNew.GetCompact(), bnNew.getuint256().ToString().c_str());

	return bnNew.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock)
{
    unsigned int nProofOfWorkLimit = Params().ProofOfWorkLimit().GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    if(pindexLast->nHeight >= 45000 || fTestNet)
    {
        nTargetTimespan = 61440;                      // ~0.711111 days difficulty retarget
        nTargetSpacing = 60 * 2;                      // 2 minutes between blocks
        nInterval = nTargetTimespan / nTargetSpacing; // 512 blocks difficulty retarget

        unsigned int TimeDaySeconds = 60 * 60 * 24;
        uint64       PastSecondsMin = TimeDaySeconds * 0.25;
        uint64       PastSecondsMax = TimeDaySeconds * 7;
        uint64       PastBlocksMin  = PastSecondsMin / nInterval;
        uint64       PastBlocksMax  = PastSecondsMax / nInterval;

        return KimotoGravityWell(pindexLast, pblock, nTargetSpacing, PastBlocksMin, PastBlocksMax);
    }

    // Only change once per interval
    if ((pindexLast->nHeight+1) % Params().Interval() != 0)
    {
        if (Params().AllowMinDifficultyBlocks())
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + Params().TargetSpacing()*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % Params().Interval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Litecoin: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int blockstogoback = Params().Interval()-1;
    if ((pindexLast->nHeight+1) != Params().Interval())
        blockstogoback = Params().Interval();

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex* pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;
    assert(pindexFirst);

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - pindexFirst->GetBlockTime();
    LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
    if (nActualTimespan < Params().TargetTimespan()/4)
        nActualTimespan = Params().TargetTimespan()/4;
    if (nActualTimespan > Params().TargetTimespan()*4)
        nActualTimespan = Params().TargetTimespan()*4;

    // Retarget
    uint256 bnNew;
    uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
    // Litecoin: intermediate uint256 can overflow by 1 bit
    bool fShift = bnNew.bits() > 235;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= Params().TargetTimespan();
    if (fShift)
        bnNew <<= 1;

    if (bnNew > Params().ProofOfWorkLimit())
        bnNew = Params().ProofOfWorkLimit();

    /// debug print
    LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("Params().TargetTimespan() = %d    nActualTimespan = %d\n", Params().TargetTimespan(), nActualTimespan);
    LogPrintf("Before: %08x  %s\n", pindexLast->nBits, bnOld.ToString());
    LogPrintf("After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits)
{
    bool fNegative;
    bool fOverflow;
    uint256 bnTarget;

    if (Params().SkipProofOfWorkCheck())
       return true;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > Params().ProofOfWorkLimit())
        return error("CheckProofOfWork() : nBits below minimum work");

    // Check proof of work matches claimed amount
    if (hash > bnTarget)
        return error("CheckProofOfWork() : hash doesn't match nBits");

    return true;
}

uint256 GetBlockProof(const CBlockIndex& block)
{
    uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}
