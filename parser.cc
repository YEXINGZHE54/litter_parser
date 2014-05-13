#include "parser.hpp"
#include <memory>
#include <utility>

namespace jrun{
  namespace parser{
    evalFun alwaysTrue = [](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd){
      return true;
    };
  }
}

using namespace jrun::parser;

void _Rule::setTransform(transformer tr)
{
  transform = tr;
}

evalFun _Rule::compile(const evalFun& next)
{
  return next;
}

// _Knee can be recognized as kind of a link, so we make a link pd for it in parsing
evalFun _KneeRule::compile(const evalFun& next)
{
  evalFun e = r->compile(alwaysTrue);
  evalFun ev = [e,next,this](offset& start1, const offset end1, const std::vector<TokenPtr>& tks1, production& pd1)->bool{
    std::shared_ptr< std::vector<production> > result(new std::vector<production>());
    
    evalFun ev2 = [e, next, &ev2, &result, this](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd)->bool{
      production npd;
      npd.result = std::static_pointer_cast<void>(result);	//here npd represents the pd of current link, pd is the parent one
      offset cs = start;
      if( e(start, end, tks, npd) )
      {
	if( ev2(start, end, tks, pd) )
	{
	  return true;
	}	
      }
      start = cs;
      if(next(start, end, tks, pd))	return true;
      return false;
    };
    
    if( ev2(start1, end1, tks1, pd1) )
    {
      if(weight != 0)	//means it expects productions
      {
	production npd{ std::static_pointer_cast<void>(result) };
	if(transform) transform(npd, 0);
	std::shared_ptr< std::vector<production> > re = std::static_pointer_cast< std::vector<production> >(pd1.result);
	re->push_back(npd);
      }
      return true;
    }
    return false;
  };
  return ev;
}

evalFun _LinkRule::compile(const evalFun& next)
{
  evalFun ev, evn;
  RulePtr rend = rules.back();
  ev = rend->compile(alwaysTrue);
  evn = [ev, next](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& npd)->bool{      
      if(ev(start, end, tks, npd))
      {	
	std::shared_ptr< std::vector<production> > re = std::static_pointer_cast< std::vector<production> >(npd.result);
	production pd = re->front();	//here pd mean parent link pd of current link
	if( next(start, end, tks, pd) )
	{
	  re->erase(re->begin());
	  return true;
	}
      }
      return false;
  };
  
  for(int i = rules.size()-2; i >0; --i)
  {
    RulePtr r = rules.at(i);
    evn = r->compile(evn);    
  }
  RulePtr r = rules.at(0);
  ev = r->compile(evn);
  evn = [ev, this](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd)->bool
  {	//here pd is parent link pd of link
      std::shared_ptr< std::vector<production> > result(new std::vector<production>());
      production npd;	npd.result = std::static_pointer_cast<void>(result);
      result->push_back(pd);
      if(ev(start, end, tks, npd))	//here npd is the link pd of current linkrule
      {
	if(result->empty())	return true;	//without adding it to stack
	if(result->size() == 1)	npd.result = (result->front()).result;
	if(transform) transform(npd, 0);
	std::shared_ptr< std::vector<production> > re = std::static_pointer_cast< std::vector<production> >(pd.result);
	re->push_back(npd);
	return true;
      }
      return false;
  };
  return evn;
}

evalFun _oneTokenRule::compile(const evalFun& next)
{
  tokenID mid = id;
  evalFun evn = [mid, next](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd)->bool
  {
    if(start == end) return false;
    TokenPtr tk = tks[start];
    if(tk->id == mid)
    {
      ++start;
      if(!next(start, end, tks, pd))
      {
	return false;
      }
      std::shared_ptr< std::vector<production> > re = std::static_pointer_cast< std::vector<production> >(pd.result);
      re->push_back(production{std::shared_ptr<void>(static_cast<void*>(new std::string(tk->tok)))});
      return true;
    }
    return false;
  };
  return evn;
}

evalFun _stringRule::compile(const evalFun& next)
{
  std::string mid = id;
  evalFun evn = [mid, next](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd)->bool
  {
    if(start == end) return false;
    TokenPtr tk = tks[start];
    if(tk->id >= jrun::parser::KEYS && tk->tok == mid)
    {
      ++start;
      if(!next(start, end, tks, pd))
      {
	return false;
      }
      std::shared_ptr< std::vector<production> > re = std::static_pointer_cast< std::vector<production> >(pd.result);
      re->push_back(production{std::shared_ptr<void>(static_cast<void*>(new std::string(tk->tok)))});
      return true;
    }
    return false;
  };
  return evn;
}

