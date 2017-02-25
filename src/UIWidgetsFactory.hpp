#pragma once

//
// UI::WidgetsをJSONから生成
//

#include <cinder/ImageIo.h>
#include "Font.hpp"
#include "UIWidget.hpp"
#include "UIDrawer.hpp"
#include "Misc.hpp"


namespace ngs { namespace UI {

class WidgetsFactory
  : private boost::noncopyable
{
  Drawer& drwer_;
  
  
  // 各種値をJsonから読み取る
  void loadParams(const WidgetPtr& widget, const ci::JsonTree& params) noexcept
  {
    for (const auto& p : params["params"])
    {
      // TIPS:switch文の文字列版
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
        {
          "image",
          [](Widget& widget, const ci::JsonTree& params)
          {
            const auto& path = params.getValueAtIndex<std::string>(1);
            auto image = ci::gl::Texture2d::create(ci::loadImage(Asset::load(path)));
            widget[params.getKey()] = image;
            widget["path"] = path;
          }
        },
        {
          "font",
          [this](Widget& widget, const ci::JsonTree& params)
          {
            const auto& path = params.getValueAtIndex<std::string>(1);
            drwer_.addFont(path);
            widget[params.getKey()] = path;
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
                   const WidgetQueryPtr& widgets) noexcept
  {
    // UI::Widgetを生成するのに必要な値
    auto identifier = params.getValueForKey<std::string>("identifier");
    auto rect = Json::getRect(params["rect"]);
    const auto& type_id = params.getValueForKey<std::string>("type");
    DrawFunc draw_func = drwer_.getFunc(type_id);

    auto widget = std::make_shared<UI::Widget>(identifier, rect, widgets, draw_func);
    widget->setType(type_id);

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
  WidgetsFactory(Drawer& drwer) noexcept
    : drwer_(drwer)
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
