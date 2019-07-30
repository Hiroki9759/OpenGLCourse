/*

static int WIN_WIDTH   = 500;                       // ウィンドウの幅
static int WIN_HEIGHT  = 500;                       // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Course";     // ウィンドウのタイトル
static float theta = 0.0f;

// シェーダファイル
static std::string VERT_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.vert";
static std::string FRAG_SHADER_FILE = std::string(SHADER_DIRECTORY) + "render.frag";
//オブジェクトファイル
static std::string TEX_FILE = std::string(DATA_DIRECTORY) + "drop.png";

// 頂点オブジェクト
struct Vertex {
    Vertex(const glm::vec3 &position_, const glm::vec2 &tex_)
        : position(position_)
        , tex(tex_) {
    }
    glm::vec3 position;
    glm::vec2 tex;
};

static const  glm::vec3 positions[4]={ 
    glm::vec3(-1.0f,-1.0f,0.0f),
    glm::vec3(-0.75f,-1.0f,0.0f),
    glm::vec3(-1.0f,-0.75f,0.0f),
    glm::vec3(-0.75,-0.75f,0.0f)
};
static const unsigned int drops[2][3] = {
     { 1, 0, 2 }, { 1, 3, 2 }
};

float dx = 1.0f / 4.0f, dy = 1.0f / 8.0f;
static const  glm::vec2 tex_pos[7] = {
    glm::vec2(1 / 4.0f,  1 / 8.0f),
    glm::vec2(1 / 4.0f,  3 / 8.0f),
    glm::vec2(1 / 4.0f,  5 / 8.0f), 
    glm::vec2(1 / 4.0f,  7 / 8.0f), 
    glm::vec2(3 / 4.0f,  3 / 8.0f),
    glm::vec2(3 / 4.0f,  5 / 8.0f), 
    glm::vec2(3 / 4.0f,  7 / 8.0f)    
};

static const glm::vec2 tex_offset[6] = {
	glm::vec2(-dx, dy), glm::vec2(dx, dy),glm::vec2(dx, -dy),glm::vec2(-dx, dy),glm::vec2(-dx,-dy),glm::vec2(dx,-dy)
	
};

// バッファを参照する番号
GLuint vaoId;
GLuint vertexBufferId;
GLuint indexBufferId;

// シェーダを参照する番号
GLuint programId;
GLuint textureId;
// Arcballコントロールのための変数
bool isDragging = false;
enum ArcballMode {
    ARCBALL_MODE_NONE = 0x00,
    ARCBALL_MODE_TRANSLATE = 0x01,
    ARCBALL_MODE_ROTATE = 0x02,
    ARCBALL_MODE_SCALE = 0x04
};
int arcballMode = ARCBALL_MODE_NONE;

//glm::mat4  projMat;
int selectObject = 0;
bool selectMode = false;
glm::vec3 gravity;
glm::ivec2 oldPos;
glm::ivec2 newPos;

struct Operate{
    glm::mat4 modelMat,acTransMat;
    static int cubenumber;
    int cubeId;
    // int droptype;
    
    // Droptype(){
    //     droptype++;
    
    // }

    Operate(){
        cubenumber++;
        cubeId = cubenumber;
    }
 
    void initialize() {
        // l = std::rand()%7;
        // その他の行列の初期化
        modelMat = glm::mat4(1.0);
        acTransMat = glm::mat4(1.0);
    }
    void draw(){
        glm::mat4 mvpMat =  modelMat * acTransMat;

        // Uniform変数の転送
        GLuint uid;
        uid = glGetUniformLocation(programId, "u_mvpMat");
        glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));
        uid = glGetUniformLocation(programId, "u_selectID");
        glUniform1i(uid, selectMode ? cubeId : -1);
        glDrawElements(GL_TRIANGLES, 48, GL_UNSIGNED_INT, 0);
    }
};
int Operate::cubenumber = 0;
Operate  operate[64];
// int Operate::droptype= 0;
// Operate droptype[7];

// struct MakeDrop{
//     static int droptype;
//     int droptypeId;
//     MakeDrop(){
//         droptype++;
//         droptypeId = droptype;
//     }

//     void inits(){
//     std::vector<Vertex> vertices;
//     std::vector<unsigned int> indices;
//     int idx = 0;
//     gravity = glm::vec3(0.0f, 0.0f, 0.0f);
//     for (int k = 0; k < 2; k++) {
//         for (int j = 0; j < 3; j++) {
//             Vertex v(positions[drops[k][j]], tex_pos[droptype]+tex_offset[j+k*3]);
//             vertices.push_back(v);
//             indices.push_back(idx++);
//             gravity += v.position;
//         }
//     }
//     gravity /= indices.size();
//     }
// };
// int MakeDrop::droptype = 0;
// MakeDrop makeDrop[7];
 
 

 


// VAOの初期化
void initVAO() {
    
    // Vertex配列の作成
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    int idx = 0;
   
    gravity = glm::vec3(0.0f, 0.0f, 0.0f);
   
    
    int l= 0; 
    l = std::rand()%7;
    for (int k = 0; k < 2; k++) {
        for (int j = 0; j < 3; j++){

            Vertex v(positions[drops[k][j]], tex_pos[l]+tex_offset[j+k*3]);
            vertices.push_back(v);
            indices.push_back(idx++);
            gravity += v.position;
        }
    }
    
   
    gravity /= indices.size();
   
    // VAOの作成
    glGenVertexArrays(1, &vaoId);
    glBindVertexArray(vaoId);

    // 頂点バッファの作成
    glGenBuffers(1, &vertexBufferId);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // 頂点バッファの有効化
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex));

    // 頂点番号バッファの作成
    glGenBuffers(1, &indexBufferId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
                 indices.data(), GL_STATIC_DRAW);

    // VAOをOFFにしておく
    glBindVertexArray(0);
}

GLuint compileShader(const std::string &filename, GLuint type) {
    // シェーダの作成
    GLuint shaderId = glCreateShader(type);
    
    // ファイルの読み込み
    std::ifstream reader;
    size_t codeSize;
    std::string code;

    // ファイルを開く
    reader.open(filename.c_str(), std::ios::in);
    if (!reader.is_open()) {
        // ファイルを開けなかったらエラーを出して終了
        fprintf(stderr, "Failed to load a shader: %s\n", VERT_SHADER_FILE.c_str());
        exit(1);
    }

    // ファイルをすべて読んで変数に格納 (やや難)
    reader.seekg(0, std::ios::end);             // ファイル読み取り位置を終端に移動 
    codeSize = reader.tellg();                  // 現在の箇所(=終端)の位置がファイルサイズ
    code.resize(codeSize);                      // コードを格納する変数の大きさを設定
    reader.seekg(0);                            // ファイルの読み取り位置を先頭に移動
    reader.read(&code[0], codeSize);            // 先頭からファイルサイズ分を読んでコードの変数に格納

    // ファイルを閉じる
    reader.close();

    // コードのコンパイル
    const char *codeChars = code.c_str();
    glShaderSource(shaderId, 1, &codeChars, NULL);
    glCompileShader(shaderId);

    // コンパイルの成否を判定する
    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus == GL_FALSE) {
        // コンパイルが失敗したらエラーメッセージとソースコードを表示して終了
        fprintf(stderr, "Failed to compile a shader!\n");

        // エラーメッセージの長さを取得する
        GLint logLength;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            // エラーメッセージを取得する
            GLsizei length;
            std::string errMsg;
            errMsg.resize(logLength);
            glGetShaderInfoLog(shaderId, logLength, &length, &errMsg[0]);

            // エラーメッセージとソースコードの出力
            fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
            fprintf(stderr, "%s\n", code.c_str());
        }
        exit(1);
    }

    return shaderId;
}

GLuint buildShaderProgram(const std::string &vShaderFile, const std::string &fShaderFile) {
    // シェーダの作成
    GLuint vertShaderId = compileShader(vShaderFile, GL_VERTEX_SHADER);
    GLuint fragShaderId = compileShader(fShaderFile, GL_FRAGMENT_SHADER);
    
    // シェーダプログラムのリンク
    GLuint programId = glCreateProgram();
    glAttachShader(programId, vertShaderId);
    glAttachShader(programId, fragShaderId);
    glLinkProgram(programId);
    
    // リンクの成否を判定する
    GLint linkState;
    glGetProgramiv(programId, GL_LINK_STATUS, &linkState);
    if (linkState == GL_FALSE) {
        // リンクに失敗したらエラーメッセージを表示して終了
        fprintf(stderr, "Failed to link shaders!\n");

        // エラーメッセージの長さを取得する
        GLint logLength;
        glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            // エラーメッセージを取得する
            GLsizei length;
            std::string errMsg;
            errMsg.resize(logLength);
            glGetProgramInfoLog(programId, logLength, &length, &errMsg[0]);

            // エラーメッセージを出力する
            fprintf(stderr, "[ ERROR ] %s\n", errMsg.c_str());
        }
        exit(1);
    }
    
    // シェーダを無効化した後にIDを返す
    glUseProgram(0);
    return programId;
}

// シェーダの初期化
void initShaders() {
    programId = buildShaderProgram(VERT_SHADER_FILE, FRAG_SHADER_FILE);
}

void initTexture() {

	// テクスチャの設定
	int texWidth, texHeight, channels;
	unsigned char *bytes = stbi_load(TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);

	if (!bytes) {
		fprintf(stderr, "Failed to load image file: %s\n", TEX_FILE.c_str());
		exit(1);
	}

	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, bytes);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(bytes);
}

// OpenGLの初期化関数
void initializeGL() {
    // 深度テストの有効化
    glEnable(GL_DEPTH_TEST);

    // 背景色の設定 (黒)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // VAOの初期化
    initVAO();

    // シェーダの用意
    initShaders();
    //テクスチャの用意
    initTexture();
 
    for(int i = 0;i<64;i++){
        
        operate[i].initialize();
        operate[i].modelMat = glm::translate(operate[i].modelMat, glm::vec3((i%8)*0.25f, (i/8)*0.25f, 0.0f));
    
    }
    
   
}

// OpenGLの描画関数
void paintGL() {
    // 背景色の描画
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // シェーダの有効化
    glUseProgram(programId);
        
    // VAOの有効化
    glBindVertexArray(vaoId);
    // テクスチャの有効化とシェーダへの転送
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureId);
    
    
    //立方体を描画
    for(int i = 0;i<64;i++){
    
    operate[i].draw();
    }
    
    // VAOの無効化
    glBindVertexArray(0);

    // シェーダの無効化
    glUseProgram(0);
    // テクスチャの無効化
	glActiveTexture(GL_TEXTURE0);
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

// スクリーン上の位置をアークボール球上の位置に変換する関数
glm::vec3 getVector(double x, double y) {
    glm::vec3 pt( 2.0 * x / WIN_WIDTH  - 1.0,
                 -2.0 * y / WIN_HEIGHT + 1.0, 0.0);

    const double xySquared = pt.x * pt.x + pt.y * pt.y;
    if (xySquared <= 1.0) {
        // 単位円の内側ならz座標を計算
        pt.z = std::sqrt(1.0 - xySquared);
    } else {
        // 外側なら球の外枠上にあると考える
        pt = glm::normalize(pt);
    }

    return pt;
}

void mouseEvent(GLFWwindow *window, int button, int action, int mods) {
    // クリックされた位置を取得
    double px, py;
    glfwGetCursorPos(window, &px, &py);

    if (action == GLFW_PRESS) {
       const int cx = (int)px;
       const int cy = (int)py; 
        
        // 選択モードでの描画
        selectMode = true;
        paintGL();
        selectMode = false;
        // ピクセルの大きさの計算 (Macの場合には必要)
        int renderBufferWidth, renderBufferHeight;
        glfwGetFramebufferSize(window, &renderBufferWidth, &renderBufferHeight);
        int pixelSize = std::max(renderBufferWidth / WIN_WIDTH, renderBufferHeight / WIN_HEIGHT);
        // より適切なやり方
        unsigned char byte[4];
        glReadPixels(cx*pixelSize, (WIN_HEIGHT - cy - 1)*pixelSize, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &byte);
        printf("Mouse position: %d %d\n", cx, cy);
        printf("Select object %d\n", (int)byte[0]);
        selectObject = (int)byte[0]-1;
    }

    if (!isDragging) {
        isDragging = true;
        oldPos = glm::ivec2(px, py);
        newPos = glm::ivec2(px, py);
            // if((newPos.x>byte[0]%8*0.25f)||(newPos.x<(byte[0]%8-1)*0.25f)){

            // }
    }else {
        isDragging = false;
        oldPos = glm::ivec2(0, 0);
        newPos = glm::ivec2(0, 0);
        arcballMode = ARCBALL_MODE_NONE;
    }
            
}


void updateTranslate() {
    // オブジェクト重心のスクリーン座標を求める
     glm::vec4 gravityScreenSpace = (  operate[selectObject].modelMat) * glm::vec4(gravity.x, gravity.y, gravity.z, 1.0f);
    gravityScreenSpace /= gravityScreenSpace.w;

    // スクリーン座標系における移動量
     glm::vec4 newPosScreenSpace(2.0 * newPos.x / WIN_WIDTH, -2.0 * newPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);
     glm::vec4 oldPosScreenSpace(2.0 * oldPos.x / WIN_WIDTH, -2.0 * oldPos.y / WIN_HEIGHT, gravityScreenSpace.z, 1.0f);

    // スクリーン座標の情報をオブジェクト座標に変換する行列
    const glm::mat4 s2oMat = glm::inverse(  operate[selectObject].modelMat);
    
    // スクリーン空間の座標をオブジェクト空間に変換
    glm::vec4 newPosObjSpace = s2oMat* newPosScreenSpace;
    glm::vec4 oldPosObjSpace = s2oMat * oldPosScreenSpace;
    newPosObjSpace /= newPosObjSpace.w;
    oldPosObjSpace /= oldPosObjSpace.w;
    // オブジェクト座標系での移動量
    const glm::vec3 transObjSpace= glm::vec3(newPosObjSpace - oldPosObjSpace);

    // オブジェクト空間での平行移動
    operate[selectObject].acTransMat = glm::translate(operate[selectObject].acTransMat, transObjSpace);
}


// void updateMouse() {
//    updateTranslate(); 
   
// }
void mouseMoveEvent(GLFWwindow *window, double xpos, double ypos) {
    if (isDragging) {
        // マウスの現在位置を更新
        newPos = glm::ivec2(xpos, ypos);

        // マウスがあまり動いていない時は処理をしない
        const double dx = newPos.x - oldPos.x;
        const double dy = newPos.y - oldPos.y;
        const double length = dx * dx + dy * dy;
        if (length < 2.0f * 2.0f) {
            return;
        } else {
            if(selectObject >= 0){
            updateTranslate();
            oldPos = glm::ivec2(xpos, ypos);
            }
        }
    }
}


int main(int argc, char **argv) {
    // OpenGLを初期化する
    if (glfwInit() == GL_FALSE) {
        fprintf(stderr, "Initialization failed!\n");
        return 1;
    }

    // OpenGLのバージョン設定 (Macの場合には必ず必要)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

    // マウスのイベントを処理する関数を登録
    glfwSetMouseButtonCallback(window, mouseEvent);
    glfwSetCursorPosCallback(window, mouseMoveEvent);
  
  　
    // OpenGL 3.x/4.xの関数をロードする (glfwMakeContextCurrentの後でないといけない)
    const int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        fprintf(stderr, "Failed to load OpenGL 3.x/4.x libraries!\n");
        return 1;
    }

    // バージョンを出力する
    printf("Load OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    // ウィンドウのリサイズを扱う関数の登録
    glfwSetWindowSizeCallback(window, resizeGL);

    // OpenGLを初期化
    initializeGL();

    // メインループ
    while (glfwWindowShouldClose(window) == GL_FALSE) {
        // 描画
        
        paintGL();
        
        // 描画用バッファの切り替え
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}
*/
#include <cmath>
#include <ctime> 
#include <cstdlib>    
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>



