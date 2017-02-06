#pragma once

//
// Tweenをひとまとめにしたクラス
//

#include <vector>
#include "Tween.hpp"


namespace ngs {

class TweenClip
{
  std::vector<Tween> tweens_;


public:
  TweenClip(const ci::JsonTree& params) noexcept
  {
    for (const auto& p : params)
    {
      tweens_.emplace_back(p);
    }
  }


  // Tween再生開始
  void start(const ci::TimelineRef& timeline, float* target) noexcept
  {
    tweens_[0].apply(timeline, target);
    for (u_int i = 1; i < tweens_.size(); ++i)
    {
      tweens_[i].append(timeline, target);
    }
  }
  
};

}
