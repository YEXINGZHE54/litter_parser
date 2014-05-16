#ifndef JRUN_PARSER_PARSER_HPP
#define JRUN_PARSER_PARSER_HPP

#include <vector>
#include <functional>
#include <memory>
#include <iostream>
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
    struct _rule_wrapper{
      std::shared_ptr<_Rule> ptr;
    };
    typedef std::vector<production> pdsType;
    typedef std::shared_ptr< pdsType > pdsPtr;
    typedef std::shared_ptr<_rule_wrapper> RulePtr;
    typedef std::shared_ptr<_LinkRule> LRulePtr;
    typedef std::shared_ptr<_KneeRule> KRulePtr;
    typedef std::shared_ptr<_OptionRule> ORulePtr;
    typedef std::function<void(production&, const int)> transformer;   
    typedef std::function<bool(offset&, const offset, const std::vector<TokenPtr>&, production&)> evalFun;
    // note: all start rule should be converted to linkrule
    // production arg in evalFun is supposed to be link production
    // after evalFun is called, start is always changed disregarding whether matched! so, carfully to store start before call
    // also, after evalFun is called, pds are pushed
    // if parse return true, pd is pushed, everything is done; if return false, none pd is pushed
    // in function parse, first r->parse(), then next(), then push pd of r
    extern evalFun alwaysTrue;
    /*
     * note:
     * LinkRule produces a vector, containing things. if only one in it, it was assigned to be the one. if none, it produces none
     * KneeRule produces a vector, containing more same things. if all void rule in it, it produces none
     * OptionRule is just a wrapper for delegated one. when matched, it will call a transfor,where u can produces a variant or do other
     * but, if the delegated one does not produce anything, then it do none
     * _oneTokenRule is like qi::alpha, qi::num, _stringRule is like qi::string, _unusedTokenRule is like qi::unused
     * note:
     * linkRule produces the reversed links, so you can visit in reversed order
     */
    class _Rule : public std::enable_shared_from_this<_Rule>{
    public:
      virtual evalFun compile(const evalFun& next);
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next);
      transformer transform;    
      void setTransform(transformer tr);
      int weight = 1;
#ifdef DEBUG
      std::string name;
