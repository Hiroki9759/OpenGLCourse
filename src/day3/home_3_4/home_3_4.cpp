#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>

#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#define GLFW_INCLUDE_GLU
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/hash.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

// ディレクトリの設定ファイル
#include "common.h"

static int WIN_WIDTH   = 500;                       // ウィンドウの幅
static int WIN_HEIGHT  = 500;                       // ウィンドウの高さ
static const char *WIN_TITLE = "OpenGL Cource";     // ウィンドウのタイトル

static const double PI = 4.0 * std::atan(1.0);

//テクスチャファイル
static const std::string DIFF_TEX_FILE = std::string(DATA_DIRECTORY) + "sweater_diff.png";
static const std::string NORMAL_TEX_FILE = std::string(DATA_DIRECTORY) + "sweater_normal.png";
static GLuint texture0Id = 0u;
static GLuint texture1Id = 0u;

// シェーダファイル
static const std::string VERT_SHADER_FILE =std::string(SHADER_DIRECTORY)+"render.vert";
static const std::string FRAG_SHADER_FILE =std::string(SHADER_DIRECTORY)+ "render.frag";

// モデルファイル
static const std::string OBJECT_FILE = std::string(DATA_DIRECTORY) + "teapot.obj";
static const glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 5.0f);

// 頂点オブジェクト
struct Vertex {
	Vertex(const glm::vec3 &position_, const glm::vec2 texcoord_,
		const glm::vec3 &normal_, const glm::vec3 &tangent_,
		const glm::vec3 &binormal_)
		: position(position_) , texcoord(texcoord_), 
		normal(normal_), tangent(tangent_), binormal(binormal_) {
	}

	glm::vec3 position;
	glm::vec2 texcoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 binormal;
};
//シェーダを参照する番号
GLuint programId;

// 立方体の回転角度
static float theta = 0.0f;

GLuint compileShader(const std::string &filename, GLuint type) {
	// シェーダの作成
	GLuint shaderId = glCreateShader(type);

	//  ファイルの読み込み
	std::ifstream reader;
	size_t codeSize;
	std::string code;

	// ファイルを開く
	reader.open(filename.c_str(), std::ios::in);
	if (!reader.is_open()) {
		//  ファイルを開けなかったらエラーを出して終了
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
		//リンクに失敗したらエラーメッセージを表示して終了
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

			//エラーメッセージを出力する
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

struct VertexArrayObject {
	VertexArrayObject() {};
	VertexArrayObject(const std::string &filename)
		: FILE_NAME(filename) {};

	std::string FILE_NAME;

	// バッファを参照する番号
	GLuint vaoId;
	GLuint vertexBufferId;
	GLuint indexBufferId;

	// インデックスバッファのサイズ (glDrawElementsで使用)
	size_t indexBufferSize;

	void initialize() {
		//モデルのロード
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string err;
		bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, FILE_NAME.c_str());
		if (!err.empty()) {
			std::cerr << "[WARNING] " << err << std::endl;
		}

		if (!success) {
			std::cerr << "Failed to load OBJ file: " << FILE_NAME << std::endl;
			exit(1);
		}

		//  Vertex配列の作成
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		std::unordered_map<glm::vec3, std::vector<unsigned int>> posmap;
		for (int s = 0; s < shapes.size(); s++) {
			const tinyobj::mesh_t &mesh = shapes[s].mesh;
			glm::vec3 temppos[3];
			glm::vec2 tempuv[3];
			int idx = 0;
			for (int i = 0; i < mesh.indices.size(); i++) {
				const tinyobj::index_t &index = mesh.indices[i];

				glm::vec3 position;
				glm::vec2 texcoord;

				if (index.vertex_index >= 0) {
					position = glm::vec3(attrib.vertices[index.vertex_index * 3 + 0],
						attrib.vertices[index.vertex_index * 3 + 1],
						attrib.vertices[index.vertex_index * 3 + 2]);
					temppos[idx] = position;
				}

				if (index.texcoord_index >= 0) {
					texcoord = glm::vec2(attrib.texcoords[index.texcoord_index * 2 + 0],
						attrib.texcoords[index.texcoord_index * 2 + 1]);
					tempuv[idx] = texcoord;
				}

				idx++;

				if (idx == 3) {
					glm::vec3 normal;

					glm::vec3 deltaPos1 = temppos[1] - temppos[0];
					glm::vec3 deltaPos2 = temppos[2] - temppos[0];
					glm::vec2 deltaUV1 = tempuv[1] - tempuv[0];
					glm::vec2 deltaUV2 = tempuv[2] - tempuv[0];

					float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
					glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
					glm::vec3 binormal = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

					normal = glm::cross(tangent, binormal);

					for (int i = 0; i < 3; i++) {
						if (posmap.count(temppos[i]) == 0) {
							const unsigned int vertexIndex = vertices.size();
							vertices.push_back(Vertex(temppos[i], tempuv[i], normal, tangent, binormal));
							indices.push_back(vertexIndex);
							posmap[temppos[i]].push_back(vertexIndex);
						}

						else {
							for (unsigned int vert : posmap[temppos[i]]) {
								vertices[vert].normal += normal;
								vertices[vert].tangent += tangent;
								vertices[vert].binormal += binormal;
								if (vertices[vert].texcoord == tempuv[i]) {
									indices.push_back(vert);
									continue;
								}
								Vertex v = vertices[vert];
								v.texcoord = tempuv[i];
								vertices.push_back(v);
								indices.push_back(vertices.size() - 1);
							}
						}
					}

					idx = 0;
				}
			}
		}

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


		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, binormal));

		// 頂点番号バッファの作成
		glGenBuffers(1, &indexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
			indices.data(), GL_STATIC_DRAW);

		//頂点バッファのサイズを変数に入れておく
		indexBufferSize = indices.size();

		// VAOをOFFにしておく
		glBindVertexArray(0);
	}

	void draw() {
		// 座標の変換
		glm::mat4 projMat = glm::perspective(45.0f,
			(float)WIN_WIDTH / (float)WIN_HEIGHT, 0.1f, 1000.0f);

		glm::mat4 viewMat = glm::lookAt(glm::vec3(3.0f, 4.0f, 5.0f),   //視点の位置
			glm::vec3(0.0f, 0.0f, 0.0f),   // 見ている先
			glm::vec3(0.0f, 1.0f, 0.0f));  // 視界の上方向

		glm::mat4 modelMat = glm::rotate(glm::radians(theta), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::mat4 mvMat = viewMat * modelMat;
		glm::mat4 mvpMat = projMat * viewMat * modelMat;
		glm::mat4 normMat = glm::transpose(glm::inverse(mvMat));
		glm::mat4 lightMat = viewMat;

		// VAOの有効化
		glBindVertexArray(vaoId);

		// シェーダの有効化
		glUseProgram(programId);

		// Uniform変数の転送
		GLuint uid;
		uid = glGetUniformLocation(programId, "u_mvMat");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvMat));
		uid = glGetUniformLocation(programId, "u_mvpMat");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(mvpMat));
		uid = glGetUniformLocation(programId, "u_normMat");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(normMat));
		uid = glGetUniformLocation(programId, "u_lightMat");
		glUniformMatrix4fv(uid, 1, GL_FALSE, glm::value_ptr(lightMat));

		uid = glGetUniformLocation(programId, "u_lightPos");
		glUniform3fv(uid, 1, glm::value_ptr(lightPos));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture0Id);
		uid = glGetUniformLocation(programId, "u_texture0");
		glUniform1i(uid, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1Id);
		uid = glGetUniformLocation(programId, "u_texture1");
		glUniform1i(uid, 1);

		// 三角形の描画
		glDrawElements(GL_TRIANGLES, indexBufferSize, GL_UNSIGNED_INT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// VAOの無効化
		glBindVertexArray(0);

		// シェーダの無効化
		glUseProgram(0);
	}
};

