#pragma once

//
// UI描画
//

#include "UIWidget.hpp"


namespace ngs { namespace UI { namespace Drawer {

// 何も描画しない
void blank(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
{
}


// 枠だけ描画
void rect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
{
  // FIXME:仮描画
  ci::gl::color(widget.getColor());
  float line_width = widget.at<float>("line_width");
  ci::gl::drawStrokedRect(rect, line_width);
}

// 一色塗り潰し
void fillRect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
{
  // FIXME:仮描画
  ci::gl::color(widget.getColor());
  ci::gl::drawSolidRect(rect);
}

// 角丸矩形
void roundedRect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
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
void roundedFillRect(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
{
  // FIXME:仮描画
  ci::gl::color(widget.getColor());
  float corner_radius = widget.at<float>("corner_radius");
  ci::gl::drawSolidRoundedRect(rect, corner_radius);
}


// 画像描画
void image(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
{
}

// 文字列表示
void text(const UI::Widget& widget, const ci::Rectf& rect, const ci::vec2& scale)
{
}

} } }
