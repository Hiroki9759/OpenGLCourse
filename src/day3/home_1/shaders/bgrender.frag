#version 330 core

uniform vec4 top_color;
uniform vec4 bottom_color;
in vec2 v_uv;
// ディスプレイへの出力変数
out vec4 frag_color;

void main() {
    // 描画色を代入
    frag_color = bottom_color * (1 - v_uv.y) + top_color * v_uv.y;
}