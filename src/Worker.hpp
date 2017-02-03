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
#include "UICanvas.hpp"
#include "UIDrawer.hpp"
#include "UIWidgetsFactory.hpp"


namespace ngs {

class Worker {
  // 汎用的なコールバック管理
  Event<Arguments> event_;
  ConnectionHolder holder_;

  // ゲーム内パラメーター
  ci::JsonTree params_;

  // UIなどきっかけが必要な演出用
  ci::TimelineRef timeline_;

  // UI
  UI::Canvas canvas_;

  // UI描画用関数群
  const std::map<std::string, UI::DrawFunc> draw_func_ = {
    { "blank",              UI::Drawer::blank },
    { "rect",               UI::Drawer::rect },
    { "fill_rect",          UI::Drawer::fillRect },
    { "rounded_rect",       UI::Drawer::roundedRect },
    { "rounded_fill_rect",  UI::Drawer::roundedFillRect },
    { "image",              UI::Drawer::image },
    { "text",               UI::Drawer::text },
  };

  // UI生成用
  UI::WidgetsFactory widgets_factory_;

  bool touching_ = false;
  uint32_t touch_id_;


public:
  Worker() noexcept
  : params_(Params::load("params.json")),
    timeline_(ci::Timeline::create()),
    widgets_factory_(timeline_, draw_func_)
  {
    // JSONファイルからWidgetを生成
    canvas_.setWidgets(widgets_factory_.construct(Params::load("widgets.json")));

    
#if 0
    // // コールバック関数
    auto callback = [this](Connection, UI::Widget& widget, const UI::Widget::TouchEvent touch_event, const Touch&)
      {
        DOUT << widget.getIdentifier() << ":";

        switch (touch_event)
        {
        case UI::Widget::TouchEvent::BEGAN:
          {
            DOUT << "TOUCH_BEGAN" << std::endl;
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

    canvas_->find("button1")->connect(callback);
    canvas_->find("button2")->connect(callback);
#endif
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

      // root_widget_->touchBegan(touches[0], -size / 2.0f, size);
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
        // root_widget_->touchMoved(touch, -size / 2.0f, size);
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
        // root_widget_->touchEnded(touch, -size / 2.0f, size);

        touching_ = false;
        break;
      }
    }
  }


  void resize() noexcept
  {
    canvas_.resize(ci::app::getWindowSize());
  }

  void update() noexcept
  {
    timeline_->stepTo(ci::app::getElapsedSeconds());
  }

  void draw() noexcept
  {
    ci::gl::clear(ci::Color(0, 0, 0));
    canvas_.draw();
  }

};

}
