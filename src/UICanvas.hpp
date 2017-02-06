#pragma once

//
// UI管理
//  TODO:CameraPerspにも対応
//

#include <cinder/Camera.h>
#include "UIWidget.hpp"


namespace ngs { namespace UI {

class Canvas
{
  ci::CameraOrtho camera_;

  ci::vec2 size_;
  ci::Rectf rect_;

  UI::WidgetPtr root_widget_;


public:
  Canvas() noexcept
  {
    size_ = ci::app::getWindowSize();

    // 画面中央が原点。右方向がX軸プラス、上方向がY軸プラスの座標系
    camera_.setOrtho(-size_.x / 2.0f, size_.x / 2.0f,
                     -size_.y / 2.0f, size_.y / 2.0f,
                     -1.0f, 100.0f);

    rect_ = ci::Rectf(-size_.x / 2.0f, -size_.y / 2.0f, size_.x / 2.0f, size_.y / 2.0f);
  }

  void setWidgets(const UI::WidgetPtr& root_widget) noexcept
  {
    root_widget_ = root_widget;
  }

  Widget* findWidget(const std::string& identifier) noexcept
  {
    return root_widget_->find(identifier);
  }


  void resize(const ci::vec2& size) noexcept
  {
    size_ = size;

    camera_.setOrtho(-size_.x / 2.0f, size_.x / 2.0f,
                     -size_.y / 2.0f, size_.y / 2.0f,
                     -1.0f, 100.0f);

    rect_ = ci::Rectf(-size_.x / 2.0f, -size_.y / 2.0f, size_.x / 2.0f, size_.y / 2.0f);
  }


  void touchBegan(const Touch& touch)
  {
    ci::vec2 scale{ 1.0f, 1.0f };
    root_widget_->touchBegan(touch, rect_, scale);
  }

  void touchMoved(const Touch& touch)
  {
    ci::vec2 scale{ 1.0f, 1.0f };
    root_widget_->touchMoved(touch, rect_, scale);
  }

  void touchEnded(const Touch& touch)
  {
    ci::vec2 scale{ 1.0f, 1.0f };
    root_widget_->touchEnded(touch, rect_, scale);
  }


  void draw() noexcept
  {
    ci::gl::setMatrices(camera_);

    ci::vec2 scale{ 1.0f, 1.0f };
    root_widget_->draw(rect_, scale);
  }

};

} }
