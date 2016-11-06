#pragma once

//
// アプリの外枠
//

#include <cinder/Timeline.h>
#include <cinder/Camera.h>
#include "Event.hpp"
#include "Arguments.hpp"
#include "ConnectionHolder.hpp"
#include "Params.hpp"
#include "UIDrawer.hpp"
#include "UIWidget.hpp"


namespace ngs {

class Worker {
  // 汎用的なコールバック管理
  Event<Arguments> event_;
  ConnectionHolder holder_;

  // ゲーム内パラメーター
  ci::JsonTree params_;

  // UIなどきっかけが必要な演出用
  ci::TimelineRef timeline_;

  // UI用カメラ
  ci::CameraOrtho ui_camera_;

  
  UI::Widget root_widget_;

  bool touching_ = false;
  uint32_t touch_id_;

  bool test_ = false;
  
  static void setupUICamera(ci::CameraOrtho& camera) noexcept
  {
    auto size = ci::app::getWindowSize();

    // 画面中央が原点。右方向がX軸プラス、上方向がY軸プラスの座標系
    camera.setOrtho(-size.x / 2.0f, size.x / 2.0f,
                    -size.y / 2.0f, size.y / 2.0f,
                    -1.0f, 100.0f);
  }

  
public:
  Worker() noexcept
  : params_(Params::load("params.json")),
    timeline_(ci::Timeline::create()),
    root_widget_("root", ci::vec2(), ci::app::getWindowSize(), UI::Drawer::fillRect)
  {
    setupUICamera(ui_camera_);

    root_widget_.setAnchor(UI::Widget::Anchor::TOP_LEFT,
                           UI::Widget::Anchor::TOP_LEFT);
    root_widget_.setVerticalScaling(1.0f);
    root_widget_.setHorizontalScaling(0.5f);
    root_widget_.setParam("color", ci::Color(1, 0, 0));

    // コールバック関数
    auto callback = [this](Connection, UI::Widget& widget, const UI::Widget::TouchEvent touch_event, const Touch&)
      {
        DOUT << widget.getIdentifier() << ":";
        
        switch (touch_event)
        {
        case UI::Widget::TouchEvent::BEGAN:
          {
            DOUT << "TOUCH_BEGAN" << std::endl;

            auto w = widget.find("button1");
            auto& color = w->getParam<ci::Color>("color");
            color = test_ ? ci::Color(0.5, 0, 0)
                          : ci::Color(0, 0.5, 0);

            test_ = !test_;
          }
          break;

        case UI::Widget::TouchEvent::MOVED_IN:
          {
            DOUT << "TOUCH_MOVED_IN" << std::endl;
          }
          break;

        case UI::Widget::TouchEvent::MOVED_OUT:
          {
            DOUT << "TOUCH_MOVED_OUT" << std::endl;
          }
          break;

        case UI::Widget::TouchEvent::MOVED_EDGE_OUT:
          {
            DOUT << "TOUCH_MOVED_EDGE_OUT" << std::endl;
          }
          break;

        case UI::Widget::TouchEvent::MOVED_EDGE_IN:
          {
            DOUT << "TOUCH_MOVED_EDGE_IN" << std::endl;
          }
          break;
          
        case UI::Widget::TouchEvent::ENDED_IN:
          {
            DOUT << "TOUCH_ENDED_IN" << std::endl;
          }
          break;

        case UI::Widget::TouchEvent::ENDED_OUT:
          {
            DOUT << "TOUCH_ENDED_OUT" << std::endl;
          }
          break;

        default:
          break;
        }
      };

    {
      UI::WidgetPtr widget = std::make_shared<UI::Widget>("button1", ci::vec2(), ci::vec2(200, 100), UI::Drawer::fillRect);
      root_widget_.addChild(widget);
      widget->setParam("color", ci::Color(0, 1, 0));
      widget->setAnchor(UI::Widget::Anchor::BOTTOM_CENTER, UI::Widget::Anchor::MIDDLE_CENTER);
      widget->enableTouchEvent(true);
      widget->connect(callback);
    }

    {
      UI::WidgetPtr widget = std::make_shared<UI::Widget>("button2", ci::vec2(0, 20), ci::vec2(200, 100), UI::Drawer::fillRect);
      root_widget_.addChild(widget);
      widget->setParam("color", ci::Color(0, 0, 1));
      widget->setAnchor(UI::Widget::Anchor::BOTTOM_CENTER, UI::Widget::Anchor::BOTTOM_CENTER);
      widget->enableTouchEvent(true);
      widget->connect(callback);
    }
  }


  // touching_numはタッチ操作中の数(新たに発生したのも含む)
  // touchesは新たに発生したタッチイベント内容
  void touchesBegan(const int touching_num,
                    const std::vector<Touch>& touches) noexcept
  {
    bool touching = false;
#if defined (CINDER_COCOA_TOUCH)
    // iOS版は最初のを覚えとく
    if (touching_num == 1)
    {
      touch_id_ = touches[0].getId();
      touching  = true;
    }
#else
    // PC版はMouseDownを覚えておく
    if ((touches.size() == 1) && touches[0].isMouse())
    {
      touch_id_ = touches[0].getId();
      touching  = true;
    }
#endif

    // Widgetへタッチ判定を送る
    if (touching)
    {
      touching_ = true;
      ci::vec2 size = ci::app::getWindowSize();

      root_widget_.touchBegan(touches[0], -size / 2.0f, size);
    }
  }

  // touching_numはタッチ操作中の数
  void touchesMoved(const int touching_num,
                    const std::vector<Touch>& touches) noexcept
  {
    if (!touching_) return;
    
    for (const auto touch : touches)
    {
      if (touch_id_ == touch.getId())
      {
        ci::vec2 size = ci::app::getWindowSize();
        root_widget_.touchMoved(touch, -size / 2.0f, size);
        break;
      }
    }
  }

  // touching_numは残りのタッチ操作中の数
  void touchesEnded(const int touching_num,
                    const std::vector<Touch>& touches) noexcept
  {
    if (!touching_) return;

    for (const auto touch : touches)
    {
      if (touch_id_ == touch.getId())
      {
        ci::vec2 size = ci::app::getWindowSize();
        root_widget_.touchEnded(touch, -size / 2.0f, size);

        touching_ = false;
        break;
      }
    }
  }

  
  void resize() noexcept
  {
    setupUICamera(ui_camera_);
  }
  
  void update() noexcept
  {
    timeline_->stepTo(ci::app::getElapsedSeconds());
  }

  void draw() noexcept
  {
    ci::gl::clear(ci::Color(0, 0, 0));
    ci::gl::setMatrices(ui_camera_);

    ci::vec2 size = ci::app::getWindowSize();
    root_widget_.draw(-size / 2.0f, size);
  }

};

}
