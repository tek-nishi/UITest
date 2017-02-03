#pragma once

//
// UI::WidgetsをJSONから生成
//

#include "UIWidget.hpp"
#include "UITween.hpp"
#include "UIDrawer.hpp"


namespace ngs { namespace UI {

class WidgetsFactory
  : private boost::noncopyable
{
  ci::TimelineRef timeline_;
  
  // TIPS:文字列から描画関数を指定用
  const std::map<std::string, DrawFunc> draw_func_;

  // Tween設定
  Tween tween_;

#if 0
  void tweenCallback(const WidgetPtr& widget) const noexcept
  {
    widget->connect([this](Connection, UI::Widget& widget, const UI::Widget::TouchEvent touch_event, const Touch&)
                    {
                      switch (touch_event)
                      {
                      case UI::Widget::TouchEvent::BEGAN:
                        tween_.start(widget.getTimeline(), widget.getPosition(), widget.getScale(), widget.getColor());
                        break;
                        
                      case UI::Widget::TouchEvent::ENDED_IN:
                      case UI::Widget::TouchEvent::ENDED_OUT:
                        break;
                      }
                    });
  }
#endif

  
  // 各種値をJsonから読み取る
  static void loadParams(const WidgetPtr& widget, const ci::JsonTree& params)
  {
    for (const auto& p : params["params"])
    {
      // TIPS:switch文の文字列版
      static std::map<std::string, std::function<void(Widget& widget, const ci::JsonTree& params)>> functions = {
        {
          "color",
          [](Widget& widget, const ci::JsonTree& params)
          {
            ci::Color color(params.getValueAtIndex<float>(1),
                            params.getValueAtIndex<float>(2),
                            params.getValueAtIndex<float>(3));
            widget[params.getKey()] = color;
          }
        },
        {
          "string",
          [](Widget& widget, const ci::JsonTree& params)
          {
            widget[params.getKey()] = params.getValueAtIndex<std::string>(1);
          }
        },
        {
          "int",
          [](Widget& widget, const ci::JsonTree& params)
          {
            widget[params.getKey()] = params.getValueAtIndex<int>(1);
          }
        },
        {
          "float",
          [](Widget& widget, const ci::JsonTree& params)
          {
            widget[params.getKey()] = params.getValueAtIndex<float>(1);
          }
        },
        {
          "double",
          [](Widget& widget, const ci::JsonTree& params)
          {
            widget[params.getKey()] = params.getValueAtIndex<double>(1);
          }
        },
        {
          "vec2",
          [](Widget& widget, const ci::JsonTree& params)
          {
            ci::vec2 vec(params.getValueAtIndex<float>(1),
                         params.getValueAtIndex<float>(2));
            
            widget[params.getKey()] = vec;
          }
        },
        {
          "vec3",
          [](Widget& widget, const ci::JsonTree& params)
          {
            ci::vec3 vec(params.getValueAtIndex<float>(1),
                         params.getValueAtIndex<float>(2),
                         params.getValueAtIndex<float>(3));
            
            widget[params.getKey()] = vec;
          }
        },
      };
      
      // 配列の最初の値が型
      auto type = p.getValueAtIndex<std::string>(0);
      functions.at(type)(*widget.get(), p);
    }
  }

  // JSONから生成(階層構造も含む)
  WidgetPtr create(const ci::JsonTree& params,
                   const std::shared_ptr<std::map<std::string, Widget*>>& widgets) const noexcept
  {
    // UI::Widgetを生成するのに必要な値
    auto identifier = params.getValueForKey<std::string>("identifier");
    auto rect = Json::getRect(params["rect"]);
    DrawFunc draw_func = draw_func_.at(params.getValueForKey<std::string>("type"));

    auto widget = std::make_shared<UI::Widget>(identifier, rect, widgets, timeline_, draw_func);

    // アンカー
    if (params.hasChild("anchor"))
    {
      widget->setAnchor(Json::getVec<ci::vec2>(params["anchor"][0]),
                        Json::getVec<ci::vec2>(params["anchor"][1]));
    }
    
    // スケーリング
    if (params.hasChild("scale"))
    {
      widget->setScale(Json::getVec<ci::vec2>(params["scale"]));
    }

    if (params.hasChild("pivot"))
    {
      widget->setPivot(Json::getVec<ci::vec2>(params["pivot"]));
    }

    if (params.hasChild("color"))
    {
      widget->setColor(Json::getColorA<float>(params["color"]));
    }

    // 各種動作指定
    widget->enableActive(params.getValueForKey<bool>("active"));
    widget->enableDisplay(params.getValueForKey<bool>("display"));
    widget->enableTouchEvent(params.getValueForKey<bool>("touch_event"));
    
    // パラメーター読み込み
    loadParams(widget, params);

    // Tween
    // tweenCallback(widget);

    
    // 子供を追加
    // TIPS:再帰で実装
    if (params.hasChild("childlen"))
    {
      for (const auto& child : params["childlen"])
      {
        widget->addChild(create(child, widgets));
      }
    }
    
    return widget;
  }

  
public:
  WidgetsFactory(const ci::TimelineRef& timeline, const std::map<std::string, DrawFunc>& draw_func) noexcept
    : timeline_(timeline),
      draw_func_(draw_func),
      tween_(Params::load("tween.json"))
  {
  }

  ~WidgetsFactory() = default;


  // JSONからWidgetを生成する
  WidgetPtr construct(const ci::JsonTree& params) noexcept
  {
    // クエリ用
    std::shared_ptr<std::map<std::string, Widget*>> widgets = std::make_shared<std::map<std::string, Widget*>>();
    return create(params, widgets);
  }
  
};

} }
