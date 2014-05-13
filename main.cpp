#include <iostream>
#include <string>
#include <vector>
#include "lexer.hpp"
#include "parser.hpp"
#include "grammar.hpp"
#include "transformer.hpp"
#include "generation.hpp"

int main(int argc, char **argv) {
    std::string str("acm.td.pd.fs.k");
    jrun::parser::LexerState state;
    state.addTokenDef("[");	state.addTokenDef("]");
    state.addTokenDef(".");
    const char* start = str.c_str();
    state.parse(start, &start[str.size()]);
    std::vector<jrun::parser::TokenPtr> tks = state.readAll();  
    jrun::parser::TokenPtr tk = tks.back();
    std::cout << tk->id << tk->tok << std::endl;
    jrun::parser::PDA pda(tks);
    jrun::grammar::mGrammar startRule;
    jrun::parser::production pd;
    bool r = pda.parse(startRule, pd);
    if(!r)
      std::cerr << "error parse" << std::endl;
    else
    {
      jrun::generation::names* pds = static_cast<jrun::generation::names*>(pd.result.get());
      std::cout << pds->size() << std::endl;
      for(auto c : *pds)
	std::cout << c << std::endl;
    }
    return 0;
}
