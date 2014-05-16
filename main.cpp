#include <iostream>
#include <string>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "grammar.hpp"
#include "transformer.hpp"
#include "generation.hpp"
#include <fstream>

int main(int argc, char **argv) {
    std::string str;  
    std::ifstream in("data.txt", std::ios_base::in);
    if (!in)
    {
        std::cerr << "Error: Could not open input file: "
            << "data.txt" << std::endl;
        return false;
    }
    in.unsetf(std::ios::skipws); // No white space skipping!
    std::copy(
      std::istream_iterator<char>(in),
      std::istream_iterator<char>(),
      std::back_inserter(str)
    );
  
    jrun::parser::LexerState state;
    std::vector<const char*> chars{">", "<", "=", "-", "+", "*", "/", "!", "%", "^", "&", "(", ")", "[", "]", "{", "}", ",", ".", ";", ":"};
    for(auto c : chars)
      state.addTokenDef(std::string(c));
    const char* start = str.c_str();
    state.parse(start, &start[str.size()]);
    std::vector<jrun::parser::TokenPtr> tks = state.readAll();  
    jrun::parser::PDA pda(tks);
    jrun::grammar::mGrammar startRule;
    using namespace jrun::parser;
    jrun::parser::production pd;
    bool re = pda.parse(startRule, pd);
    if(!re)
      std::cerr << "error parse" << std::endl;
    else
    {
      std::vector<jrun::generation::mCommand>* pds = static_cast<std::vector<jrun::generation::mCommand>*>(pd.result.get());
      std::cout << pds->size() << std::endl;
    }
    return 0;
}
