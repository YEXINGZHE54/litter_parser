#ifndef JRUN_PARSER_GRAMMAR_HPP
#define JRUN_PARSER_GRAMMAR_HPP

#include "parser.hpp"
namespace jrun{
  namespace grammar{
    struct mGrammar : jrun::parser::Rule{
      mGrammar();
    };
//end of namespace
  }
}
/*
#include <vector>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_auto.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/phoenix.hpp>

#include "generation.hpp"

namespace jrun {
  namespace parser{   
    namespace qi = boost::spirit::qi;
    namespace phoenix = boost::phoenix;
    namespace ascii = boost::spirit::ascii;
    namespace data = jrun::generation;

    bool debug_printer(const char *msg){
	return true;
    }
    
    template<typename T>
    struct mXml_grammer : qi::grammar<T, data::AST(), ascii::space_type >
    {
    mXml_grammer();
    //typedef typename LexerType::iterator_type T;   
#define ruleDefine(name, type) qi::rule<T, type(), ascii::space_type > name

    ruleDefine(start, data::AST);	ruleDefine(charRule, std::string);
    ruleDefine(literRule, data::literValue);	ruleDefine(symName, std::string);
    ruleDefine(nameRule, data::names);		
    ruleDefine(mapkeyRule, data::mapKey);	ruleDefine(mapConstRule, data::mapConst);
    ruleDefine(leftVRule, data::leftValue);	ruleDefine(rightVRule, data::rightValue);
    ruleDefine(funcRule, data::funCall);	ruleDefine(AFuncRule, data::AnnoFunc);		ruleDefine(NFuncRule, data::NamedFunc);
    ruleDefine(sOpRule, data::sOpValue);	ruleDefine(dOpRule, data::dOpValue);
    ruleDefine(tVRule, data::tValue);		ruleDefine(wrapRightVRule, data::wrappedRightValue);
    ruleDefine(objectRule, data::Objectdef);	ruleDefine(propertyAssignRule, data::propertyAssign);
    ruleDefine(retRule, data::retCommand);
    ruleDefine(vargsRule, data::virtualArgs);	ruleDefine(rargsRule, data::RealArgs);
    ruleDefine(commandRule, data::mCommand);	ruleDefine(exprRule, data::Expr);	ruleDefine(assignRule, data::Assign);
    ruleDefine(ifRule, data::ifCommand);	ruleDefine(forRule, data::forCommand);
    ruleDefine(zOmExprs, std::vector<data::Expr>);
    ruleDefine(whileRule, data::whileCommand);	ruleDefine(breakRule, data::breakCommand);
#undef ruleDefine
    void report (const char * a) { std::cout << a; };
    };
    
    template<typename T>
    mXml_grammer<T> :: mXml_grammer() : mXml_grammer::base_type(start)
    {
      using namespace qi;	using namespace phoenix;	using namespace jrun::log;
      
      const char opBrace = '{', clBrace = '}', opBracket = '[', clBracket = ']', opParen = '(', clParen = ')';
      const char comma = ',', dot = '.', semicolon = ';', colon = ':', equal = '=';
      
      symName %= ascii::alpha >> *(ascii::alnum | '_');	symName.name("symNameRule");
      literRule %= ascii::string("undefined") | ascii::string("null") | ascii::string("true") | ascii::string("false") | (ascii::char_('"') >> *(ascii::char_ - '"') >> '"' ) | (+ascii::digit) ;
      //literRule = omit[ ascii::char_("\"") ] >> *(ascii::char_ - ascii::char_("\"")) > ascii::char_("\"");
      literRule.name("LiteralRule");
      nameRule %= symName % dot;	nameRule.name("names");
     
      mapkeyRule %= nameRule >> opBracket >> nameRule >> clBracket;	mapkeyRule.name("mapKey");
      mapConstRule %= nameRule >> opBracket >> literRule >> clBracket;	mapConstRule.name("mapConst");
      leftVRule = mapkeyRule[_val = qi::_1] || mapConstRule[_val = qi::_1] || nameRule[_val = qi::_1];		
      leftVRule.name("leftValue");
      //to avoid circulation between dOpvalue and rightValue, by '('
      wrapRightVRule = ( '(' >> rightVRule[_val = qi::_1] >> ')' ) || tVRule[_val = qi::_1];
      wrapRightVRule.name("wrappedRightValue");
      dOpRule = wrapRightVRule[at_c<0>(_val) = qi::_1] 
		>> ( ascii::string("+")|ascii::string("-")|ascii::string("*")|ascii::string("/") 
		  | ascii::string("===") | ascii::string("!==")
		  | ascii::string(">=") | ascii::string("<=") | ascii::string("==") | ascii::string("!=") | ascii::string("&&")  | ascii::string("||") 
		  | ascii::string(">") | ascii::string("<")
		)[at_c<1>(_val) = qi::_1]
		>> ( (rightVRule[at_c<2>(_val) = qi::_1] >> qi::attr(false)[at_c<3>(_val) = qi::_1] ) 
		    | ( '(' >> rightVRule[at_c<2>(_val) = qi::_1] >> ')' >> qi::attr(true)[at_c<3>(_val) = qi::_1] ) ); 
      dOpRule.name("dOp");
      sOpRule %= tVRule >> (ascii::string("++")|ascii::string("--"));
      sOpRule.name("sOp");
      tVRule = funcRule[_val = qi::_1] || leftVRule[_val = qi::_1] || literRule[_val = qi::_1];
      tVRule.name("tVRule");
      objectRule %= opBrace >> *(propertyAssignRule >> qi::lit(semicolon)) >> clBrace;
      objectRule.name("object");
      rightVRule = AFuncRule[_val = qi::_1] || dOpRule[_val = qi::_1] || sOpRule[_val = qi::_1] || 
		   funcRule[_val = qi::_1] || objectRule[_val = qi::_1]
		    || leftVRule[_val = qi::_1] || literRule[_val = qi::_1];
      rightVRule.name("rightValue");
      vargsRule = -symName[push_back(_val, qi::_1)] >> *(comma >> symName[push_back(_val, qi::_1)]);
      vargsRule.name("vargs");
      //NFuncRule = qi::lit("function") >> symName[at_c<0>(_val)=qi::_1];
      NFuncRule %=  qi::lit("function") >> symName
		>> opParen >> vargsRule >> clParen
		>> opBrace >> *(commandRule >> qi::lit(semicolon)) >> clBrace;
      NFuncRule.name("NamedFunctionRule");
      AFuncRule %= qi::lit("function")
		>> opParen >> vargsRule >> clParen 
		>> opBrace >> *(commandRule >> qi::lit(semicolon)) >> clBrace;
      AFuncRule.name("AnnoymousFunctionRule");
      //rargsRule %= rightVRule % comma;
      rargsRule = -rightVRule[push_back(_val, qi::_1)] >> *(comma >> rightVRule[push_back(_val, qi::_1)]);
      rargsRule.name("rargsRule");
      funcRule %= leftVRule >> opParen >> rargsRule >> clParen;
      funcRule.name("funCallRule");
      exprRule = assignRule[_val = qi::_1] || rightVRule[_val = qi::_1];
      exprRule.name("expr");
      assignRule %= leftVRule >> equal >> rightVRule;
      assignRule.name("AssignmentRule");
      propertyAssignRule %= symName  >> colon >> rightVRule ;
      propertyAssignRule.name("property");
      retRule %= qi::lit("return") >> rightVRule ;
      retRule.name("return");
      commandRule = NFuncRule[_val = qi::_1] || retRule[_val = qi::_1] || ifRule[_val = qi::_1] || 
		    forRule[_val = qi::_1] || whileRule[_val = qi::_1] || breakRule[_val = qi::_1] || exprRule[_val = qi::_1] ;
      commandRule.name("command");
      ifRule = qi::lit("if")
		>> opParen >> exprRule[at_c<0>(_val) = qi::_1] >> clParen 
		>>( (opBrace >> *(commandRule[push_back(at_c<1>(_val), qi::_1)] >> qi::lit(semicolon)) >> clBrace)
		| (commandRule[push_back(at_c<1>(_val), qi::_1)] >> qi::lit(semicolon)) )
		>> qi::lit("else")
		>>( (opBrace >> *(commandRule[push_back(at_c<2>(_val), qi::_1)] >> qi::lit(semicolon)) >> clBrace)
		| ( commandRule[push_back(at_c<2>(_val), qi::_1)] ) );
		;
      zOmExprs = -exprRule[push_back(_val, qi::_1)] >> *(comma >> exprRule[push_back(_val, qi::_1)]);
      forRule = qi::lit("for")
		>> opParen >> zOmExprs[at_c<0>(_val) = qi::_1] >> semicolon >> 
		exprRule[at_c<1>(_val) = qi::_1] >> semicolon >>
		zOmExprs[at_c<2>(_val) = qi::_1] >> clParen
		>>( (opBrace >> *(commandRule[push_back(at_c<3>(_val), qi::_1)] >> qi::lit(semicolon)) >> clBrace)
		| commandRule[push_back(at_c<3>(_val), qi::_1)] );
      whileRule = qi::lit("while")
		>> opParen >> exprRule[at_c<0>(_val) = qi::_1] >> clParen
		>>( (opBrace >> *(commandRule[push_back(at_c<1>(_val), qi::_1)] >> qi::lit(semicolon)) >> clBrace)
		| commandRule[push_back(at_c<1>(_val), qi::_1)] );
      whileRule.name("while");
      forRule.name("forRule");
      ifRule.name("ifRule");
      breakRule = qi::lit("break") >> qi::attr(true);
      start = +(commandRule[push_back(at_c<0>(_val), qi::_1)] >> qi::lit(semicolon));
      start.name("start");
    }
  }
}
*/
#endif