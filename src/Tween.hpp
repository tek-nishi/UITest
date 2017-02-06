#pragma once

//
// 簡易アニメーション
//

#include <boost/optional.hpp>
#include "EasingUtil.hpp"


namespace ngs {

class Tween
{
  ci::EaseFn ease_;

  boost::optional<float> start_;
  boost::optional<float> end_;

  float duration_;
  boost::optional<float> delay_;
  
  bool ping_pong_;
  bool loop_;
  

public:
  Tween(const ci::JsonTree& params) noexcept
    : ease_(getEaseFunc(params.getValueForKey<std::string>("type"))),
      duration_(params.getValueForKey<float>("duration")),
      ping_pong_(Json::getValue(params, "ping_pong", false)),
      loop_(Json::getValue(params, "loop", false))
  {
    if (params.hasChild("start"))
    {
      start_ = params.getValueForKey<float>("start");
    }
    if (params.hasChild("end"))
    {
      end_ = params.getValueForKey<float>("end");
    }

    if (params.hasChild("delay"))
    {
      delay_ = params.getValueForKey<float>("delay");
    }
  }

  void apply(const ci::TimelineRef& timeline, float* target) noexcept
  {
    auto option = start_ ? timeline->applyPtr(target, *start_, *end_, duration_, ease_)
                         : timeline->applyPtr(target, *end_, duration_, ease_);

    if (delay_) option.delay(*delay_);
    option.loop(loop_);
    option.pingPong(ping_pong_);
  }
  
  void append(const ci::TimelineRef& timeline, float* target) noexcept
  {
    auto option = start_ ? timeline->appendToPtr(target, *start_, *end_, duration_, ease_)
                         : timeline->appendToPtr(target, *end_, duration_, ease_);

    if (delay_) option.delay(*delay_);
    option.loop(loop_);
    option.pingPong(ping_pong_);
  }
};

}
