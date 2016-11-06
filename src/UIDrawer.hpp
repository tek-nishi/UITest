#pragma once

//
// UI描画
//

#include "UIWidget.hpp"


namespace ngs { namespace UI { namespace Drawer {

// 何も描画しない
void blank(const UI::Widget& widget, const ci::vec2& pos, const ci::vec2& size)
{
}

// 一色塗り潰し
void fillRect(const UI::Widget& widget, const ci::vec2& pos, const ci::vec2& size)
{
  // FIXME:仮描画
  ci::Rectf rect(pos, pos + size);
  ci::gl::color(widget.getParam<ci::Color>("color"));
  ci::gl::drawSolidRect(rect);
}

// 画像描画
void image(const UI::Widget& widget, const ci::vec2& pos, const ci::vec2& size)
{
}

// 文字列表示
void text(const UI::Widget& widget, const ci::vec2& pos, const ci::vec2& size)
{
}

} } }