#endif
    };
    class _LinkRule : public _Rule {
    public:
      _LinkRule(RulePtr ptr1, RulePtr ptr2) : rules{ptr1, ptr2}{  weight = ptr1->ptr->weight + ptr2->ptr->weight;    };
      _LinkRule(_Rule* ptr1, _Rule* ptr2) : rules{RulePtr(new _rule_wrapper{std::shared_ptr<_Rule>(ptr1)}), RulePtr(new _rule_wrapper{std::shared_ptr<_Rule>(ptr2)})}
      {  weight = ptr1->weight + ptr2->weight;    };
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      std::vector<RulePtr> rules;
    };
    class _KneeRule : public _Rule {
    public:    
      _KneeRule(RulePtr ptr) : r(ptr){  weight = ptr->ptr->weight; };
      _KneeRule(_Rule* ptr) : r(new _rule_wrapper{ std::shared_ptr<_Rule>(ptr) }){ weight = ptr->weight; };
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      RulePtr r;
    };
    class _OptionRule : public _Rule {
    public:      
      _OptionRule(RulePtr ptr1, RulePtr ptr2) : rules{ptr1, ptr2}{  weight = ptr1->ptr->weight? ptr1->ptr->weight : ptr2->ptr->weight;    };
      _OptionRule(_Rule* ptr1, _Rule* ptr2) : rules{RulePtr(new _rule_wrapper{std::shared_ptr<_Rule>(ptr1)}), RulePtr(new _rule_wrapper{std::shared_ptr<_Rule>(ptr2)})}
      {  weight = ptr1->weight? ptr1->weight : ptr2->weight;    };
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      std::vector<RulePtr> rules;
    };
    class _oneTokenRule : public _Rule {
    public:
      _oneTokenRule(tokenID idt) : id(idt){};
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      tokenID id;
    };
    class _stringRule : public _Rule {
    public:
      _stringRule(std::string& idt) : id(idt){};
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      std::string id;
    };
    class _unusedTokenRule : public _Rule {
    public:
      _unusedTokenRule(std::string& idt) : id(idt){ weight = 0; };
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      std::string id;
    };
    template<typename T>
    class _attrRule : public _Rule {
    public:
      _attrRule(T& t);
      virtual evalFun compile(const evalFun& next) override;
      virtual bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next) override;
      const T attr;
    };
    
    template<typename T>
    _attrRule<T>::_attrRule(T& t) : attr(t)
    {
    }
    
    class RulePrinter;
    template<typename T>
    bool _attrRule<T>::parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next)
    {
      RulePrinter(this);
      if( next(start, end, tks, pd) )
      {
	pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
	pds->push_back(production{std::shared_ptr<void>(static_cast<void*>(new T(attr)))});
	return true;
      }
      return false;
    };

    template<typename T>
    evalFun _attrRule<T>::compile(const evalFun& next)
    {
    return jrun::parser::_Rule::compile(next);
    }

    class LinkRule; class KneeRule; class OptionRule;
    /*
     * note:
     * when constructing rule from the same type rule, we should use shared_ptr as arg, such as LinkRule::operator>>
     * when constructing new type rule from different rule, we can use raw ptr as arg, such as Rule::operator>>
     * every Rule object should prodce a pd
     */
    class Rule {
    public:
      Rule() : Rule(new _Rule()){};
      Rule(_Rule* r) : ptr((new _rule_wrapper{std::shared_ptr<_Rule>(r)})) {  };
      Rule(std::shared_ptr<_Rule> r) : ptr((new _rule_wrapper{r})) {  };
      Rule(RulePtr r) : ptr(r){};
      Rule(_Rule* r, transformer tr) { ptr->ptr = std::shared_ptr<_Rule>(r); ptr->ptr->setTransform(tr); };
      Rule(Rule&& r) : ptr(r.ptr){};
      Rule(Rule& r) : ptr(r.ptr){};
      void setTransform(transformer tr){ ptr->ptr->setTransform(tr); };
      evalFun compile(evalFun next) { return ptr->ptr->compile(next); };
      bool parse(offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd, const evalFun& next)
      {	return ptr->ptr->parse(start, end, tks, pd, next); };
      LinkRule operator>>(Rule r);
      LinkRule operator>>(LinkRule r);
      LinkRule operator>>(const char* c);
      KneeRule operator*();
      OptionRule operator||(Rule r);
      OptionRule operator||(OptionRule r);
      Rule& operator[](transformer tr){ ptr->ptr->setTransform(tr); return *this; };
      Rule& operator=(Rule& r);
      Rule& operator=(Rule&& r);
      RulePtr ptr;
#ifdef DEBUG
      void setName(std::string&& str){ ptr->ptr->name = std::move<>(str); };
      void setName(std::string& str){ ptr->ptr->name = (str); };
#endif      
    };
    class LinkRule : public Rule {
    public:
      LinkRule(_LinkRule* r) : Rule(r){};
      LinkRule(LRulePtr r) : Rule(r){};
      LinkRule(_LinkRule* r, transformer tr) : Rule(r, tr){};
      LinkRule operator>>(Rule r);
      LinkRule operator>>(LinkRule r);
      LinkRule operator>>(const char* c);
      LinkRule& operator[](transformer tr){ ptr->ptr->setTransform(tr); return *this; };
    };
    class KneeRule : public Rule {
    public:
      KneeRule(_KneeRule* r) : Rule(r){};
      KneeRule(KRulePtr r) : Rule(r){};
      KneeRule(_KneeRule* r, transformer tr) : Rule(r, tr){};
      KneeRule& operator[](transformer tr){ ptr->ptr->setTransform(tr); return *this; };
    };
    class OptionRule : public Rule {
    public:
      OptionRule(_OptionRule* r) : Rule(r){};
      OptionRule(ORulePtr r) : Rule(r){};
      OptionRule(_OptionRule* r, transformer tr) : Rule(r, tr){};
      OptionRule operator||(Rule r);
      OptionRule operator||(OptionRule r);
      OptionRule& operator[](transformer tr){ ptr->ptr->setTransform(tr); return *this; };
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
    template<typename T>
    class AttrRule : public Rule {
    public:
      AttrRule(T& t) : Rule(new _attrRule<T>(t)){};
      AttrRule(T&& t) : Rule(new _attrRule<T>(t)){};
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
    
    class RulePrinter {
    public:
      RulePrinter(_Rule* ptr);
      ~RulePrinter();
    private:
#ifdef JRUN_PARSER_DEBUG
      static int count;
      _Rule* r;
#endif      
    };
  }
}

#endif