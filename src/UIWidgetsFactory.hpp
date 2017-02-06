#pragma once

//
// UI::WidgetsをJSONから生成
//

#include <cinder/ImageIo.h>
#include "UIWidget.hpp"
#include "UIDrawer.hpp"


namespace ngs { namespace UI {

class WidgetsFactory
  : private boost::noncopyable
{
  // TIPS:文字列から描画関数を指定用
  const std::map<std::string, DrawFunc> draw_func_;

  
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
        {
          "image",
          [](Widget& widget, const ci::JsonTree& params)
          {
            auto image = ci::gl::Texture2d::create(ci::loadImage(Asset::load(params.getValueAtIndex<std::string>(1))));
            widget[params.getKey()] = image;
          }
        }
      };
      
      // 配列の最初の値が型
      auto type = p.getValueAtIndex<std::string>(0);
      functions.at(type)(*widget.get(), p);
    }
  }

  // JSONから生成(階層構造も含む)
  WidgetPtr create(const ci::JsonTree& params,
                   const WidgetQueryPtr& widgets) const noexcept
  {
    // UI::Widgetを生成するのに必要な値
    auto identifier = params.getValueForKey<std::string>("identifier");
    auto rect = Json::getRect(params["rect"]);
    DrawFunc draw_func = draw_func_.at(params.getValueForKey<std::string>("type"));

    auto widget = std::make_shared<UI::Widget>(identifier, rect, widgets, draw_func);

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
  WidgetsFactory(const std::map<std::string, DrawFunc>& draw_func) noexcept
    : draw_func_(draw_func)
  {
  }

  ~WidgetsFactory() = default;


  // JSONからWidgetを生成する
  WidgetPtr construct(const ci::JsonTree& params) noexcept
  {
    // クエリ用
    auto widgets = std::make_shared<WidgetQuery>();
    return create(params, widgets);
  }
  
};

} }
