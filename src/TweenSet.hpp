#pragma once

//
// 複数のTweenHandlerをひとまとめにして扱う
//

#include <map>
#include "TweenHandler.hpp"


namespace ngs {

class TweenSet
{
  std::map<std::string, TweenHandler> handlers_;

  
public:
  TweenSet(const ci::JsonTree& params) noexcept
  {
    for (const auto& p : params)
    {
      handlers_.emplace(std::piecewise_construct,
                        std::forward_as_tuple(p.getValueForKey<std::string>("id")),
                        std::forward_as_tuple(Params::load(p.getValueForKey<std::string>("handle"))));
    }
  }

  template<typename T>
  void start(const std::string& id, const ci::TimelineRef& timeline, T* object) noexcept
  {
    auto& handle = handlers_.at(id);
    handle.start(timeline, object);
  }
  
};

}
