#include <cstdio>
#include <cmath>
#include <string>

#define GLFW_INCLUDE_GLU  // GLUライブラリを使用するのに必要
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "common.h"

static int WIN_WIDTH   = 500;                 // ウィンドウの幅
static int WIN_HEIGHT  = 500;                 // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Course";     // ウィンドウのタイトル

static float theta = 0.0f;

static const std::string TEX_FILE = std::string(DATA_DIRECTORY) + "drop_hi.png";
static GLuint textureId = 0u;
static bool enableMipmap = true;

static const float positions[9][3] = {
    { -2.0f,  0.0f, -2.0f },
    {  0.0f,  0.0f, -2.0f },
    {  2.0f,  0.0f, -2.0f },
    { -2.0f,  0.0f,  0.0f },
    {  0.0f,  0.0f,  0.0f },
    {  2.0f,  0.0f,  0.0f },
    { -2.0f,  0.0f,  2.0f },
    {  0.0f,  0.0f,  2.0f },
    {  2.0f,  0.0f,  2.0f },
    
};

static const float texcoords[4][2] = {
    {  0.0f,  0.0f },
    {  1.0f,  0.0f },
    {  0.0f,  1.0f },
    {  1.0f,  1.0f }
};
static const unsigned int indices1[2][3] = {
   { 1,0,2 }, {1,3,2 }
};
static const unsigned int indices2[8][3] = {
    { 0, 1, 4 }, { 0, 3, 4},
    { 1, 2, 5 }, { 1, 4, 5 },
    { 3, 4, 7 }, { 3, 6,7 },
    { 4, 5, 8 }, { 4, 7,8 }
};

float dx = 1 / 2.0f, dy = 1 / 2.0f;
static const glm::vec2 tex_pos[1] = {
		glm::vec2(1 / 2.0f,  1 / 2.0f) 
};

static const glm::vec2 tex_offset[6] = {
	glm::vec2(0.0, 0.0), glm::vec2(dx, 0.0),glm::vec2(dx, dy),
	glm::vec2(0.0, 0.0), glm::vec2(dx, dy), glm::vec2(0.0, dy)
};
// OpenGLの初期化関数
void initializeGL() {
    // 背景色の設定 (黒)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // 深度テストの有効化
    glEnable(GL_DEPTH_TEST);

    // テクスチャの有効化
    glEnable(GL_TEXTURE_2D);

    // テクスチャの設定
    int texWidth, texHeight, channels;
    unsigned char *bytes = stbi_load(TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
    if (!bytes) {
        fprintf(stderr, "Failed to load image file: %s\n", TEX_FILE.c_str());
        exit(1);
    }

    // テクスチャの生成と有効化
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    
    // 単純なテクスチャの転送
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

    // テクスチャの画素値参照方法の設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // テクスチャ境界の折り返し設定
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // テクスチャの無効化
    glBindTexture(GL_TEXTURE_2D, 0);

    // ロードした画素情報の破棄
    stbi_image_free(bytes);
}

// OpenGLの描画関数
void paintGL() {
    // 背景色と深度値のクリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 座標の変換
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)WIN_WIDTH / (float)WIN_HEIGHT, 1.0f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.1f, 5.0f, 0.1f,     // 視点の位置
              0.1f, -5.0f, -2.0f/3.0f,     // 見ている先
              0.0f, 1.0f, 0.0f);    // 視界の上方向
    

    // 立方体の描画
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBegin(GL_TRIANGLES);
    for(int j=0;j<4;j++){
        for (int i = 0; i < 3; i++) {
          glTexCoord2fv(texcoords[indices1[0][i]]);
          glVertex3fv(positions[indices2[j*2][i]]);
        }
    }
    for(int j=0;j<4;j++){
        for (int i = 0; i < 3; i++) {
          glTexCoord2fv(texcoords[indices1[1][i]]);
          glVertex3fv(positions[indices2[j*2+1][i]]);
        }
    }
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void resizeGL(GLFWwindow *window, int width, int height) {
    // ユーザ管理のウィンドウサイズを変更
    WIN_WIDTH = width;
    WIN_HEIGHT = height;
    
    // GLFW管理のウィンドウサイズを変更
    glfwSetWindowSize(window, WIN_WIDTH, WIN_HEIGHT);

    // 実際のウィンドウサイズ (ピクセル数) を取得
    int renderBufferWidth, renderBufferHeight;
    glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);

    // ビューポート変換の更新
    glViewport(0, 0, renderBufferWidth, renderBufferHeight);
}

// アニメーションのためのアップデート
void animate() {
    theta += 1.0f;  // 1度だけ回転
}

int main(int argc, char **argv) {
    // OpenGLを初期化する
    if (glfwInit() == GL_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }

    // Windowの作成
    GLFWwindow *window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, WIN_TITLE,
                                          NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Window creation failed!");
        glfwTerminate();
        return 1;
    }

    // OpenGLの描画対象にWindowを追加
    glfwMakeContextCurrent(window);

    // ウィンドウのリサイズを扱う関数の登録
    glfwSetWindowSizeCallback(window, resizeGL);

    // OpenGLを初期化
    initializeGL();

    // メインループ
    while (glfwWindowShouldClose(window) == GL_FALSE) {
        // 描画
        paintGL();

        // アニメーション
        animate();

        // 描画用バッファの切り替え
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}