evalFun _OptionRule::compile(const evalFun& next)
{
  int sz = rules.size();
  std::vector<evalFun> evs(sz);
  for(int i = 0; i < sz; ++i)
  {
    evs[i] = (rules.at(i))->compile(alwaysTrue);
  }
  evalFun evn = [evs, next, this](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd)->bool
  {
    const offset cs = start;	int count = 0;
    std::shared_ptr< std::vector<production> > pds = std::static_pointer_cast< std::vector<production> >(pd.result);     
    const std::size_t cur = pds->size();
    for(std::vector<evalFun>::const_iterator it = evs.begin(); it != evs.end(); ++it)
    {
      start = cs;      
      if( (*it)(start, end, tks, pd) )
      {
	if(pds->size() > cur)	//option rule produces a thing
	{
	  production& npd = pds->back();
	  if(transform) transform(npd, count);
	}
	if(next(start, end, tks, pd)) return true;
      }
      ++count;
    }
    return false;
  };
  return evn;
}

evalFun _unusedTokenRule::compile(const evalFun& next)
{
  std::string mid = id;
  evalFun evn = [mid, next](offset& start, const offset end, const std::vector<TokenPtr>& tks, production& pd)->bool
  {
    if(start == end) return false;
    TokenPtr tk = tks[start];
    if(tk->id >= jrun::parser::KEYS && tk->tok == mid)
    {
      ++start;
      if(!next(start, end, tks, pd))
      {
	return false;
      }
      return true;
    }
    return false;
  };
  return evn;
}

LinkRule::LinkRule(LRulePtr r): Rule(r)
{
  //ptr->transform = listCollector;
}

KneeRule::KneeRule(_KneeRule* r) : Rule(r){
	//ptr->transform = listCollector;
};

//transformer oneCollector = [](const std::vector<production>& pds)->production{ return pds.front(); };

OptionRule::OptionRule(_OptionRule* r): Rule(r)
{
  //ptr->transform = oneCollector;
}

OptionRule::OptionRule(ORulePtr r): Rule(r)
{

}
      
BasicRule::BasicRule(tokenID id) : Rule(new _oneTokenRule(id)){
	//ptr->transform = oneCollector;
};

StringRule::StringRule(std::string id) : Rule(new _stringRule(id)){
	//ptr->transform = oneCollector;
};

VoidRule::VoidRule(std::string id): Rule(new _unusedTokenRule(id))
{

}
      
KneeRule Rule::operator*()
{
  return KneeRule(new _KneeRule(ptr));
}

LinkRule Rule::operator>>(Rule r)
{
  return LinkRule(new _LinkRule(ptr, r.ptr));
}

OptionRule Rule::operator||(Rule r)
{
  return OptionRule(new _OptionRule(ptr, r.ptr));
}

LinkRule Rule::operator>>(LinkRule r)
{
  LRulePtr p = std::static_pointer_cast<_LinkRule>(r.ptr);
  p->rules.insert(p->rules.begin(), ptr);
  p->weight += ptr->weight;
  return LinkRule(p);
}

LinkRule Rule::operator>>(const char* c)
{
  return this->operator>>(VoidRule(c));
}

OptionRule Rule::operator||(OptionRule r)
{
  ORulePtr p = std::static_pointer_cast<_OptionRule>(r.ptr);
  p->rules.insert(p->rules.begin(), ptr);
  if(p->weight == 0) p->weight = ptr->weight;
  return OptionRule(p);
}

LinkRule LinkRule::operator>>(Rule r)
{
  LRulePtr p = std::static_pointer_cast<_LinkRule>(ptr);
  p->rules.push_back(r.ptr);
  p->weight += r.ptr->weight;
  return LinkRule(p);
}

LinkRule LinkRule::operator>>(LinkRule r)
{
  LRulePtr p1 = std::static_pointer_cast<_LinkRule>(ptr);
  LRulePtr p2 = std::static_pointer_cast<_LinkRule>(r.ptr);
  p1->rules.insert(p1->rules.end(), p2->rules.begin(), p2->rules.end());
  p1->weight += p2->weight;
  return LinkRule(p1);
}

LinkRule LinkRule::operator>>(const char* c)
{
  return this->operator>>(VoidRule(c));
}

OptionRule OptionRule::operator||(OptionRule r)
{
  ORulePtr p1 = std::static_pointer_cast<_OptionRule>(ptr);
  ORulePtr p2 = std::static_pointer_cast<_OptionRule>(r.ptr);
  p1->rules.insert(p1->rules.end(), p2->rules.begin(), p2->rules.end());
  if(p1->weight == 0) p1->weight = p2->weight;
  return OptionRule(p1);
}

OptionRule OptionRule::operator||(Rule r)
{
  ORulePtr p = std::static_pointer_cast<_OptionRule>(ptr);
  p->rules.push_back(r.ptr);
  if(p->weight == 0) p->weight = r.ptr->weight;
  return OptionRule(p);
}

bool PDA::parse(Rule& r, production& pd)
{
  offset start = 0;
  offset end = sequences.size();
  evalFun ev = r.compile(alwaysTrue);
  std::shared_ptr< std::vector<production> > result(new std::vector<production>());
  pd.result = std::static_pointer_cast<void>(result);
  if( ev(start, end, sequences, pd) )
  {
#ifdef DEBUG
    if(result->size() == 2) throw 1;
#endif
    if(result->size() == 1) pd.result = (result->front()).result;    
    return true;
  }
  return false;
}