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

void transformer_for_rightValue(production& pd, const int n)
{
  rightValue* l = new rightValue;
  switch(n)
  {
    case 0:
      *l = *(static_cast<AnnoFunc*>(pd.result.get()));
      break;
    case 1:
      *l = *(static_cast<dOpValue*>(pd.result.get()));
      break;
    case 2:
      *l = *(static_cast<sOpValue*>(pd.result.get()));
      break;
    case 3:
      *l = *(static_cast<funCall*>(pd.result.get()));
      break;
    case 4:
      *l = *(static_cast<Objectdef*>(pd.result.get()));
      break;
    case 5:
      *l = *(static_cast<leftValue*>(pd.result.get()));
      break;
    case 6:
      *l = *(static_cast<literValue*>(pd.result.get()));
      break;
    default:
      return;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(l));
}

void transformer_for_dOp(production& pd, const int)
{
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  pdsPtr pds1 = std::static_pointer_cast<pdsType>((pds->front()).result);
  bool b = *(static_cast<bool*>((pds1->front()).result.get()));
  rightValue* second = static_cast<rightValue*>((pds1->back()).result.get());
  std::string* op = static_cast<std::string*>((pds->at(1)).result.get());
  wrappedRightValue* first = static_cast<wrappedRightValue*>((pds->back()).result.get());
  dOpValue* m = new dOpValue{(*first), std::move(*op), *second, b};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
}

void transformer_for_command(production& pd, const int n)
{
  mCommand* l = new mCommand;
  switch(n)
  {
    case 0:
      *l = *(static_cast<NamedFunc*>(pd.result.get()));
      break;
    case 1:
      *l = *(static_cast<retCommand*>(pd.result.get()));
      break;
    case 2:
      *l = *(static_cast<ifCommand*>(pd.result.get()));
      break;
    case 3:
      *l = *(static_cast<forCommand*>(pd.result.get()));
      break;
    case 4:
      *l = *(static_cast<whileCommand*>(pd.result.get()));
      break;
    case 5:
      *l = *(static_cast<breakCommand*>(pd.result.get()));
      break;
    case 6:
      *l = *(static_cast<Expr*>(pd.result.get()));
      break;
    default:
      return;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(l));
}

using namespace jrun::grammar;
using namespace jrun::transform;
namespace parser = jrun::parser;

