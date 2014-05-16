#ifndef JRUN_PARSER_TRANSFORM_HPP
#define JRUN_PARSER_TRANSFORM_HPP

#include <memory>
#include "parser.hpp"

namespace jrun{
  namespace transform{
    
    using jrun::parser::transformer;
    using jrun::parser::production;
    using jrun::parser::pdsPtr;
    using jrun::parser::pdsType;

 //T must support [] operator
template <typename T, typename U>
transformer transformer_for_rule_Knee()
{
  return [](production& pd, const int){
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  int sz = pds->size();
  T* ns = new T(sz);
  int i = 0;
  for(auto npd : *pds)
  {
    (*ns)[i] = (*static_cast<U*>(npd.result.get()));
    ++i;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(ns));
  };
}

// *(a >> ".") >> a
template <typename T, typename U>
transformer transformer_for_rule_Knee_Separated()
{
  return [](production& pd, const int){
  pdsPtr pds0 = std::static_pointer_cast<pdsType>(pd.result);
  pdsPtr pds = std::static_pointer_cast<pdsType>((pds0->back()).result);
  int sz = pds->size();
  T* ns = new T(sz+1);
  int i = 0;
  for(auto npd : *pds)
  {
    (*ns)[i] = (*static_cast<U*>(npd.result.get()));
    ++i;
  }
  (*ns)[sz] = (*static_cast<U*>((pds0->front()).result.get()));

  pd.result = std::shared_ptr<void>(static_cast<void*>(ns));
  };
}
    
template <typename T, typename U>
transformer transformer_for_rule1()
{
  return [](production& pd, const int){
  U* key = static_cast<U*>(pd.result.get());
  T* m = new T{*key};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
  };
}

template <typename T, typename U1, typename U2>
transformer transformer_for_rule2()
{
  return [](production& pd, const int){
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  U2* key = static_cast<U2*>((pds->front()).result.get());
  U1* map = static_cast<U1*>((pds->back()).result.get());
  T* m = new T{*map, *key};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
  };
}

template <typename T, typename U1, typename U2, typename U3>
transformer transformer_for_rule3()
{
  return [](production& pd, const int){
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  U3* key = static_cast<U3*>((pds->front()).result.get());
  U2* middle = static_cast<U2*>((pds->at(1)).result.get());
  U1* map = static_cast<U1*>((pds->back()).result.get());
  T* m = new T{*map, *middle, *key};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
  };
}

template <typename T, typename U1, typename U2, typename U3, typename U4>
transformer transformer_for_rule4()
{
  return [](production& pd, const int){
  pdsPtr pds = std::static_pointer_cast<pdsType>(pd.result);
  U4* key = static_cast<U4*>((pds->front()).result.get());
  U3* middle3 = static_cast<U3*>((pds->at(1)).result.get());
  U2* middle2 = static_cast<U2*>((pds->at(2)).result.get());
  U1* map = static_cast<U1*>((pds->back()).result.get());
  T* m = new T{*map, *middle2, *middle3, *key};
  pd.result = std::shared_ptr<void>(static_cast<void*>(m));
  };
}

template <typename T, typename U1, typename U2>
transformer transformer_for_rule2_Option(){
return [](production& pd, const int n)
{
  T* l = new T;
  switch(n)
  {
    case 0:
      *l = *(static_cast<U1*>(pd.result.get()));
      break;
    case 1:
      *l = *(static_cast<U2*>(pd.result.get()));
      break;
    default:
      return;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(l));
};
}

template <typename T, typename U1, typename U2, typename U3>
transformer transformer_for_rule3_Option(){
return [](production& pd, const int n)
{
  T* l = new T;
  switch(n)
  {
    case 0:
      *l = *(static_cast<U1*>(pd.result.get()));
      break;
    case 1:
      *l = *(static_cast<U2*>(pd.result.get()));
      break;
    case 2:
      *l = *(static_cast<U3*>(pd.result.get()));
      break;
    default:
      return;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(l));
};
}

template <typename T, typename U1, typename U2, typename U3, typename U4>
transformer transformer_for_rule4_Option(){
return [](production& pd, const int n)
{
  T* l = new T;
  switch(n)
  {
    case 0:
      *l = *(static_cast<U1*>(pd.result.get()));
      break;
    case 1:
      *l = *(static_cast<U2*>(pd.result.get()));
      break;
    case 2:
      *l = *(static_cast<U3*>(pd.result.get()));
      break;
    case 3:
      *l = *(static_cast<U4*>(pd.result.get()));
      break;
    default:
      return;
  }
  pd.result = std::shared_ptr<void>(static_cast<void*>(l));
};
}
    
//end of namespace
  }
}

#endif