//fieldのマス目をAAで描画するためにprintfをつかえるようにする
#include<stdio.h>
 
//systemかんすうを使えるようにする
#include<stdlib.h>
 
//文字列操作関数を使えるようにする
#include<string.h>
 
//乱数を取得すらために現在の時刻を取得する
#include<time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// ディレクトリの設定ファイル
#include "common.h"

 
//フィールドの実装
#define FIELD_WIDTH 8
#define FIELD_HEIGHT 8
 
//ブロックの種類数の定義
#define BLOCK_TYPE_MAX 7
 
//フィールドについて、何もない状態を足して８種類の状態を列挙定数で定義
enum {
           CELL_TYPE_NONE,
           CELL_TYPE_BLOCK0,
           CELL_TYPE_BLOCK1,
           CELL_TYPE_BLOCK2,
           CELL_TYPE_BLOCK3,
           CELL_TYPE_BLOCK4,
           CELL_TYPE_BLOCK5,
           CELL_TYPE_BLOCK6,
           CELL_TYPE_MAX,
};
 
//それぞれのセルがどのようなAAで表示するかを文字列で表記する
char cellAA[][2+2] = {
           "・",//CELL_TYPE_NONE,
           "〇",//CELL_TYPE_BLOCK0,
           "△",//CELL_TYPE_BLOCK1,
           "□",//CELL_TYPE_BLOCK2,
           "●",//CELL_TYPE_BLOCK3,
           "▲",//CELL_TYPE_BLOCK4,
           "■",//CELL_TYPE_BLOCK5,
           "★"//CELL_TYPE_BLOCK6,
};
 
