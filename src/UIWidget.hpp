#pragma once

//
// UI部品
//

#include <boost/noncopyable.hpp>
#include <boost/any.hpp>
#include <boost/optional.hpp>
#include "Touch.hpp"
#include "Event.hpp"
#include "Misc.hpp"


namespace ngs { namespace UI {

// TIPS:自分自身を引数に取る関数があるので先行宣言が必要
struct Widget;
using WidgetPtr = std::shared_ptr<Widget>;

// 描画用関数
using DrawFunc = std::function<void (const Widget&, const ci::vec2& pos, const ci::vec2& size)>;

class Widget
  : private boost::noncopyable
{
public:
  // 画面レイアウト用の指示
  enum Anchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,

    MIDDLE_LEFT,
    MIDDLE_CENTER,
    MIDDLE_RIGHT,

    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
  };

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
  
  ci::vec2 position_;
  ci::vec2 size_;

  Anchor anchor_pivot_    = Anchor::MIDDLE_CENTER;
  Anchor anchor_position_ = Anchor::MIDDLE_CENTER;

  bool active_      = true;       // 有効・無効
  bool display_     = true;       // 表示・非表示
  bool touch_event_ = false;      // タッチイベント有効・無効
  
  boost::optional<float> vertical_scaling_;
  boost::optional<float> horizontal_scaling_;

  // TIPS:振る舞いの違いを継承を使わないで実現する作戦
  std::map<std::string, boost::any> params_;
  
  std::vector<WidgetPtr> childs_;
  // クエリ用
  std::shared_ptr<std::map<std::string, WidgetPtr>> widgets_ = std::make_shared<std::map<std::string, WidgetPtr>>();

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
  Widget(std::string identifier, const ci::vec2& position, const ci::vec2& size, DrawFunc drawer) noexcept
    : identifier_(std::move(identifier)),
      position_(position),
      size_(size),
      drawer_(drawer)
  {
  }
  
  ~Widget() = default;


  // FIXME:上流でシングルタッチ判定を行う
  // TODO:酷いコピペを減らす
  void touchBegan(const Touch& touch, const ci::vec2& parent_position, const ci::vec2& parent_size)
  {
    // TIPS:子供も含めて判定しない
    if (!display_) return;

    ci::vec2 size = getSize(parent_size); 
    ci::vec2 pos  = getPosition(position_, size, parent_size) + parent_position;

    if (execTouchEvent())
    {
      if (testPointRect(touch.getPos(), pos, pos + size))
      {
        // タッチイベント発生
        touching_ = true;
        events_(*this, TouchEvent::BEGAN, touch);
      }
    }
    
    for (auto& widget : childs_)
    {
      widget->touchBegan(touch, pos, size);
    }
  }

  void touchMoved(const Touch& touch, const ci::vec2& parent_position, const ci::vec2& parent_size)
  {
    // TIPS:子供も含めて判定しない
    if (!display_) return;

    ci::vec2 size = getSize(parent_size); 
    ci::vec2 pos  = getPosition(position_, size, parent_size) + parent_position;

    if (execTouchEvent() && touching_)
    {
      bool prev_in = testPointRect(touch.getPrevPos(), pos, pos + size);
      bool cur_in  = testPointRect(touch.getPos(), pos, pos + size);

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
      widget->touchMoved(touch, pos, size);
    }
  }

  void touchEnded(const Touch& touch, const ci::vec2& parent_position, const ci::vec2& parent_size)
  {
    // TIPS:子供も含めて判定しない
    if (!display_) return;

    ci::vec2 size = getSize(parent_size); 
    ci::vec2 pos  = getPosition(position_, size, parent_size) + parent_position;

    if (execTouchEvent() && touching_)
    {
      touching_ = false;

      TouchEvent event = testPointRect(touch.getPos(), pos, pos + size) ? TouchEvent::ENDED_IN
                                                                        : TouchEvent::ENDED_OUT;
      events_(*this, event, touch);
    }
    
    for (auto& widget : childs_)
    {
      widget->touchEnded(touch, pos, size);
    }
  }


  void draw(const ci::vec2& parent_position, const ci::vec2& parent_size) noexcept
  {
    // TIPS:子供も含めて非表示
    if (!display_) return;
    
    ci::vec2 size = getSize(parent_size); 
    ci::vec2 pos  = getPosition(position_, size, parent_size) + parent_position;

    drawer_(*this, pos, size);

    for (const auto& widget : childs_)
    {
      widget->draw(pos, size);
    }
  }
  

