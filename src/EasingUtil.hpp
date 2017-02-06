#pragma once

//
// Ease関連
//

#include <map>
#include <string>
#include <cinder/Easing.h>


namespace ngs {

namespace EaseParam {

// FIXME:グローバル変数を排除
float elastic_in_a    = 2;
float elastic_in_b    = 1;
float elastic_out_a   = 2;
float elastic_out_b   = 1;
float elastic_inout_a = 2;
float elastic_inout_b = 1;
float elastic_outin_a = 2;
float elastic_outin_b = 1;

}


ci::EaseFn getEaseFunc(const std::string& name) noexcept
{
  static const std::map<std::string, ci::EaseFn> tbl = {
    { "EaseInQuad",    ci::EaseInQuad() },
    { "EaseOutQuad",   ci::EaseOutQuad() },
    { "EaseInOutQuad", ci::EaseInOutQuad() },
    { "EaseOutInQuad", ci::EaseOutInQuad() },

    { "EaseInCubic",    ci::EaseInCubic() },
    { "EaseOutCubic",   ci::EaseOutCubic() },
    { "EaseInOutCubic", ci::EaseInOutCubic() },
    { "EaseOutInCubic", ci::EaseOutInCubic() },

    { "EaseInQuart",    ci::EaseInQuart() },
    { "EaseOutQuart",   ci::EaseOutQuart() },
    { "EaseInOutQuart", ci::EaseInOutQuart() },
    { "EaseOutInQuart", ci::EaseOutInQuart() },

    { "EaseInQuint",    ci::EaseInQuint() },
    { "EaseOutQuint",   ci::EaseOutQuint() },
    { "EaseInOutQuint", ci::EaseInOutQuint() },
    { "EaseOutInQuint", ci::EaseOutInQuint() },

    { "EaseInSine",    ci::EaseInSine() },
    { "EaseOutSine",   ci::EaseOutSine() },
    { "EaseInOutSine", ci::EaseInOutSine() },
    { "EaseOutInSine", ci::EaseOutInSine() },

    { "EaseInExpo",    ci::EaseInExpo() },
    { "EaseOutExpo",   ci::EaseOutExpo() },
    { "EaseInOutExpo", ci::EaseInOutExpo() },
    { "EaseOutInExpo", ci::EaseOutInExpo() },

    { "EaseInCirc",    ci::EaseInCirc() },
    { "EaseOutCirc",   ci::EaseOutCirc() },
    { "EaseInOutCirc", ci::EaseInOutCirc() },
    { "EaseOutInCirc", ci::EaseOutInCirc() },

    { "EaseInAtan",    ci::EaseInAtan() },
    { "EaseOutAtan",   ci::EaseOutAtan() },
    { "EaseInOutAtan", ci::EaseInOutAtan() },
    { "EaseNone",      ci::EaseNone() },

    { "EaseInBack",    ci::EaseInBack() },
    { "EaseOutBack",   ci::EaseOutBack() },
    { "EaseInOutBack", ci::EaseInOutBack() },
    { "EaseOutInBack", ci::EaseOutInBack() },

    { "EaseInBounce",    ci::EaseInBounce() },
    { "EaseOutBounce",   ci::EaseOutBounce() },
    { "EaseInOutBounce", ci::EaseInOutBounce() },
    { "EaseOutInBounce", ci::EaseOutInBounce() },

    { "EaseInElastic",    ci::EaseInElastic(EaseParam::elastic_in_a, EaseParam::elastic_in_b) },
    { "EaseOutElastic",   ci::EaseOutElastic(EaseParam::elastic_out_a, EaseParam::elastic_out_b) },
    { "EaseInOutElastic", ci::EaseInOutElastic(EaseParam::elastic_inout_a, EaseParam::elastic_inout_b) },
    { "EaseOutInElastic", ci::EaseOutInElastic(EaseParam::elastic_outin_a, EaseParam::elastic_outin_b) },
  };

  return tbl.at(name);
}

}
