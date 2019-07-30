#version 330

// Varying変数

in vec2  f_texPos;
// ディスプレイへの出力変数
out vec4 out_color;

// 選択を判定するためのID
uniform int u_selectID;
uniform sampler2D u_texture;

void main() {
     if (u_selectID > 0) {
         // 選択のIDが0より大きければIDで描画する
        float c = u_selectID / 255.0;
        out_color = vec4(c, c, c, 1.0);
     } else {
        // 描画色を代入
     
            out_color =  vec4(texture(u_texture, f_texPos).rgb, 1.0);
       

     }
}

