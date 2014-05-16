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
  unsigned char ops[]{'>','<', '=', '-', '+', '*', '/', '!', '%', '^', '&'};
  unsigned char puns[]{'(', ')', '[', ']', '{', '}', ',', '.', ';', ':'};
  stateID* tr = tb[0];
  for(unsigned char c = 'a'; c <= 'z'; ++c)
    tr[c] = jrun::parser::ID;
  for(unsigned char c = 'A'; c <= 'Z'; ++c)
    tr[c] = jrun::parser::ID;
  for(unsigned char c = '0'; c <= '9'; ++c)
    tr[c] = jrun::parser::NUMBER;
  tr['"'] = TEMP1; 	tr['_'] = jrun::parser::ID; 	tr['\''] = TEMP2;
  for(auto c : ops)
    tr[c] = jrun::parser::OPERATOR;
  for(auto c : puns)
    tr[c] = jrun::parser::PUNCTURE;
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
  tr = tb[TEMP1];	//temp row for string
  for(unsigned char c = 0; c < 128; ++c)
    tr[c] = TEMP1;
  tr['"'] = jrun::parser::STRING;
  tr = tb[TEMP2];
  for(unsigned char c = 0; c < 128; ++c)
    tr[c] = TEMP2;
  tr['\''] = jrun::parser::STRING2;
  tr = tb[jrun::parser::OPERATOR];
  for(auto c : ops)
    tr[c] = jrun::parser::OPERATOR;
  //tr['>'] = tr['<'] = tr['='] = tr['!'] = tr['+'] = tr['-'] = tr['*'] = tr['/'] = tr['%'] = tr['^'] = tr['&'] = tr['('] = tr[')'] = jrun::parser::OPERATOR;
}

static void initKeyTokens(std::map<std::string, tokenID>& m)
{
  m["function"] = jrun::parser::KEYS;
  m["break"] = jrun::parser::KEYS;
  m["for"] = jrun::parser::KEYS;
  m["while"] = jrun::parser::KEYS;
  m["if"] = jrun::parser::KEYS;
  m["else"] = jrun::parser::KEYS;
  m["return"] = jrun::parser::KEYS;
}

jrun::parser::Token::Token(tokenID idt, const char* start, const char* end) : id(idt), tok(std::string(start,end))
{
  if((id >= jrun::parser::KEYS || id == jrun::parser::ID) && LexerState::keyTokens.count(tok) > 0)
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