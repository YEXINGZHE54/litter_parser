#include "lexer.hpp"
#include <iostream>
#include <boost/concept_check.hpp>

using jrun::parser::stateID;
using namespace jrun::parser;

std::map<std::string, tokenID> LexerState::keyTokens;

static stateID tb[9][128] = {
  0
};

static void initTable()
{
  std::vector<unsigned char> chars{'>','<', '=', '-', '+', '*', '/', '!', '%', '^', '&', '(', ')', '[', ']', '{', '}', ',', '.', ';', ':'};
  stateID* tr = tb[0];
  for(unsigned char c = 'a'; c <= 'z'; ++c)
    tr[c] = jrun::parser::ID;
  for(unsigned char c = 'A'; c <= 'Z'; ++c)
    tr[c] = jrun::parser::ID;
  for(unsigned char c = '0'; c <= '9'; ++c)
    tr[c] = jrun::parser::NUMBER;
  tr['"'] = 7; 	tr['_'] = jrun::parser::ID; 	tr['\''] = 8;
  for(auto c : chars)
    tr[c] = jrun::parser::TEMP;
  //tr['>'] = tr['<'] = tr['='] = tr['!'] = tr['+'] = tr['-'] = tr['*'] = tr['/'] = tr['%'] = tr['^'] = tr['&'] = jrun::parser::OPERATOR;
  //tr[','] = tr[';'] = tr['['] = tr[']'] = tr['{'] = tr['}'] = tr['('] = tr[')'] = jrun::parser::PUNCTURE;
  tr = tb[jrun::parser::ID];
  for(unsigned char c = 'a'; c <= 'z'; ++c)
    tr[c] = jrun::parser::ID;
  for(unsigned char c = 'A'; c <= 'Z'; ++c)
    tr[c] = jrun::parser::ID;
  for(unsigned char c = '0'; c <= '9'; ++c)
    tr[c] = jrun::parser::ID;
  tr['_'] = jrun::parser::ID;
  tr = tb[jrun::parser::NUMBER];
  for(unsigned char c = '0'; c <= '9'; ++c)
    tr[c] = jrun::parser::NUMBER;
  tr = tb[7];	//temp row for string
  for(unsigned char c = 0; c < 128; ++c)
    tr[c] = 7;
  tr['"'] = jrun::parser::STRING;
  tr = tb[8];
  for(unsigned char c = 0; c < 128; ++c)
    tr[c] = 8;
  tr['\''] = jrun::parser::STRING2;
  tr = tb[jrun::parser::TEMP];
  for(auto c : chars)
    tr[c] = jrun::parser::TEMP;
  //tr['>'] = tr['<'] = tr['='] = tr['!'] = tr['+'] = tr['-'] = tr['*'] = tr['/'] = tr['%'] = tr['^'] = tr['&'] = tr['('] = tr[')'] = jrun::parser::OPERATOR;
}

static void initKeyTokens(std::map<std::string, tokenID>& m)
{
  m["function"] = jrun::parser::KEY_FUNCTION;
  m["break"] = jrun::parser::KEY_BREAK;
  m["for"] = jrun::parser::KEY_FOR;
  m["while"] = jrun::parser::KEY_WHILE;
  m["if"] = jrun::parser::KEY_IF;
  m["else"] = jrun::parser::KEY_ELSE;
  m["return"] = jrun::parser::KEY_RETURN;
}

jrun::parser::Token::Token(tokenID idt, const char* start, const char* end) : id(idt), tok(std::string(start,end))
{
  if((id == jrun::parser::TEMP || id == jrun::parser::ID) && LexerState::keyTokens.count(tok) > 0)
    id = LexerState::keyTokens[tok];
}

jrun::parser::LexerState::LexerState()
{
  if(tb[0]['a'] == 1) return;
  initTable();
  initKeyTokens(keyTokens);
}

bool jrun::parser::LexerState::parse(const char* begin, const char* endl)
{
  start = begin;
  end = endl;
  return true;
}

bool LexerState::addTokenDef(std::string str, tokenID id)
{
  keyTokens[str] = id;
  return true;
}

bool LexerState::addTokenDef(std::string str)
{
  return addTokenDef(str, jrun::parser::KEYS);
}

jrun::parser::Token* jrun::parser::LexerState::read()
{  
  const char* cs = start;
  stateID state = 0;
  while(start != end)
  {
    unsigned char c = *start;
    stateID new_state = tb[state][c];
    if( new_state == 0 )
    {
      if(state == 0) 
      {
	++start;
	cs = start;
	continue;
      }
      else
      {
	if(state < LexerState::maxID )
	  return new Token(state, cs, start);
      }
    }
    else
    {
      state = new_state;
    }
    ++start;
  }
  if(cs != start) //reads some chars
  {
    if(state < LexerState::maxID )
      return new Token(state, cs, start);
  } else {
    start = cs;
  }
  return nullptr;
}

std::vector< TokenPtr > LexerState::readAll()
{
  std::vector< TokenPtr > tks;
  Token* tk;
  while((tk = read()) != nullptr)
      tks.push_back(std::shared_ptr<Token>(tk));

  return tks;
}