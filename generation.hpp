#ifndef JRUN_GENERATION_HPP
#define JRUN_GENERATION_HPP

#include <vector>
#include <boost/iterator/iterator_concepts.hpp>
#include <boost/variant/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

namespace jrun{
  namespace generation{
    using namespace std;    

    typedef std::string literValue;
    struct funCall; struct commands; struct AnnoFunc;
    typedef std::vector<std::string> names;
    struct mapKey {
      names map;
      names key;
    };
    struct mapConst {
      names map;
      literValue key;
    };
    typedef boost::variant<
      mapKey, mapConst,  names
    > leftValue;
    struct propertyAssign;
    struct Objectdef{
      std::vector<propertyAssign> properties;
    };
    struct dOpValue; struct sOpValue;
    typedef boost::variant<
     boost::recursive_wrapper<AnnoFunc>, 
     boost::recursive_wrapper<dOpValue>, boost::recursive_wrapper<sOpValue>, 
     boost::recursive_wrapper<funCall>, boost::recursive_wrapper<Objectdef>,
     leftValue, literValue 
    > rightValue;
    typedef boost::variant<
     boost::recursive_wrapper<funCall>,
     leftValue, literValue 
    > tValue;
    typedef boost::variant<
      rightValue, tValue
    > wrappedRightValue;
    struct dOpValue { //exclude =
      wrappedRightValue first;	std::string op;	rightValue second;	bool grouped;
    };
    struct sOpValue {
      tValue first; std::string op;
    };
    typedef std::vector<rightValue> RealArgs;
    struct funCall {
      leftValue name;
      RealArgs args;
    };   
    
    struct Assign; struct NamedFunc;
    typedef boost::variant<
      boost::recursive_wrapper<Assign>, rightValue
    > Expr;
    struct Assign {
      leftValue key;
      rightValue value;
    };    
    
    struct retCommand {
      rightValue exr;
    };
    
    struct ifCommand;	struct forCommand; struct whileCommand;	struct breakCommand;
    typedef boost::variant<
      boost::recursive_wrapper<NamedFunc>, retCommand, boost::recursive_wrapper<ifCommand>, 
      boost::recursive_wrapper<forCommand>, boost::recursive_wrapper<whileCommand>, 
      breakCommand, Expr
    > mCommand;
    struct propertyAssign {
      std::string key;
      rightValue value;
    };     
    
    typedef std::vector<std::string> virtualArgs;
    struct AnnoFunc {
      virtualArgs argsv;
      std::vector<mCommand> commands;
    };
    struct NamedFunc {
      std::string name;
      virtualArgs argsv;
      std::vector<mCommand> commands;
    };       
    struct ifCommand {
      Expr e;
      std::vector<mCommand> commands;
      std::vector<mCommand> elsecoms;
    };
    struct forCommand {
      std::vector<Expr> start;
      Expr condition;
      std::vector<Expr> after;
      std::vector<mCommand> commands;
    };
    struct whileCommand {
      Expr condition;
      std::vector<mCommand> commands;
    };
    struct breakCommand {
      bool notUsed;
    };
    
    struct AST {
      std::vector<mCommand> commands;
    };
  }
}

#endif