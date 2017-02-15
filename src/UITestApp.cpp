//
// UIテスト
// 

#include <cinder/app/App.h>
#include <cinder/app/RendererGl.h>
#include <cinder/gl/gl.h>

#include "Defines.hpp"
#include "Asset.hpp"
#include "Params.hpp"
#include "JsonUtil.hpp"
#include "Touch.hpp"
#include "Worker.hpp"


namespace ngs {

class UITestApp : public ci::app::App
{
  ci::JsonTree params_;

  // タッチ操作中の数
  int touch_num_ = 0;
  ci::ivec2 mouse_prev_pos_;

  // アプリ外枠
  std::unique_ptr<Worker> worker_;


  // タッチ座標→UI座標
  static ci::vec2 calcScreenPosition(const ci::ivec2& pos) noexcept
  {
    // 画面中央を原点とする座標系
    ci::ivec2 half_size = ci::app::getWindowSize() / 2;

    return ci::vec2(pos.x - half_size.x, half_size.y - pos.y);
  }

  
  static std::vector<Touch> createTouchInfo(const std::vector<ci::app::TouchEvent::Touch>& touches) noexcept
  {
    std::vector<Touch> app_touches;
    // TIPS:要素数が確定しているので予め確保できる
    app_touches.reserve(touches.size());

    for (const auto& t : touches)
    {
      Touch touch{ t.getId(), calcScreenPosition(t.getPos()), calcScreenPosition(t.getPrevPos()), false };
      app_touches.push_back(std::move(touch));
    }

    return app_touches;
  }

  
public:
  UITestApp() noexcept
  : params_(Params::load("params.json")),
    worker_(std::unique_ptr<Worker>(new Worker))
  {
  }

  
	void mouseDown(ci::app::MouseEvent event) noexcept override
  {
    // タッチ判定との整合性を取るため左クリック以外は無視
    if (!event.isLeft()) return;

    const auto& pos = event.getPos();
    Touch touch(std::numeric_limits<uint32_t>::max(),
                calcScreenPosition(pos),
                calcScreenPosition(pos),
                true);

    mouse_prev_pos_ = event.getPos();

    touch_num_ += 1;
    worker_->touchesBegan(touch_num_, { touch });
  }

	void mouseDrag(ci::app::MouseEvent event) noexcept override
  {
    if (!event.isLeftDown()) return;

    const auto& pos = event.getPos();
    Touch touch(std::numeric_limits<uint32_t>::max(),
                calcScreenPosition(pos),
                calcScreenPosition(mouse_prev_pos_),
                true);

    mouse_prev_pos_ = pos;
    worker_->touchesMoved(touch_num_, { touch });
  }

	void mouseUp(ci::app::MouseEvent event) noexcept override
  {
    if (!event.isLeft()) return;

    Touch touch(std::numeric_limits<uint32_t>::max(),
                calcScreenPosition(event.getPos()),
                calcScreenPosition(mouse_prev_pos_),
                true);

    touch_num_ = std::max(touch_num_ - 1, 0);
    worker_->touchesEnded(touch_num_, { touch });
  }

  
  void touchesBegan(ci::app::TouchEvent event) noexcept override
  {
    const auto& touches = event.getTouches();
    touch_num_ += touches.size();

    auto app_touches = createTouchInfo(touches);
    worker_->touchesBegan(touch_num_, app_touches);
  }
  
  void touchesMoved(ci::app::TouchEvent event) noexcept override
  {
    const auto& touches = event.getTouches();
    auto app_touches = createTouchInfo(touches);
    worker_->touchesMoved(touch_num_, app_touches);
  }
  
  void touchesEnded(ci::app::TouchEvent event) noexcept override
  {
    const auto& touches = event.getTouches();
    touch_num_ = std::max(touch_num_ - int(touches.size()), 0);

    auto app_touches = createTouchInfo(touches);
    worker_->touchesEnded(touch_num_, app_touches);
  }

  
  void keyDown(ci::app::KeyEvent event) noexcept override
  {
    // TODO:Soft Reset
  }

  
  void resize() noexcept override
  {
    worker_->resize();
  }

  void update() noexcept override
  {
    worker_->update();
  }
  
	void draw() noexcept override
  {
    worker_->draw();
  }
};

}


CINDER_APP(ngs::UITestApp, ci::app::RendererGl,
           [](ci::app::App::Settings* settings) noexcept
           {
             // FIXME:ここで設定ファイルを読むなんて...
             auto params = ngs::Params::load("params.json");

             settings->setWindowSize(ngs::Json::getVec<ci::ivec2>(params["app.size"]));
             settings->setMultiTouchEnabled();

             settings->setTitle(PREPRO_TO_STR(PRODUCT_NAME));
           })
