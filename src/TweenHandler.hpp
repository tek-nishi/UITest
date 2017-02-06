#pragma once

//
// TweenClipをひとまとめにして扱う
//

#include <vector>
#include <boost/optional.hpp>
#include "TweenClip.hpp"


namespace ngs {

class TweenHandler
{
  struct Property {
    boost::optional<std::string> identifier;
    std::string target;
    TweenClip clip;
  };

  std::vector<Property> properties_;
  

public:
  TweenHandler(const ci::JsonTree& params) noexcept
  {
    for (const auto& p : params)
    {
      Property property = {
        boost::none,
        p.getValueForKey<std::string>("target"),
        TweenClip(p["clip"]),
      };

      if (p.hasChild("id"))
      {
        property.identifier = p.getValueForKey<std::string>("id");
      }

      properties_.push_back(std::move(property));
    }
  }

  template<typename T>
  void start(const ci::TimelineRef& timeline, T* object) noexcept
  {
    for (auto& p : properties_)
    {
      if (p.identifier)
      {
        T* child = object->find(*p.identifier);
        
        float *target = child->getParam(p.target);
        p.clip.start(timeline, target);
      }
      else
      {
        float* target = object->getParam(p.target);
        p.clip.start(timeline, target);
      }
    }
  }
  
};
  
}
