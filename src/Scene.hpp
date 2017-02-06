#pragma once

//
// シーン
// UI::CanvasやUI::WidgetとTweenをセットにして管理する
//

#include "UICanvas.hpp"
#include "TweenSet.hpp"
#include "UIWidgetsFactory.hpp"


namespace ngs {

class Scene
{
  UI::Canvas canvas_;
  TweenSet tween_set_;

  
public:
  Scene(const ci::JsonTree& params, UI::WidgetsFactory& widgets_factory) noexcept
    : canvas_(widgets_factory.construct(Params::load(params.getValueForKey<std::string>("widget")))),
      tween_set_(Params::load(params.getValueForKey<std::string>("tween")))
  {
  }


  UI::Canvas& getCanvas() noexcept
  {
    return canvas_;
  }

  const UI::Canvas& getCanvas() const noexcept
  {
    return canvas_;
  }

  TweenSet& getTweenSet() noexcept
  {
    return tween_set_;
  }

  const TweenSet& getTweenSet() const noexcept
  {
    return tween_set_;
  }
  
  
};

}
