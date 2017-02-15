#pragma once

//
// 細々とした実装
//

#include <cinder/gl/Shader.h>
#include "Shader.hpp"

namespace ngs {

// 点と矩形の交差
bool testPointRect(const ci::vec2& pos, const ci::vec2& start, const ci::vec2& end)
{
  return (pos.x > start.x)
      && (pos.x <   end.x)
      && (pos.y > start.y)
      && (pos.y <   end.y);
}

// シェーダー作成
ci::gl::GlslProgRef createShader(const std::string& vtx_shader, const std::string& frag_shader) noexcept
{
  auto shader = readShader(vtx_shader, frag_shader);
  return ci::gl::GlslProg::create(shader.first, shader.second);
}

}
