#pragma once

//
// UI::Widgetのアニメーション処理
//

#include <boost/optional.hpp>
#include <vector>
#include "EasingUtil.hpp"


namespace ngs { namespace UI {

class Tween
{
  template <typename T>
  struct Param
  {
    ci::EaseFn ease;

    boost::optional<T> start;
    boost::optional<T> end;

    float duration;
    boost::optional<float> delay;

    bool ping_pong;
    bool loop;
  };

  std::vector<Param<ci::vec2>>   position_;
  std::vector<Param<ci::vec2>>   scale_;
  std::vector<Param<ci::ColorA>> color_;


  template <typename T>
  static Param<T> createParam(const ci::JsonTree& params) noexcept
  {
    Param<T> p;

    p.ease = getEaseFunc(params.getValueForKey<std::string>("type"));

    if (params.hasChild("start"))
    {
      p.start = Json::getVec<T>(params["start"]);
    }
    if (params.hasChild("end"))
    {
      p.end = Json::getVec<T>(params["end"]);
    }

    p.duration = params.getValueForKey<float>("duration");
    if (params.hasChild("delay"))
    {
      p.delay = params.getValueForKey<float>("delay");
    }

    p.ping_pong = Json::getValue(params, "ping_pong", false);
    p.loop      = Json::getValue(params, "loop", false);

    return p;
  }

  template <typename T>
  static void apply(const ci::TimelineRef& timeline, ci::Anim<T>& target, const Param<T>& param) noexcept
  {
    auto option = param.start ? timeline->apply(&target, *param.start, *param.end, param.duration, param.ease)
                              : timeline->apply(&target, *param.end, param.duration, param.ease);

    if (param.delay) option.delay(*param.delay);
    option.loop(param.loop);
    option.pingPong(param.ping_pong);
  }

  template <typename T>
  static void append(const ci::TimelineRef& timeline, ci::Anim<T>& target, const Param<T>& param) noexcept
  {
    auto option = param.start ? timeline->appendTo(&target, *param.start, *param.end, param.duration, param.ease)
                              : timeline->appendTo(&target, *param.end, param.duration, param.ease);

    if (param.delay) option.delay(*param.delay);
    option.loop(param.loop);
    option.pingPong(param.ping_pong);
  }

  template <typename T>
  static void applyAndAppend(const ci::TimelineRef& timeline,
                             ci::Anim<T>& target, const std::vector<Param<T>>& params) noexcept
  {
    apply(timeline, target, params[0]);

    for (size_t i = 1; i < params.size(); ++i)
    {
      append(timeline, target, params[i]);
    }
  }



public:
  Tween(const ci::JsonTree& params) noexcept
  {
    for (const auto& p : params)
    {
      std::string target = p.getValueForKey<std::string>("target");
      if (target == "position")
      {
        // 位置のtween
        position_.push_back(createParam<ci::vec2>(p));

        if (p.hasChild("append"))
        {
          for (const auto& append : p["append"])
          {
            position_.push_back(createParam<ci::vec2>(append));
          }
        }
      }
      else if (target == "scale")
      {
        scale_.push_back(createParam<ci::vec2>(p));

        if (p.hasChild("append"))
        {
          for (const auto& append : p["append"])
          {
            scale_.push_back(createParam<ci::vec2>(append));
          }
        }
      }
      else if (target == "color")
      {
        color_.push_back(createParam<ci::ColorA>(p));

        if (p.hasChild("append"))
        {
          for (const auto& append : p["append"])
          {
            color_.push_back(createParam<ci::ColorA>(append));
          }
        }
      }
    }
  }


  bool hasPositionTweens() const noexcept
  {
    return !position_.empty();
  }

  bool hasScaleTweens() const noexcept
  {
    return !scale_.empty();
  }

  bool hasColorTweens() const noexcept
  {
    return !color_.empty();
  }


  void start(const ci::TimelineRef& timeline,
             ci::Anim<ci::vec2>& position,
             ci::Anim<ci::vec2>& scale,
             ci::Anim<ci::ColorA>& color) const noexcept
  {
    if (hasPositionTweens())
    {
      applyAndAppend(timeline, position, position_);
    }

    if (hasScaleTweens())
    {
      applyAndAppend(timeline, scale, scale_);
    }

    if (hasColorTweens())
    {
      applyAndAppend(timeline, color, color_);
    }
  }

};

} }
