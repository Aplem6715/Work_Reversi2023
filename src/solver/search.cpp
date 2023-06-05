﻿#include "search.h"
#include "board/bit.h"
#include "board/stone.h"
#include "movelist.h"
#include "search_result.h"

namespace solver
{
    Searcher::Searcher()
    {
    }

    Searcher::~Searcher()
    {
    }

    void Searcher::Reset()
    {
    }

    void Searcher::Setup(SearchOption option)
    {
        option_ = option;
    }

    void Searcher::SetStones(stone_t own, stone_t opp)
    {
        nbEmpty_ = CountBits(~(own | opp));

        stones_->own_ = own;
        stones_->opp_ = opp;

        eval_->Reload(own, opp, Side::Own);
    }

    void Searcher::Search(SearchResult* result)
    {
        Position pos = Position::NoMove;

        if (nbEmpty_ <= option_.endDepth_)
        {
            EndRoot(result);
        }
        else
        {
            MidRoot(result);
        }
    }

    void Searcher::MakeMoveList(MoveList* moveList)
    {
        Move* cursor  = moveList->moves_;
        Move* prev    = nullptr;
        Stone* stones = stones_;
        uint64_t mob  = CalcMobility64(stones->own_, stones->opp_);
        int nbMove    = 0;

        Position pos;
        for_bits(pos, mob)
        {
            ++nbMove;
            cursor->pos_   = pos;
            cursor->value_ = 0;
            cursor->flips_ = stones->CalcFlip(pos);

            if (prev)
            {
                prev->next_ = cursor;
            }
            prev = cursor;
        }
        prev->next_       = nullptr;
        moveList->length_ = nbMove;
    }

    /* 中盤探索 */

    void Searcher::MidRoot(SearchResult* result)
    {
        score_t lower     = EvalMin;
        score_t upper     = EvalMax;
        score_t bestScore = EvalInvalid;

        MakeMoveList(result->moveList_);
        MoveList* moveList = result->moveList_;

        while (Move* move = moveList->GetNextBest())
        {
            Update(move, true);
            score_t score = -MidMinMax(option_.midDepth_, false);
            Restore(move, true);

            if (score > bestScore)
            {
                bestScore    = score;
                move->value_ = score;
            }
        }
    }

    score_t Searcher::MidMinMax(int depth, bool passed)
    {
        if (depth == 0)
        {
            return eval_->Evaluate(nbEmpty_);
        }

        score_t bestScore;
        MoveList moveList[1];

        MakeMoveList(moveList);

        if (moveList->IsEmpty())
        {
            if (passed)
            {
                return WinJudge();
            }
            else
            {
                UpdatePass();
                bestScore = -MidMinMax(depth - 1, true);
                UpdatePass();
            }
        }
        else
        {
            while (const Move* move = moveList->GetNextBest())
            {
                Update(move, true);
                const score_t score = -MidMinMax(depth - 1, false);
                Restore(move, true);

                if (score > bestScore)
                {
                    bestScore = score;
                }
            }
        }

        return bestScore;
    }

    score_t Searcher::MidAlphaBeta(score_t upper, score_t lower, int depth, bool passed)
    {
        // TODO: implement
        return score_t();
    }

    /* 終盤探索 */

    void Searcher::EndRoot(SearchResult* result)
    {
        score_t lower     = EvalMin;
        score_t upper     = EvalMax;
        score_t bestScore = EvalInvalid;

        MakeMoveList(result->moveList_);
        MoveList* moveList = result->moveList_;

        while (Move* move = moveList->GetNextBest())
        {
            Update(move, true);
            score_t score = -MidMinMax(option_.endDepth_, false);
            Restore(move, true);

            if (score > bestScore)
            {
                bestScore    = score;
                move->value_ = score;
            }
        }
    }

    score_t Searcher::EndMinMax(int depth, bool passed)
    {
        if (depth == 0)
        {
            return stones_->GetCountDiff();
        }

        score_t bestScore;
        MoveList moveList[1];

        MakeMoveList(moveList);

        if (moveList->IsEmpty())
        {
            if (passed)
            {
                return WinJudgeEnd();
            }
            else
            {
                UpdatePass();
                bestScore = -EndMinMax(depth - 1, true);
                UpdatePass();
            }
        }
        else
        {
            while (const Move* move = moveList->GetNextBest())
            {
                Update(move, true);
                const score_t score = -EndMinMax(depth - 1, false);
                Restore(move, true);

                if (score > bestScore)
                {
                    bestScore = score;
                }
            }
        }

        return bestScore;
    }

    score_t Searcher::EndAlphaBeta(score_t upper, score_t lower, int depth, bool passed)
    {
        // TODO: implement
        return score_t();
    }
}