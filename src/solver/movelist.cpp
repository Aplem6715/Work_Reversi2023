#include "movelist.h"
#include "hash.h"
#include "search.h"

namespace solver
{
    Move* solver::MoveList::GetNextBest()
    {
        if (!lastMove_->next_)
        {
            return nullptr;
        }

        // 最善手を取得
        Move* best = lastMove_;
        for (Move* target = lastMove_; target->next_ != nullptr; target = target->next_)
        {
            if (target->next_->value_ > best->next_->value_)
            {
                best = target;
            }
        }

        if (best != lastMove_)
        {
            // bestを切り離す
            Move* tmp   = best->next_;
            best->next_ = tmp->next_;
            tmp->next_  = lastMove_->next_;

            // 繋げ直し
            lastMove_->next_ = tmp;
        }
        // 次の着手
        lastMove_ = lastMove_->next_;
        return lastMove_;
    }

    Move* solver::MoveList::GetNext()
    {
        lastMove_ = lastMove_->next_;
        return lastMove_;
    }

    void MoveList::Evaluate(Searcher& searcher, const HashData& hashData)
    {
        for (auto move = moves_->next_; move; move = move->next_)
        {
            move->Evaluate(searcher, hashData);
        }
    }

    void Move::Evaluate(Searcher& searcher, const HashData& hashData)
    {
        constexpr int kWipeoutOrder      = 30;
        constexpr int kBestMoveOrder     = 29;
        constexpr int kSecondMoveOrder   = 28;
        constexpr int kMobilityOrder     = 16;
        constexpr int kOneStepScoreOrder = 8;
        constexpr int kPosScoreOrder     = 0;

        const board::Stone& stone = searcher.GetStone();

        // 全消し
        if (flips_ == stone.opp_)
        {
            value_ = (1 << kWipeoutOrder);
            return;
        }

        // 最善手
        if (hashData.bestMove_ != Position::NoMove)
        {
            value_ = (1 << kBestMoveOrder);
            return;
        }

        if (hashData.secondMove_ != Position::NoMove)
        {
            value_ = (1 << kSecondMoveOrder);
            return;
        }
        else
        {
            eval::Evaluator& eval = searcher.GetEval();

            Position pos    = pos_;
            uint64_t posBit = PosToBit(pos);
            uint64_t flips  = flips_;

            board::Stone nextStone = stone;
            nextStone.Update(posBit, flips);

            // 着手可能数での評価=速度優先探索（24~16bit）
            const uint64_t nextMob = nextStone.CalcMobility();
            // 正の値にするためのバイアス
            int mobCount = kMaxMove + 4 /*角ボーナス分*/;
            mobCount += CountBits(nextMob);
            mobCount += CountBits(nextMob & 0x8100000000000081);
            // 相手の手が多い＝マイナス
            int value = mobCount * (1 << kMobilityOrder);

            // 一手読みのスコア付け（16~8bit目)
            // 着手して相手のターンに進める
            eval.Update(posBit, flips);
            const score32_t score = eval.Evaluate(searcher.GetNumEmpty() - 1);
            eval.Restore(posBit, flips);
            value -= score * (1 << kOneStepScoreOrder);

            // 着手位置でスコア付け(8~0bit)
            value = ValueTable[static_cast<int>(pos)] * (1 << kPosScoreOrder);

            value_ = value;
        }
    }
}