//それぞれのセルの状態を保持する変数を定義する
int cells[FIELD_WIDTH][FIELD_HEIGHT];
 
//ブロックごとにチェック済みかどうかのフラグを用意する
int checked[FIELD_WIDTH][FIELD_HEIGHT];
 
//ブロックのカーソル選択時のカーソルの位置を定義
int cursorX, cursorY;
 
//選択している石の座標の定義
int selectedX=-1, selectedY=-1;
 
//ロックの定義
bool locked = true
;
 
//ブロックの連結数を取得する関数の作成
int getConnectedBlockCount(int _x, int _y, int _cellType, int _count) {
           if (
                      (_x < 0) || (_x >= FIELD_WIDTH) || (_y < 0) || (_y >= FIELD_HEIGHT)
                      || checked[_y][_x]
                      || (cells[_y][_x] == CELL_TYPE_NONE)
                      || (cells[_y][_x] != _cellType)
                      )
                      return _count;
 
           _count++;
           checked[_y][_x] = true;
 
           _count = getConnectedBlockCount(_x, _y-1, _cellType, _count);
           _count = getConnectedBlockCount(_x-1, _y, _cellType, _count);
           _count = getConnectedBlockCount(_x, _y+1, _cellType, _count);
           _count = getConnectedBlockCount(_x+1, _y, _cellType, _count);
}
void eraseConnectedBlocks(int _x, int _y, int _cellType) {
           if (
                      (_x < 0) || (_x >= FIELD_WIDTH) || (_y < 0) || (_y >= FIELD_HEIGHT)
                      || (cells[_y][_x] == CELL_TYPE_NONE)
                      || (cells[_y][_x] != _cellType)
                      )
                      return;
           cells[_y][_x] = CELL_TYPE_NONE;
           eraseConnectedBlocks(_x, _y - 1, _cellType);
           eraseConnectedBlocks(_x - 1, _y , _cellType);
           eraseConnectedBlocks(_x, _y + 1, _cellType);
           eraseConnectedBlocks(_x + 1, _y, _cellType);
}
//フィールドのすべての連結しているブロックをすべて消す
void eraseConnectedBlocksAll() {
           memset(checked, 0, sizeof checked);
           for (int y = 0; y < FIELD_HEIGHT; y++)
                      for (int x = 0; x < FIELD_WIDTH; x++) {
                                  int n = getConnectedBlockCount(x, y, cells[y][x], 0);
                                  if (n >= 3) {
                                             //他のキーを押したとき選択された石を変更
                                             eraseConnectedBlocks(x, y, cells[y][x]);
                                             locked = true;
                                  }
                      }
}
void display() {
           //画面をクリアする
           system("cls");
 
           //縦方向への繰り返し
           for (int y = 0; y < FIELD_HEIGHT; y++) {
                      //横一行描画
                      for (int x = 0; x < FIELD_WIDTH; x++)
                                  //描画するX,Y座標がカーソルの座標と一致したらカーソル(◎)を表示
                                  if ((x == cursorX) && (y == cursorY) && (!locked))
                                             printf("◎");
                                  else
                                             //AAの描画
                                             printf("%s", cellAA[cells[y][x]]);
                      //選択した石のy座標を表示
                      if (y == selectedY)
                                  printf("←");
                      //改行処理
                      printf("\n");
           }
 
           //選択した石のx座標を表示
           for (int x = 0; x < FIELD_WIDTH; x++)
                      printf("%s", (x == selectedX) ? "↑" : "  ");
 
}
int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
int main() {
//乱数をシャッフルする
//シャッフルするのはtime関数で秒単位の時間を取得する
srand((unsigned int)time(NULL));

//ランダムな石の配置を描画
for (int y = 0; y < FIELD_HEIGHT; y++)
            for (int x = 0; x < FIELD_WIDTH; x++)
                        cells[y][x] = CELL_TYPE_BLOCK0 + rand() % BLOCK_TYPE_MAX;
//時間経過判定用のタイマーを作る
time_t t = time(NULL);

//キーボードキー入力後再描画
while (1) {
    if (t < time(NULL)) {
        t = time(NULL);
        //もしロックがかかっていたら石が降ってくる処理
        if(locked){
        
            locked = false;
            for (int y = FIELD_HEIGHT - 2; y >= 0; y--)
                for (int x = 0; x < FIELD_WIDTH; x++)
                    if ((cells[y][x] != CELL_TYPE_NONE)
                        && (cells[y + 1][x] == CELL_TYPE_NONE)) {
                        cells[y + 1][x] = cells[y][x];
                        cells[y][x] = CELL_TYPE_NONE;
                        locked = true;
                    }
            //石の補充    
            for (int x = 0; x < FIELD_WIDTH; x++)
                if (cells[0][x] == CELL_TYPE_NONE) {
                    cells[0][x] = CELL_TYPE_BLOCK0 + rand() % BLOCK_TYPE_MAX;
                }
            if (!locked) {
                    eraseConnectedBlocksAll();
            }
                
        }

        display();
    }
    //もしキーボードの入力があれば以下の処理を実行する
    if(kbhit()){
        if (locked)
                    getche();
        else {
            //キーボードの入力待ち状態
            //戻り値は入力されたキー
            //戻り値をswitch文にわたし、入力されたキーによって分岐(wasd&その他)
            switch (getche()) {
                case'w':cursorY--; break;//上
                case's':cursorY++; break;//下
                case'a':cursorX--; break;//左
                case'd':cursorX++; break;//右
                default:
                //画面の範囲外で選択されてなければ選択する
                //elseで石を入れ替える 入れ替え後選択を解除
                if (selectedX < 0) {
                    selectedX = cursorX;
                    selectedY = cursorY;
                    //隣接する石でしか入れ替えられないようにするため距離を定義
                }
                else {
                    int distance = abs(selectedY - cursorY) + abs(selectedX - cursorX);
                    if (distance == 0) {
                        //解除機能
                        selectedX = selectedY = -1;
                    }
                    else if (distance == 1) {
                        int temp = cells[cursorY][cursorX];
                        cells[cursorY][cursorX] = cells[selectedY][selectedX];
                        cells[selectedY][selectedX] = temp;

                        eraseConnectedBlocksAll();
                        

                        selectedX = selectedY = -1;
                        locked = true;
                    }
                    else
                        printf("a");
                            
                }

                break;

            }
            //キーボードの入力があれば即座に再描画
            display();
        }
    }

}
}