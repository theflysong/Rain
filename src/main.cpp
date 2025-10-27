#include <iostream>

#include "lexer/lexer.h"
#include "file/helper.h"

int main(int, char**){
    rain::Lexer lexer(rain::readall("./text.txt"));
    int a = 0;
    std::cin >> a;
    return 0;
}