mGrammar::mGrammar(): Rule()
{
    Rule rightRule;
    Rule namesRule = *( rule(ID) >> "." ) >> rule(ID);
    namesRule.setTransform(std::bind<>(transformer_for_names, std::placeholders::_1, std::placeholders::_2));
    namesRule.setName("names");
    Rule constRule = rule(STRING) || rule(NUMBER);
    Rule mapconstRule = namesRule >> "[" >> constRule >> "]";
    mapconstRule.setTransform(transformer_for_rule2<mapConst, names, literValue>());
    mapconstRule.setName("mapc");
    Rule mapkeyRule = namesRule >> "[" >> namesRule >> "]";
    mapkeyRule.setTransform(transformer_for_rule2<mapKey, names, names>());
    mapkeyRule.setName("mapk");
    Rule leftRule = mapkeyRule || mapconstRule || namesRule;
    leftRule.setTransform(std::bind<>(transformer_for_leftValue, std::placeholders::_1, std::placeholders::_2));
    leftRule.setName("left");
    Rule propertyRule = rule(ID) >> ":" >> rightRule >> ";";
    propertyRule.setTransform(transformer_for_rule2<propertyAssign, std::string, rightValue>());
    propertyRule.setName("property");
    Rule objectRule = lit("{") >> (*(propertyRule))[transformer_for_rule_Knee<std::vector<propertyAssign>, propertyAssign>()] >> "}";
    objectRule[transformer_for_rule1<Objectdef, std::vector<propertyAssign> >()];
    objectRule.setName("object");
    Rule Afun;		Rule dOp; Rule sOp;	Rule fun;
    rightRule = Afun || dOp || sOp || fun || objectRule || leftRule || constRule;
    rightRule[std::bind(transformer_for_rightValue, std::placeholders::_1, std::placeholders::_2)];
    rightRule.setName("right");
    Rule tvRule = fun || leftRule || rule(ID);
    tvRule[transformer_for_rule3_Option<tValue, funCall, leftValue, literValue>()];
    tvRule.setName("tvRule");
    Rule wrappedRRule = ( lit("(") >> rightRule >> ")" ) || tvRule;
    wrappedRRule[transformer_for_rule2_Option<wrappedRightValue, rightValue, tValue>()];
    wrappedRRule.setName("wrappedR");
    dOp = wrappedRRule >> ( parser::string("+") ) >> ( ( rightRule >> AttrRule<bool>(false) ) || ( lit("(") >> rightRule >> lit(")") >> AttrRule<bool>(true) ) ); 
    dOp[std::bind<>(transformer_for_dOp, std::placeholders::_1, std::placeholders::_2)];
    dOp.setName("dOp");
    sOp = tvRule >> (parser::string("++") || parser::string("--"));
    sOp[transformer_for_rule2<sOpValue, tValue, std::string>()];
    sOp.setName("sop");
    Rule rargsRule = ( (*(rightRule >> ",") >> rightRule)[transformer_for_rule_Knee_Separated<RealArgs, rightValue>()] ) || AttrRule<RealArgs>(RealArgs());
    rargsRule.setName("rargs");
    fun = leftRule >> "(" >> rargsRule >> ")";
    fun[transformer_for_rule2<funCall, leftValue, RealArgs>()];
    fun.setName("fun");
    Rule assign;
    Rule expr = assign || rightRule;
    expr[transformer_for_rule2_Option<Expr, Assign, rightValue>()];
    expr.setName("expr");
    assign = leftRule >> "=" >> rightRule;
    assign[transformer_for_rule2<Assign, leftValue, rightValue>()];
    assign.setName("ass");
    Rule retRule = lit("return") >> rightRule;
    retRule[transformer_for_rule1<retCommand, rightValue>()];
    retRule.setName("ret");
    Rule NFuncRule;	Rule ifRule;	Rule forRule;	Rule whileRule;	Rule breakRule;
    Rule commandRule = NFuncRule|| retRule || ifRule || forRule || whileRule || breakRule || expr ;
    commandRule[std::bind(transformer_for_command, std::placeholders::_1, std::placeholders::_2)];
    commandRule.setName("command");
    Rule commands = *(commandRule >> ";");
    commands[transformer_for_rule_Knee<std::vector<mCommand>, mCommand>()];
    commands.setName("commands");
    Rule vargsRule0 = ( *(rule(ID) >> ",") >> rule(ID) );
    vargsRule0[transformer_for_rule_Knee_Separated<virtualArgs, std::string>()];
    Rule vargsRule = vargsRule0 || AttrRule<virtualArgs>(virtualArgs());
    vargsRule.setName("vargs");
    Afun = lit("function") >> "(" >> vargsRule >> ")" >> "{" >> commands >> "}";
    Afun[transformer_for_rule2<AnnoFunc, virtualArgs, std::vector<mCommand> >()];
    Afun.setName("Afun");
    NFuncRule =  lit("function") >> rule(ID) >> "(" >> vargsRule >> ")"
		>> "{" >> commands >> "}";
    NFuncRule[transformer_for_rule3<NamedFunc, std::string, virtualArgs, std::vector<mCommand> >()];
    NFuncRule.setName("NF");
    ifRule = lit("if") >> "(" >> expr >> ")"
		>> "{" >> commands >> "}"
		>> ("else") >>"{" >> commands >> "}";
    ifRule[transformer_for_rule3<ifCommand, Expr, std::vector<mCommand>, std::vector<mCommand> >()];
    ifRule.setName("if");
    Rule zOmExprs = (*(expr >> ",") >> expr)[transformer_for_rule_Knee_Separated<std::vector<Expr>, Expr>()] || AttrRule< std::vector<Expr> >(std::vector<Expr>());
    forRule = lit("for") >> "(" >> zOmExprs >> ";" >> expr >> ";" >> zOmExprs >> ")"
		>> "{" >> commands >> "}";
    forRule[transformer_for_rule4<forCommand, std::vector<Expr>, Expr, std::vector<Expr>, std::vector<mCommand> >()];
    forRule.setName("for");
    whileRule = lit("while") >> "(" >> expr >> ")"
		>> "{" >> commands >> "}";
    whileRule[transformer_for_rule2<whileCommand, Expr, std::vector<mCommand> >()];
    whileRule.setName("while");
    breakRule = lit("break") >> AttrRule<bool>(false);
    breakRule[transformer_for_rule1<breakCommand, bool>()];
    breakRule.setName("break");
    Rule start = commands;
    ptr = start.ptr;
}