  // 識別子
  const std::string& getIdentifier() const noexcept
  {
    return identifier_;
  }
  
  // 各種設定
  void setAnchor(const Anchor pivot, const Anchor position) noexcept
  {
    anchor_pivot_    = pivot;
    anchor_position_ = position;
  }

  // 縦方向は親のサイズに従う
  void setVerticalScaling(boost::optional<float> value) noexcept
  {
    vertical_scaling_ = value;
  }

  // 横方向は親のサイズに従う
  void setHorizontalScaling(boost::optional<float> value) noexcept
  {
    horizontal_scaling_ = value;
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
  
  
  void addChild(const WidgetPtr& widget) noexcept
  {
    childs_.push_back(widget);
    
    widget->widgets_ = widgets_;
    widgets_->insert({ widget->identifier_, widget });
  }

  WidgetPtr find(const std::string& identifier) noexcept
  {
    return widgets_->at(identifier);
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

  
#if 0
  template<typename T>
  void setParam(const std::string& key, const T& param)
  {
    params_.insert({ key, param });
  }

  template<typename T>
  T& getParam(const std::string& key) noexcept
  {
    return boost::any_cast<T&>(params_.at(key));
  }

  template<typename T>
  const T& getParam(const std::string& key) const noexcept
  {
    return boost::any_cast<const T&>(params_.at(key));
  }
#endif


  template<typename F>
  Connection connect(F callback) noexcept
  {
    return events_.connect_extended(callback);
  }

  
private:
  // スケーリングを適用したサイズを取得
  ci::vec2 getSize(const ci::vec2& parent_size) noexcept
  {
    ci::vec2 size = size_;

    if (horizontal_scaling_) size.x = parent_size.x * *horizontal_scaling_;
    if (vertical_scaling_)   size.y = parent_size.y * *vertical_scaling_;
    
    return size;
  }
  
  // アンカーを適用した表示位置を取得
  ci::vec2 getPosition(ci::vec2 pos, const ci::vec2& size, const ci::vec2& parent_size) noexcept
  {
    switch (anchor_pivot_)
    {
    case Anchor::TOP_LEFT:
      pos.y -= size.y;
      break;

    case Anchor::TOP_CENTER:
      pos.x -= size.x / 2;
      pos.y -= size.y;
      break;

    case Anchor::TOP_RIGHT:
      pos.x -= size.x;
      pos.y -= size.y;
      break;

      
    case Anchor::MIDDLE_LEFT:
      pos.y -= size.y / 2;
      break;

    case Anchor::MIDDLE_CENTER:
      pos.x -= size.x / 2;
      pos.y -= size.y / 2;
      break;

    case Anchor::MIDDLE_RIGHT:
      pos.x -= size.x;
      pos.y -= size.y / 2;
      break;


    case Anchor::BOTTOM_LEFT:
      break;

    case Anchor::BOTTOM_CENTER:
      pos.x -= size.x / 2;
      break;

    case Anchor::BOTTOM_RIGHT:
      pos.x -= size.x;
      break;
    }

    
    switch (anchor_position_)
    {
    case Anchor::TOP_LEFT:
      pos.y += parent_size.y;
      break;

    case Anchor::TOP_CENTER:
      pos.x += parent_size.x / 2;
      pos.y += parent_size.y;
      break;

    case Anchor::TOP_RIGHT:
      pos.x += parent_size.x;
      pos.y += parent_size.y;
      break;

      
    case Anchor::MIDDLE_LEFT:
      pos.y += parent_size.y / 2;
      break;

    case Anchor::MIDDLE_CENTER:
      pos.x += parent_size.x / 2;
      pos.y += parent_size.y / 2;
      break;

    case Anchor::MIDDLE_RIGHT:
      pos.x += parent_size.x;
      pos.y += parent_size.y / 2;
      break;


    case Anchor::BOTTOM_LEFT:
      break;

    case Anchor::BOTTOM_CENTER:
      pos.x += parent_size.x / 2;
      break;

    case Anchor::BOTTOM_RIGHT:
      pos.x += parent_size.x;
      break;
    }

    return pos;
  }
};

} }