VertexArrayObject vao;

//OpenGLの初期化関数
void initializeGL() {
	

	// 背景色の設定 (黒)
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	vao = VertexArrayObject(OBJECT_FILE);
	vao.initialize();
	// 深度テストの有効化
    glEnable(GL_DEPTH_TEST);

	// シェーダの初期化
	initShaders();

	// テクスチャの設定
	int texWidth, texHeight, channels;
	unsigned char *bytes0 = stbi_load(DIFF_TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
	if (!bytes0) {
		fprintf(stderr, "Failed to load image file: %s\n", DIFF_TEX_FILE.c_str());
		exit(1);
	}

	// テクスチャの生成１
	glGenTextures(1, &texture0Id);
	//テクスチャの有効化１
	glBindTexture(GL_TEXTURE_2D, texture0Id);

	// 単純なテクスチャの転送１
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, bytes0);
	//テクスチャの画素値参照方法の設定１
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//テクスチャ境界の折り返し設定１
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// ロードした画素情報の破棄１
	stbi_image_free(bytes0);

	// ÉeÉNÉXÉ`ÉÉÇÃê›íË
	unsigned char *bytes1 = stbi_load(NORMAL_TEX_FILE.c_str(), &texWidth, &texHeight, &channels, STBI_rgb_alpha);
	if (!bytes1) {
		fprintf(stderr, "Failed to load image file: %s\n", NORMAL_TEX_FILE.c_str());
		exit(1);
	}

	// テクスチャの生成と有効化２
	glGenTextures(1, &texture1Id);
	glBindTexture(GL_TEXTURE_2D, texture1Id);

	// 単純なテクスチャの転送２
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texWidth, texHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, bytes1);

	
	
	// テクスチャの画素値参照方法の設定２
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// テクスチャ境界の折り返し設定２
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// テクスチャの無効化
	glBindTexture(GL_TEXTURE_2D, 0);

	//ロードした画素情報の破棄２
	stbi_image_free(bytes1);
}

//OpenGLの描画関数
void paintGL() {
	// 背景色の描画
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vao.draw();
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
	theta += 0.05f;  
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

		// アニメーション
		animate();

		// 描画用バッファの切り替え
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}