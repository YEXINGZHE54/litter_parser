#include "parser.hpp"
#include "generation.hpp"
#include "grammar.hpp"
#include "transformer.hpp"
#include <utility>
#include <functional>

using namespace jrun::parser;
using namespace jrun::generation;

void transformer_for_names(production& pd, const int)
{
  pdsPtr pds0 = std::static_pointer_cast<pdsType>(pd.result);
  pdsPtr pds = std::static_pointer_cast<pdsType>((pds0->back()).result);
  int sz = pds->size();
  names* ns = new names(sz+1);
  int i = 0;
  for(auto npd : *pds)
  {
    (*ns)[i] = std::move(*static_cast<std::string*>(npd.result.get()));
    ++i;
  }
  (*ns)[sz] = std::move(*static_cast<std::string*>((pds0->front()).result.get()));

  pd.result = std::shared_ptr<void>(static_cast<void*>(ns));
}

void transformer_for_mapKey(production& pd, const int)
{
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  if(pds->size() != 2) {
    //debug it
    return;
  }
  names* key = static_cast<names*>((pds->front()).result.get());
  names* map = static_cast<names*>((pds->back()).result.get());
  mapKey* m = new mapKey{std::move(*map), std::move(*key)};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
}

void transformer_for_mapConst(production& pd, const int)
{
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  if(pds->size() != 2) {
    //debug it
    return;
  }
  std::string* key = static_cast<std::string*>((pds->front()).result.get());
  names* map = static_cast<names*>((pds->back()).result.get());
  mapConst* m = new mapConst{std::move(*map), std::move(*key)};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
}

void transformer_for_leftValue(production& pd, const int n)
{
  leftValue* l = new leftValue;
  switch(n)
  {
    case 0:
      *l = *(static_cast<mapKey*>(pd.result.get()));
      break;
    case 1:
      *l = *(static_cast<mapConst*>(pd.result.get()));
      break;
    case 2:
      *l = *(static_cast<names*>(pd.result.get()));
      break;
    default:
      return;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(l));
}

using namespace jrun::grammar;
using namespace jrun::transform;

mGrammar::mGrammar(): Rule()
{
    Rule rightRule;
    Rule namesRule = *( rule(ID) >> "." ) >> rule(ID);
    namesRule.setTransform(std::bind<>(transformer_for_names, std::placeholders::_1, std::placeholders::_2));
    Rule mapconstRule = namesRule >> "[" >> rule(STRING) >> "]";
    mapconstRule.setTransform(transformer_for_rule2<mapConst, names, literValue>());
    Rule mapkeyRule = namesRule >> "[" >> namesRule >> "]";
    mapkeyRule.setTransform(transformer_for_rule2<mapKey, names, names>());
    Rule leftRule = mapkeyRule || mapconstRule || namesRule;
    leftRule.setTransform(std::bind<>(transformer_for_leftValue, std::placeholders::_1, std::placeholders::_2));
    Rule propertyRule = rule(ID) >> ":" >> rightRule;
    propertyRule.setTransform(transformer_for_rule2<propertyAssign, std::string, rightValue>());
    Rule objectRule = lit("{") >> (*(propertyRule))[transformer_for_rule_Knee<std::vector<propertyAssign>, propertyAssign>()] >> "}";
    objectRule[transformer_for_rule1<Objectdef, std::vector<propertyAssign> >()];
    //objectRule.setTransform();
    ptr = namesRule.ptr;
}
