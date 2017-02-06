#pragma once

//
// UI部品
//  TODO:WidgetのRectの計算量を減らす
//

#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include "Touch.hpp"
#include "Event.hpp"
#include "Misc.hpp"


namespace ngs { namespace UI {

// TIPS:自分自身を引数に取る関数があるので先行宣言が必要
class Widget;
using WidgetPtr = std::shared_ptr<Widget>;

// クエリ用コンテナ
using WidgetQuery    = std::map<std::string, Widget*>;
using WidgetQueryPtr = std::shared_ptr<WidgetQuery>;

// 描画用関数
using DrawFunc = std::function<void (const Widget&, const ci::Rectf& rect, const ci::vec2& scale)>;


class Widget
  : private boost::noncopyable
{
public:
  enum TouchEvent {
    BEGAN,               // 領域内でタッチ

    MOVED_IN,            // タッチ位置移動(領域内)
    MOVED_OUT,           // タッチ位置移動(領域外)
    MOVED_EDGE_OUT,      // タッチ位置移動→領域外
    MOVED_EDGE_IN,       // 領域外→領域内

    ENDED_IN,            // 領域内でタッチ終了
    ENDED_OUT,           // 領域外でタッチ終了

    CANCELED,            // タッチイベント中断
  };


private:
  std::string identifier_;

  ci::Rectf rect_;

  // スケーリングの中心(normalized)
  ci::vec2 pivot_ = { 0.5f, 0.5f };

  // 親のサイズの影響力(normalized)
  ci::vec2 anchor_min_ = { 0.5f, 0.5f };
  ci::vec2 anchor_max_ = { 0.5f, 0.5f };

  ci::vec2 scale_ = { 1.0f, 1.0f };

  ci::ColorA color_ = { 1.0f, 1.0f, 1.0f, 1.0f };


  bool active_      = true;       // 有効・無効
  bool display_     = true;       // 表示・非表示
  bool touch_event_ = false;      // タッチイベント有効・無効

  // TIPS:振る舞いの違いを継承を使わないで実現する作戦
  std::map<std::string, boost::any> params_;

  std::vector<WidgetPtr> childs_;
  // クエリ用
  WidgetQueryPtr widgets_;

  // タッチイベントのコールバック
  using EventType = boost::signals2::signal<void (Widget&, const TouchEvent, const Touch&)>;
  EventType events_;

  // タッチイベント発生中
  bool touching_ = false;

  // 描画関数
  DrawFunc drawer_;


  // タッチイベントを発生するか判定
  bool execTouchEvent() noexcept
  {
    return active_ && display_ && touch_event_;
  }


public:
  Widget(std::string identifier, const ci::Rectf& rect,
         const WidgetQueryPtr& widgets, DrawFunc drawer) noexcept
    : identifier_(std::move(identifier)),
      rect_(rect),
      widgets_(widgets),
      drawer_(drawer)
  {
    // クエリ用のコンテナに自分を登録
    widgets_->insert({ identifier_, this });
  }

  ~Widget() = default;


  // FIXME:上流でシングルタッチ判定を行う
  // TODO:酷いコピペを減らす
  void touchBegan(const Touch& touch, const ci::Rectf& parent_rect, const ci::vec2& parent_scale)
  {
    // TIPS:子供も含めて判定しない
    if (!display_) return;

    ci::vec2 scale = parent_scale * scale_;
    auto rect = calcRect(parent_rect, scale);

    if (execTouchEvent())
    {
      if (rect.contains(touch.getPos()))
      {
        // タッチイベント発生
        touching_ = true;
        events_(*this, TouchEvent::BEGAN, touch);
      }
    }

    for (auto& widget : childs_)
    {
      widget->touchBegan(touch, rect, scale);
    }
  }

  void touchMoved(const Touch& touch, const ci::Rectf& parent_rect, const ci::vec2& parent_scale)
  {
    // TIPS:子供も含めて判定しない
    if (!display_) return;

    ci::vec2 scale = parent_scale * scale_;
    auto rect = calcRect(parent_rect, scale);

    if (execTouchEvent() && touching_)
    {
      bool prev_in = rect.contains(touch.getPrevPos());
      bool cur_in  = rect.contains(touch.getPos());

      TouchEvent event = TouchEvent::MOVED_IN;

      if (!cur_in && prev_in)
      {
        // 移動しながら領域外へ
        event = TouchEvent::MOVED_EDGE_OUT;
      }
      else if (cur_in && !prev_in)
      {
        // 移動しながら領域内へ
        event = TouchEvent::MOVED_EDGE_IN;
      }
      else if (!cur_in && !prev_in)
      {
        // 領域外で移動
        event = TouchEvent::MOVED_OUT;
      }

      events_(*this, event, touch);
    }

    for (auto& widget : childs_)
    {
      widget->touchMoved(touch, rect, scale);
    }
  }

  void touchEnded(const Touch& touch, const ci::Rectf& parent_rect, const ci::vec2& parent_scale)
  {
    // TIPS:子供も含めて判定しない
    if (!display_) return;

    ci::vec2 scale = parent_scale * scale_;
    auto rect = calcRect(parent_rect, scale);

    if (execTouchEvent() && touching_)
    {
      touching_ = false;

      TouchEvent event = rect.contains(touch.getPos()) ? TouchEvent::ENDED_IN
                                                       : TouchEvent::ENDED_OUT;
      events_(*this, event, touch);
    }

    for (auto& widget : childs_)
    {
      widget->touchEnded(touch, rect, scale);
    }
  }


  void draw(const ci::Rectf& parent_rect, const ci::vec2& parent_scale) noexcept
  {
    // TIPS:子供も含めて非表示
    if (!display_) return;

    ci::vec2 scale = parent_scale * scale_;
    auto rect = calcRect(parent_rect, scale);

    // ci::app::console() << identifier_ << std::endl
    //                    << rect << std::endl
    //                    << scale << std::endl;
    
    drawer_(*this, rect, scale);

    for (const auto& widget : childs_)
    {
      widget->draw(rect, scale);
    }
  }


  // 識別子
  const std::string& getIdentifier() const noexcept
  {
    return identifier_;
  }

  // 各種設定
  void setPivot(const ci::vec2& pivot) noexcept
  {
    pivot_ = pivot;
  }

  void setAnchor(const ci::vec2& anchor_min, const ci::vec2& anchor_max) noexcept
  {
    anchor_min_ = anchor_min;
    anchor_max_ = anchor_max;
  }

  void setScale(const ci::vec2& scale) noexcept
  {
    scale_ = scale;
  }
  
  // 有効・無効
  void enableActive(const bool enable) noexcept
  {
    active_ = enable;
  }

  bool isActive() const noexcept
  {
    return active_;
  }

  // 表示・非表示
  void enableDisplay(const bool enable) noexcept
  {
    display_ = enable;
  }

  bool isDisplay() const noexcept
  {
    return display_;
  }

  // タッチイベントの有効・無効
  void enableTouchEvent(const bool enable) noexcept
  {
    touch_event_ = enable;
  }

  bool isTouchEvent() const noexcept
  {
    return touch_event_;
  }

  // 基本色
  ci::ColorA& getColor() noexcept
  {
    return color_;
  }

  const ci::ColorA& getColor() const noexcept
  {
    return color_;
  }

  void setColor(const ci::ColorA& color) noexcept
  {
    color_ = color;
  }


  void addChild(const WidgetPtr& widget) noexcept
  {
    childs_.push_back(widget);
  }

  Widget* find(const std::string& identifier) noexcept
  {
    return widgets_->at(identifier);
  }

  // 文字列の示す値を返す
  float* getParam(const std::string& target) noexcept
  {
    std::map<std::string, std::function<float*()>> table = {
      { "rect_x1", [this]() { return &rect_.x1; } },
      { "rect_x2", [this]() { return &rect_.x2; } },
      { "rect_y1", [this]() { return &rect_.y1; } },
      { "rect_y2", [this]() { return &rect_.y2; } },
      
      { "pivot_x", [this]() { return &pivot_.x; } },
      { "pivot_y", [this]() { return &pivot_.y; } },

      { "anchor_min_x", [this]() { return &anchor_min_.x; } },
      { "anchor_min_y", [this]() { return &anchor_min_.y; } },
      { "anchor_max_x", [this]() { return &anchor_max_.x; } },
      { "anchor_max_y", [this]() { return &anchor_max_.y; } },
      
      { "scale_x", [this]() { return &scale_.x; } },
      { "scale_y", [this]() { return &scale_.y; } },

      { "color_r", [this]() { return &color_.r; } },
      { "color_g", [this]() { return &color_.g; } },
      { "color_b", [this]() { return &color_.b; } },
      { "color_a", [this]() { return &color_.a; } },
    };

    float* param = table.at(target)();
    return param;
  }
  

  // パラメーターの読み書きを簡易に書くためのラッパー
  const boost::any& operator[](const std::string& key) const
  {
    return params_.at(key);
  }

  boost::any& operator[](const std::string& key)
  {
    return params_[key];
  }

  template<typename T>
  const T& at(const std::string& key) const
  {
    return boost::any_cast<const T&>(params_.at(key));
  }


  template<typename F>
  Connection connect(F callback) noexcept
  {
    return events_.connect_extended(callback);
  }


private:
  // 親の情報から自分の位置、サイズを計算
  ci::Rectf calcRect(const ci::Rectf& parent_rect, const ci::vec2& scale) const noexcept
  {
    ci::vec2 parent_size = parent_rect.getSize();

    // 親のサイズとアンカーから左下・右上の座標を計算
    ci::vec2 anchor_min = parent_size * anchor_min_;
    ci::vec2 anchor_max = parent_size * anchor_max_;

    // 相対座標(スケーリング抜き)
    ci::vec2 pos  = rect_.getUpperLeft() + anchor_min;
    ci::vec2 size = rect_.getLowerRight() + anchor_max - pos;

    // pivotを考慮したスケーリング
    ci::vec2 d = size * pivot_;
    pos -= d * scale - d;
    size *= scale;

    ci::vec2 parent_pos = parent_rect.getUpperLeft();
    return ci::Rectf(pos + parent_pos, pos + size + parent_pos);
  }
};

} }
