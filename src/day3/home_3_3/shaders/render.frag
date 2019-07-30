#version 330

// Varying変数
in vec2 f_texPos;

// ディスプレイへの出力変数
out vec4 out_color;
uniform sampler2D u_texture;

void main() {
    // 描画色を代入
    out_color = vec4(texture(u_texture, f_texPos).rgb, 1.0);
}