﻿#ifndef SEARCH_H
#define SEARCH_H

#include <vector>

#include "board/stone.h"
#include "eval/pos_eval.h"
#include "search_option.h"
#include "movelist.h"

namespace solver
{
    using namespace board;
    using namespace eval;

    struct SearchResult;

    /// 1石あたりの評価値
    constexpr int EvalStone   = 10;
    constexpr int EvalMax     = EvalStone * 64;
    constexpr int EvalMin     = -EvalMax;
    constexpr int EvalInvalid = -999;

    class Searcher
    {
    public:
        Searcher();
        ~Searcher();

        void Reset();
        void Setup(SearchOption option);
        void SetStones(stone_t own, stone_t opp);

        /// @brief 最善手を検索
        /// @return 最善手
        void Search(SearchResult* result);

    private:
        // 盤面
        Stone stones_[1];
        // 評価関数
        PositionEvaluator eval_[1];
        // 探索設定
        SearchOption option_ = DEFAULT_OPTION;
        // 探索深度
        uint8_t depth_;
        // 空きマス数
        uint8_t nbEmpty_;

    private:
        /// @brief 手のリストを作成
        /// @param moveList 手の追加先
        void MakeMoveList(MoveList* moveList);

        /* 中盤探索(search_mid.cpp) */
        /// @brief 中盤探索ルート
        void MidRoot(SearchResult* result);

        void PassMid();

        /// @brief 中盤探索MinMax法（主にテストベース用，カットなしの正しい探索と探索速度のベースを提供）
        score_t MidMinMax(int depth, bool passed);

        /// @brief 中盤αβ探索
        score_t MidAlphaBeta(score_t upper, score_t lower, int depth, bool passed);

        /* 終盤探索(search_end.cpp) */
        /// @brief 終盤探索ルート
        void EndRoot(SearchResult* result);

        /// @brief 終盤探索MinMax法（主にテストベース用，カットなしの正しい探索と探索速度のベースを提供）
        score_t EndMinMax(int depth, bool passed);

        /// @brief 終盤αβ探索
        score_t EndAlphaBeta(score_t upper, score_t lower, int depth, bool passed);

        /* 探索中に使用するinline関数 */

        void Update(const Move* move, bool updateEval)
        {
            --nbEmpty_;
            const uint64_t posBit = PosToBit(move->pos_);
            stones_->Update(posBit, move->flips_);
            if (updateEval)
            {
                eval_->Update(posBit, move->flips_);
            }
        }

        void Restore(const Move* move, bool updateEval)
        {
            ++nbEmpty_;
            const uint64_t posBit = PosToBit(move->pos_);
            stones_->Restore(posBit, move->flips_);
            if (updateEval)
            {
                eval_->Restore(posBit, move->flips_);
            }
        }

        void UpdatePass()
        {
            stones_->Swap();
        }

        score_t WinJudge()
        {
            auto diff = stones_->GetCountDiff();
            if (diff > 0)
            {
                return EvalMax;
            }
            else if (diff < 0)
            {
                return EvalMin;
            }
            else
            {
                return 0;
            }
        }

        score_t WinJudgeEnd()
        {
            auto diff = stones_->GetCountDiff();
            // 空きマスは勝った方に加算される
            if (diff > 0)
            {
                return diff + nbEmpty_;
            }
            else if (diff < 0)
            {
                return diff - nbEmpty_;
            }
            else
            {
                return 0;
            }
        }
    };
}
#endif