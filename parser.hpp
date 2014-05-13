#ifndef JRUN_PARSER_PARSER_HPP
#define JRUN_PARSER_PARSER_HPP

#include <vector>
#include <functional>
#include <memory>
#include <boost/concept_check.hpp>
#include "lexer.hpp"

/*
 * _Rule class deal with real part of parsing
 * Rule is just a box of the ptr, to deal with constructing _Rule
 */

namespace jrun{
  namespace parser{
    typedef std::size_t offset;
    struct production {
      std::shared_ptr<void>  result;
    };    
    class _LinkRule; class _KneeRule; class _OptionRule;	class _Rule;
    typedef std::vector<production> pdsType;
    typedef std::shared_ptr< pdsType > pdsPtr;
    typedef std::shared_ptr<_Rule> RulePtr;
    typedef std::shared_ptr<_LinkRule> LRulePtr;
    typedef std::shared_ptr<_KneeRule> KRulePtr;
    typedef std::shared_ptr<_OptionRule> ORulePtr;
    typedef std::function<void(production&, const int)> transformer;   
    typedef std::function<bool(offset&, const offset, const std::vector<TokenPtr>&, production&)> evalFun;
    // note: all start rule should be converted to linkrule
    // production arg in evalFun is supposed to be link production
    // after evalFun is called, start is always changed disregarding whether matched! so, carfully to store start before call
    extern evalFun alwaysTrue;
    /*
     * note:
     * LinkRule produces a vector, containing things. if only one in it, it was assigned to be the one. if none, it produces none
     * KneeRule produces a vector, containing more same things. if all void rule in it, it produces none
     * OptionRule is just a wrapper for delegated one. when matched, it will call a transfor,where u can produces a variant or do other
     * but, if the delegated one does not produce anything, then it do none
     * _oneTokenRule is like qi::alpha, qi::num, _stringRule is like qi::string, _unusedTokenRule is like qi::unused
     * note:
     * linkRule or KneeRule produces the reversed links, so you can visit in reversed order
     */
    class _Rule : public std::enable_shared_from_this<_Rule>{
    public:               
      //virtual std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      virtual evalFun compile(const evalFun& next);
      transformer transform;    
      void setTransform(transformer tr);
      int weight = 1;
    };
    class _LinkRule : public _Rule {
    public:
      _LinkRule(RulePtr ptr1, RulePtr ptr2) : rules{ptr1, ptr2}{  weight = ptr1->weight + ptr2->weight;    };
      _LinkRule(_Rule* ptr1, _Rule* ptr2) : rules{ptr1->shared_from_this(), ptr2->shared_from_this()}{  weight = ptr1->weight + ptr2->weight;    };
      //std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      virtual evalFun compile(const evalFun& next) override;
      std::vector<RulePtr> rules;
    };
    class _KneeRule : public _Rule {
    public:    
      _KneeRule(RulePtr ptr) : r(ptr){  weight = ptr->weight; };
      _KneeRule(_Rule* ptr) : r(ptr){ weight = ptr->weight; };
      //std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      virtual evalFun compile(const evalFun& next) override;
      RulePtr r;
    };
    class _OptionRule : public _Rule {
    public:
      _OptionRule(RulePtr ptr1, RulePtr ptr2) : rules{ptr1, ptr2}{  weight = ptr1->weight? ptr1->weight : ptr2->weight;    };
      _OptionRule(_Rule* ptr1, _Rule* ptr2) : rules{ptr1->shared_from_this(), ptr2->shared_from_this()}{  weight = ptr1->weight? ptr1->weight : ptr2->weight;    };
      //std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      virtual evalFun compile(const evalFun& next) override;
      //RulePtr r1; 	RulePtr r2;
      std::vector<RulePtr> rules;
    };
    class _oneTokenRule : public _Rule {
    public:
      _oneTokenRule(tokenID idt) : id(idt){};
      //std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      virtual evalFun compile(const evalFun& next) override;
      tokenID id;
    };
    class _stringRule : public _Rule {
    public:
      _stringRule(std::string idt) : id(idt){};
      //std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      virtual evalFun compile(const evalFun& next) override;
      std::string id;
    };
    class _unusedTokenRule : public _Rule {
    public:
      _unusedTokenRule(std::string idt) : id(idt){ weight = 0; };
      virtual evalFun compile(const evalFun& next) override;
      std::string id;
    };
    class LinkRule; class KneeRule; class OptionRule;
    class Rule {
    public:
      Rule(){};
      Rule(_Rule* r) : ptr(r){};
      Rule(RulePtr r) : ptr(r){};
      Rule(_Rule* r, transformer tr) : ptr(r){ ptr->setTransform(tr); };
      void setTransform(transformer tr){ ptr->setTransform(tr); };
      //std::vector<production> parse(offset& start, const offset end, const std::vector<TokenPtr>& tks);
      //production transform(const std::vector<production>&pds){ return ptr->transform(pds); };
      evalFun compile(evalFun next) { return ptr->compile(next); };
      LinkRule operator>>(Rule r);
      LinkRule operator>>(LinkRule r);
      LinkRule operator>>(const char* c);
      KneeRule operator*();
      OptionRule operator||(Rule r);
      OptionRule operator||(OptionRule r);
      Rule& operator[](transformer tr){ ptr->setTransform(tr); return *this; };
      RulePtr ptr;
    };
    class LinkRule : public Rule {
    public:
      LinkRule(_LinkRule* r) : LinkRule(LRulePtr(r)){};
      LinkRule(LRulePtr r);
      LinkRule(_LinkRule* r, transformer tr) : Rule(r, tr){};
      LinkRule operator>>(Rule r);
      LinkRule operator>>(LinkRule r);
      LinkRule operator>>(const char* c);
      LinkRule& operator[](transformer tr){ ptr->setTransform(tr); return *this; };
    };
    class KneeRule : public Rule {
    public:
      KneeRule(_KneeRule* r);
      KneeRule(_KneeRule* r, transformer tr) : Rule(r, tr){};
      KneeRule& operator[](transformer tr){ ptr->setTransform(tr); return *this; };
    };
    class OptionRule : public Rule {
    public:
      OptionRule(_OptionRule* r);
      OptionRule(_OptionRule* r, transformer tr) : Rule(r, tr){};
      OptionRule(ORulePtr r);
      OptionRule operator||(Rule r);
      OptionRule operator||(OptionRule r);
      OptionRule& operator[](transformer tr){ ptr->setTransform(tr); return *this; };
    };
    class BasicRule : public Rule {
    public:
      BasicRule(tokenID id);
      BasicRule(tokenID id, transformer tr);
    };
    class StringRule : public Rule {
    public:
      StringRule(std::string);
      StringRule(std::string, transformer tr);
    };
    class VoidRule : public Rule {
    public:
      VoidRule(std::string);
    };
    
    typedef StringRule string;
    typedef VoidRule lit;
    typedef BasicRule rule;
    
    class PDA{
    public:
      PDA(std::vector<TokenPtr>& tks) : sequences(tks){};
      bool parse(Rule& r, production& pd);
    private:
      std::vector<TokenPtr> sequences;
    };
  }
}

#endif