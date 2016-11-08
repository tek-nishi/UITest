#pragma once

//
// UI::WidgetsをJSONから生成
//

#include "UIWidget.hpp"
#include "UIDrawer.hpp"


namespace ngs { namespace UI {

class WidgetsFactory
  : private boost::noncopyable
{
  // TIPS:アンカーを文字列からenumに変換する用
  const std::map<std::string, Widget::Anchor> anchor = {
    { "TOP_LEFT",   Widget::Anchor::TOP_LEFT },
    { "TOP_CENTER", Widget::Anchor::TOP_CENTER },
    { "TOP_RIGHT",  Widget::Anchor::TOP_RIGHT },

    { "MIDDLE_LEFT",   Widget::Anchor::MIDDLE_LEFT },
    { "MIDDLE_CENTER", Widget::Anchor::MIDDLE_CENTER },
    { "MIDDLE_RIGHT",  Widget::Anchor::MIDDLE_RIGHT },

    { "BOTTOM_LEFT",   Widget::Anchor::BOTTOM_LEFT },
    { "BOTTOM_CENTER", Widget::Anchor::BOTTOM_CENTER },
    { "BOTTOM_RIGHT",  Widget::Anchor::BOTTOM_RIGHT },
  };

  // TIPS:文字列から描画関数を指定用
  const std::map<std::string, DrawFunc> draw_func_;


  // 各種値をJsonから読み取る
  static void loadParams(const WidgetPtr& widget, const ci::JsonTree& params)
  {
    for (const auto& p : params["params"])
    {
      std::map<std::string, std::function<void(Widget& widget, const ci::JsonTree& params)>> functions = {
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
      const auto& type = p.getValueAtIndex<std::string>(0);

      functions.at(type)(*widget.get(), p);
    }
  }




  
public:
  WidgetsFactory(const std::map<std::string, DrawFunc>& draw_func) noexcept
    : draw_func_(draw_func)
  {
  }

  ~WidgetsFactory() = default;

  
  // JSONから生成(階層構造も含む)
  WidgetPtr create(const ci::JsonTree& params,
                   const ci::vec2& initial_position,
                   const ci::vec2& initial_size) noexcept
  {
    // UI::Widgetを生成するのに必要な値
    const auto& identifier = params.getValueForKey<std::string>("identifier");
    const auto position    = Json::getVec(params, "position", initial_position);
    const auto size        = Json::getVec(params, "size",     initial_size);

    DrawFunc draw_func = draw_func_.at(params.getValueForKey<std::string>("type"));

    auto widget = std::make_shared<UI::Widget>(identifier, position, size, draw_func);

    // アンカー指定
    {
      const auto& anchor_pivot    = params.getValueForKey<std::string>("anchor_pivot");
      const auto& anchor_position = params.getValueForKey<std::string>("anchor_position");

      widget->setAnchor(anchor.at(anchor_pivot), anchor.at(anchor_position));
    }

    // スケーリング指定
    if (params.hasChild("vertical_scaling"))
    {
      float value = params.getValueForKey<float>("vertical_scaling");
      widget->setVerticalScaling(value);
    }

    if (params.hasChild("horizontal_scaling"))
    {
      float value = params.getValueForKey<float>("horizontal_scaling");
      widget->setHorizontalScaling(value);
    }

    // 各種動作指定
    widget->enableActive(params.getValueForKey<bool>("active"));
    widget->enableDisplay(params.getValueForKey<bool>("display"));
    widget->enableTouchEvent(params.getValueForKey<bool>("touch_event"));

    // パラメーター読み込み
    loadParams(widget, params);
    
    // 子供を追加
    // TIPS:再帰で実装
    if (params.hasChild("childlen"))
    {
      for (const auto& child : params["childlen"])
      {
        widget->addChild(create(child, initial_position, initial_size));
      }
    }
    
    return widget;
  }
  
};

} }
