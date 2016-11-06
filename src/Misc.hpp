#pragma once

//
// 細々とした実装
//

namespace ngs {

// 点と矩形の交差
bool testPointRect(const ci::vec2& pos, const ci::vec2& start, const ci::vec2& end)
{
  return (pos.x > start.x)
      && (pos.x <   end.x)
      && (pos.y > start.y)
      && (pos.y <   end.y);
}

}
