//
// UI 色＋テクスチャ
//
$version$
$precision$

uniform sampler2D	uTex0;
                                                          
in vec4 Color;
in vec2 TexCoord0;

out vec4 oColor;


void main(void) {
  oColor = texture(uTex0, TexCoord0) * Color;
}
