#pragma once

//
// UI描画
// FIXME:全部入り・・・
//

#include <boost/noncopyable.hpp>
#include "UIWidget.hpp"
#include "Font.hpp"


namespace ngs { namespace UI {

class Drawer
  : private boost::noncopyable
{
  // 文字列描画用
  Font font_ = { 1024, 1024, FONS_ZERO_BOTTOMLEFT };
  ci::gl::GlslProgRef font_shader_ = createShader("font", "font");
  

  // 何も描画しない
  void blank(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
  }

  // 枠だけ描画
  void rect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
    // FIXME:仮描画
    ci::gl::color(widget.getColor());
    float line_width = widget.at<float>("line_width");
    ci::gl::drawStrokedRect(rect, line_width);
  }

  // 一色塗り潰し
  void fillRect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
    // FIXME:仮描画
    ci::gl::color(widget.getColor());
    ci::gl::drawSolidRect(rect);
  }

  // 角丸矩形
  void roundedRect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
    // FIXME:仮描画
    ci::gl::color(widget.getColor());

    // FIXME:線の幅を指定できない
    // float line_width    = widget.at<float>("line_width");
    // ci::gl::lineWidth(line_width);
  
    float corner_radius = widget.at<float>("corner_radius");
    ci::gl::drawStrokedRoundedRect(rect, corner_radius);
  }

  // 一色塗り潰し(角丸)
  void roundedFillRect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
    // FIXME:仮描画
    ci::gl::color(widget.getColor());
    float corner_radius = widget.at<float>("corner_radius");
    ci::gl::drawSolidRoundedRect(rect, corner_radius);
  }


  // 画像描画
  void image(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
    // FIXME:仮描画
    ci::gl::color(widget.getColor());

    ci::gl::ScopedTextureBind texture(widget.at<ci::gl::Texture2dRef>("image"));
    auto shader = ci::gl::ShaderDef().texture().color();
    ci::gl::ScopedGlslProg glsl(ci::gl::getStockShader(shader));
  
    ci::gl::drawSolidRect(rect, ci::vec2(0, 0), ci::vec2(1, 1));
  }


  static ci::vec2 calcTextPos(const ci::Rectf& rect, const ci::Rectf& bounds,
                              const std::string& align_v, const std::string& align_h) noexcept
  {
    static std::map<std::string, std::function<float (const ci::Rectf& rect, const ci::Rectf& bounds)>> calc_v {
      { "top",
          [](const ci::Rectf& rect, const ci::Rectf& bounds) noexcept {
          return rect.y2 - bounds.y2;
        }
      },
      { "center",
          [](const ci::Rectf& rect, const ci::Rectf& bounds) noexcept {
          return ((rect.y2 - rect.y1) - bounds.y2) / 2.0f + rect.y1;
        }
      },
      { "bottom",
          [](const ci::Rectf& rect, const ci::Rectf& bounds) noexcept {
          return rect.y1;
        }
      },
    };
    
    static std::map<std::string, std::function<float (const ci::Rectf& rect, const ci::Rectf& bounds)>> calc_h {
      { "left",
          [](const ci::Rectf& rect, const ci::Rectf& bounds) noexcept {
          return rect.x1;
        }
      },
      { "center",
          [](const ci::Rectf& rect, const ci::Rectf& bounds) noexcept {
          return ((rect.x2 - rect.x1) - bounds.x2) / 2.0f + rect.x1;
        }
      },
      { "right",
          [](const ci::Rectf& rect, const ci::Rectf& bounds) noexcept {
          return rect.x2 - bounds.x2;
        }
      },
    };

    float x = calc_h.at(align_h)(rect, bounds);
    float y = calc_v.at(align_v)(rect, bounds);

    return ci::vec2(x, y);
  }
  
  // 文字列表示
  void text(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale) noexcept
  {
    // FIXME:仮描画
    ci::gl::ScopedGlslProg glsl(font_shader_);

    fonsSetSize(font_(), widget.at<float>("size"));
    const ci::ColorA& color(widget.getColor());
    fonsSetColor(font_(), font_.color(color.r, color.g, color.b, color.a));
    
    int f = fonsGetFontByName(font_(), widget.at<std::string>("font").c_str());
    assert(f != FONS_INVALID);
    fonsSetFont(font_(), f);

    const auto& text = widget.at<std::string>("text");
    float bounds[4];
    fonsTextBounds(font_(), 0, 0, text.c_str(), nullptr, bounds);

    const auto& align_v = widget.at<std::string>("align_v");
    const auto& align_h = widget.at<std::string>("align_h");
    
    auto pos = calcTextPos(rect, ci::Rectf(bounds[0], bounds[1], bounds[2], bounds[3]),
                           align_v, align_h);

    fonsDrawText(font_(), pos.x, pos.y, text.c_str(), nullptr);
  }
  

public:
  Drawer() noexcept
  {
    fonsClearState(font_());
    fonsSetAlign(font_(), FONS_ALIGN_LEFT | FONS_ALIGN_BOTTOM);
  }


  DrawFunc getFunc(const std::string& identifier) noexcept
  {
    // TIPS:文字列から描画関数を指定用
    std::map<std::string, DrawFunc> draw_func {
      { "blank",             std::bind(&Drawer::blank,           this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
      { "rect",              std::bind(&Drawer::rect,            this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
      { "fill_rect",         std::bind(&Drawer::fillRect,        this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
      { "rounded_rect",      std::bind(&Drawer::roundedRect,     this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
      { "rounded_fill_rect", std::bind(&Drawer::roundedFillRect, this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
      { "image",             std::bind(&Drawer::image,           this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
      { "text",              std::bind(&Drawer::text,            this, std::placeholders::_1,  std::placeholders::_2, std::placeholders::_3) },
    };

    assert(draw_func.count(identifier));
    return draw_func.at(identifier);
  }


  void addFont(const std::string& path) noexcept
  {
    font_.add(path, path);
  }

};

} }
