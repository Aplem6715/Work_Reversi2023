﻿#include "game/record.h"

int main()
{
    game::MatchBook book;
    book.ReadAscii("resource/test.txt");
    book.WriteAscii("resource/test_out.txt");
    return 0;
}