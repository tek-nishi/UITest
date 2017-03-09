#pragma once

//
// UI::Widget編集
//

#include <cinder/params/Params.h>
#include "UICanvas.hpp"
#include "UIDrawer.hpp"


namespace ngs { namespace UI {

class Editor
{
  ci::params::InterfaceGlRef list_;
  int id_index_ = 0;
  std::vector<std::string> id_list_;

  ci::params::InterfaceGlRef setting_ = ci::params::InterfaceGl::create("Setting", ci::ivec2(320, 640));

  Canvas& canvas_;
  Drawer& drawer_;
  

  // Widgetを列挙
  static void createWidgetList(std::vector<std::string>& list, std::vector<std::string>& id_list,
                               const WidgetPtr& parent_widget, int depth) noexcept
  {
    std::string id(depth, '-');
    const auto& id_widget = parent_widget->getIdentifier();
    id_list.push_back(id_widget);
    list.push_back(id + id_widget);

    for (const auto& widget : parent_widget->getChilds())
    {
      // 再起で子Widgetを列挙
      // TIPS:識別子の頭に深さを表すハイフンを付与している
      createWidgetList(list, id_list, widget, depth + 1);
    }
  }

  ci::params::InterfaceGlRef createWidgetList(const WidgetPtr& root_widget) noexcept
  {
    std::vector<std::string> id_list;
    createWidgetList(id_list, id_list_, root_widget, 0);

    auto list = ci::params::InterfaceGl::create("List", ci::ivec2(200, 200));

    list->addParam("Widget", id_list, &id_index_).updateFn([this]() {
        auto* widget = canvas_.findWidget(id_list_[id_index_]);
        createWidgetSetting(setting_, widget);
      });

    list->addSeparator();

    list->addButton("Save", [this]() {
        DOUT << "Saved Scene." << std::endl;
      });
    
    return list;
  }


  // 画像Widget編集
  void widgetImage(const ci::params::InterfaceGlRef& setting, Widget* widget) noexcept
  {
    setting->addParam("path", &widget->at<std::string>("path")).updateFn([widget]() {
        // TODO:エラー対策
        auto image = ci::gl::Texture2d::create(ci::loadImage(Asset::load(widget->at<std::string>("path"))));
              (*widget)["image"] = image;
      });
  }

  // Text Widget編集
  void widgetText(const ci::params::InterfaceGlRef& setting, Widget* widget) noexcept
  {
    setting->addParam("font", &widget->at<std::string>("font")).updateFn([this, widget]() {
        // TODO:エラー対策
        const auto& path = widget->at<std::string>("font");
        drawer_.addFont(path);
      });
    
    setting->addParam("size", &widget->at<float>("size"));
    setting->addParam("text", &widget->at<std::string>("text"));

    {
      static const std::vector<std::string> align_v_list = { "top", "center", "bottom" };
    
      setting->addParam("align_v", align_v_list,
                        [widget](int index) {
                          // Setter
                          widget->at<std::string>("align_v") = align_v_list[index];
                        },
                        [widget]() {
                          // Getter
                          const auto& align_v = widget->at<std::string>("align_v");

                          // 配列から値を探してそのindexを返却している
                          auto it = std::find(std::begin(align_v_list), std::end(align_v_list), align_v);
                          size_t index = std::distance(std::begin(align_v_list), it);
                          if (index == align_v_list.size()) index = 1;
                          return index;
                        });
    }

    {
      static const std::vector<std::string> align_h_list = { "left", "center", "right" };

      setting->addParam("align_h", align_h_list,
                        [widget](int index) {
                          // Setter
                          widget->at<std::string>("align_h") = align_h_list[index];
                        },
                        [widget]() {
                          // Getter
                          const auto& align_h = widget->at<std::string>("align_h");
                          
                          auto it = std::find(std::begin(align_h_list), std::end(align_h_list), align_h);
                          size_t index = std::distance(std::begin(align_h_list), it);
                          if (index == align_h_list.size()) index = 1;
                          return index;
                        });
    }
  }

  
  // Widget編集
  //   Widgetの種別に応じた設定
  void createWidgetSeparateSetting(const ci::params::InterfaceGlRef& setting, Widget* widget) noexcept
  {
    setting->addSeparator();
    
    static std::map<std::string, std::function<void(const ci::params::InterfaceGlRef& setting, Widget* widget)>> func_tbl = {
      { "blank",
        [](const ci::params::InterfaceGlRef& setting, Widget* widget) {} },

      { "rect",
        [](const ci::params::InterfaceGlRef& setting, Widget* widget) {
          setting->addParam("line width", &widget->at<float>("line_width"));
        } },
      { "fill_rect",
        [](const ci::params::InterfaceGlRef& setting, Widget* widget) {} },
      { "rounded_rect",
        [](const ci::params::InterfaceGlRef& setting, Widget* widget) {} },
      { "rounded_fill_rect",
        [](const ci::params::InterfaceGlRef& setting, Widget* widget) {} },

      { "image",
        std::bind(&Editor::widgetImage, this, std::placeholders::_1, std::placeholders::_2) },
      { "text",
        std::bind(&Editor::widgetText,  this, std::placeholders::_1, std::placeholders::_2) },
    };
    
    const auto& type_id = widget->getType();
    func_tbl.at(type_id)(setting, widget);
  }
  
  void createWidgetSetting(const ci::params::InterfaceGlRef& setting, Widget* widget) noexcept
  {
    setting->clear();

    // 全UI共通のパラメーター
    setting->addParam("Identifier", &widget->getIdentifier());
    setting->addParam("Type", &widget->getType());

    setting->addSeparator();
    
    auto& pivot = widget->getPivot();
    setting->addParam("pivot X", &pivot.x);
    setting->addParam("pivot Y", &pivot.y);

    auto& rect = widget->getRect();
    setting->addParam("rect X1", &rect.x1);
    setting->addParam("rect X2", &rect.x2);
    setting->addParam("rect Y1", &rect.y1);
    setting->addParam("rect Y2", &rect.y2);

    auto& anchor_min = widget->getAnchorMin();
    setting->addParam("anchor min X", &anchor_min.x);
    setting->addParam("anchor min Y", &anchor_min.y);

    auto& anchor_max = widget->getAnchorMax();
    setting->addParam("anchor max X", &anchor_max.x);
    setting->addParam("anchor max Y", &anchor_max.y);

    auto& scale = widget->getScale();
    setting->addParam("scale X", &scale.x);
    setting->addParam("scale Y", &scale.y);

    setting->addParam("color", &widget->getColor());

    setting->addParam("active", &widget->getActive());
    setting->addParam("display", &widget->getDisplay());
    setting->addParam("touch_event", &widget->getTouchEvent());

    // 個別設定
    createWidgetSeparateSetting(setting, widget);
  }


public:
  Editor(Canvas& canvas, Drawer& drawer) noexcept
    : canvas_(canvas),
      drawer_(drawer)
  {
    list_ = createWidgetList(canvas.rootWidgetPtr());
    createWidgetSetting(setting_, canvas.rootWidget());
  }

  void draw() noexcept
  {
    list_->draw();
    setting_->draw();
  }

};

} }
