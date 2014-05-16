#ifndef JRUN_PARSER_LEXER_HPP
#define JRUN_PARSER_LEXER_HPP

#include <string>
#include <map>
#include <memory>
#include <vector>

namespace jrun{
  namespace parser{
    typedef unsigned short stateID;
    typedef unsigned short tokenID;
    enum tokenIDs : stateID{      
      ID = 1,
      NUMBER,
      STRING,      
      STRING2,
      TEMP1,
      TEMP2,
      KEYS,	//user defined or special chars
      OPERATOR,
      PUNCTURE,
      MIN_ID
    };
    struct Token{
      Token(tokenID idt, const char* start, const char* end);
      tokenID id;
      std::string tok;      
    };
    typedef std::shared_ptr<Token> TokenPtr;
    class LexerState{
    public:
      LexerState();
      bool parse(const char* start, const char* end);      
      std::vector<TokenPtr> readAll();
      bool addTokenDef(std::string str);
      bool addTokenDef(std::string str, tokenID id);
      static const tokenID maxID = MIN_ID;
      static std::map<std::string, tokenID> keyTokens;
    private:
      Token* read();	//be careful! it should delete after used
      const char* start;
      const char* end;
    };    
  }
}

#endif