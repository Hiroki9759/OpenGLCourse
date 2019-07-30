//glui関連のパラメータ，関数など

#define MAX_SLICE 260//最大分割数（両辺同じ）
#define MAX_SOURCE  50//全波源数

enum WaveMode { SINGLE, CONTINUOUS};
WaveMode mode = SINGLE;

//プロジェクトのメインファイルに実装されている関数
void initObject();

int main_window;//GLUTで作るWindowのID
int flagWireframe = 0;
int flagParamShow = 1;
int flagCoordShow = 0;
float size0 = 50.0; //1辺の長さ
int nSlice0 = 200;  //分割数
int nSource0 = 0;   //波源個数
float sourceY0 = 25.0;//波源のy座標
float interval0 = 1.0; //波源間隔
float amp0 = 2.0;     //振幅
float freq0 = 0.5;     //振動数[Hz]
float waveVel0 = 3.0;  //伝搬速度
float drag0 = 0.1;     //粘性抵抗
float coefB0 = 0.8;    //境界係数
float coefDown0 = 0.001; //へこみ係数
float coefUp0   = 0.02; //持ち上げ係数
int kind0 = 0;         //剛体の種類
float sizeX0 = 3.0;
float sizeY0 = 3.0;
float sizeZ0 = 3.0;
float height0 = 0.0;   //オブジェクトの高さ
float forceY0 = 50000.0;//駆動力
float specificG0 = 0.5;//比重
int valueMode = 0;
int valueBoundary = 1;

//Pointers
GLUI *glui, *glui2;
GLUI_Panel  *animePanel;
GLUI_Button *startBtn;
GLUI_Button *freezeBtn;
GLUI_Button *stepBtn;
GLUI_Button *resetBtn;
GLUI_EditText *timeRate_edit;
GLUI_Panel *stroboPanel;
GLUI_EditText *numStrobo_edit;
GLUI_EditText *periodStrobo_edit;

GLUI_Panel *objectPanel;
GLUI_EditText *kind_edit;
GLUI_EditText *sizeX_edit;
GLUI_EditText *sizeY_edit;
GLUI_EditText *sizeZ_edit;
GLUI_EditText *height_edit;
GLUI_EditText *cInertial_edit;
GLUI_EditText *specificG_edit;
GLUI_EditText *forceY_edit;

GLUI_Panel *wavePanel;
GLUI_EditText *size_edit;
GLUI_EditText *nSlice_edit;
GLUI_EditText *sourceY_edit;
GLUI_EditText *nSource_edit;
GLUI_EditText *amp_edit;
GLUI_EditText *freq_edit;
GLUI_EditText *drag_edit;
GLUI_EditText *waveVel_edit;
GLUI_EditText *interval_edit;
GLUI_EditText *coefB_edit;
GLUI_EditText *coefDown_edit;
GLUI_EditText *coefUp_edit;
GLUI_RadioGroup *radioMode;

//光源
GLUI_Button *lightXP_btn;
GLUI_Button *lightXM_btn;
GLUI_Button *lightYP_btn;
GLUI_Button *lightYM_btn;
GLUI_Button *lightZP_btn;
GLUI_Button *lightZM_btn;
GLUI_Button *resetLight_btn;
GLUI_Panel *cameraOpe_panel;
GLUI_Button *hide_btn;
GLUI_Button *show_btn;
//カメラ操作
GLUI_Button *dollyP_btn;
GLUI_Button *dollyM_btn;
GLUI_Button *panP_btn;
GLUI_Button *panM_btn;
GLUI_Button *tiltP_btn;
GLUI_Button *tiltM_btn;
GLUI_Button *tumbleP_btn;
GLUI_Button *tumbleM_btn;
GLUI_Button *craneP_btn;
GLUI_Button *craneM_btn;
GLUI_Button *zoomP_btn;
GLUI_Button *zoomM_btn;
GLUI_Button *resetCamera_btn;

//USER ID
#define START_ID  100
#define FREEZE_ID 110
#define STEP_ID   120
#define RESET_ID  130
#define LIGHTX_P_ID 210
#define LIGHTX_M_ID 220
#define LIGHTY_P_ID 230
#define LIGHTY_M_ID 240
#define LIGHTZ_P_ID 250
#define LIGHTZ_M_ID 260
#define LIGHT_RESET_ID 270
#define HIDE_ID 280
#define SHOW_ID 290
#define DOLLY_P_ID 310
#define DOLLY_M_ID 320
#define PAN_P_ID 330
#define PAN_M_ID 340
#define TILT_P_ID 350
#define TILT_M_ID 360
#define TUMBLE_P_ID 370
#define TUMBLE_M_ID 380
#define CRANE_P_ID 390
#define CRANE_M_ID 400
#define ZOOM_P_ID 410
#define ZOOM_M_ID 420
#define CAMERA_RESET_ID 430

void control_cb(int control)
{
	//波のモード
	if(valueMode == 0) mode = SINGLE;
	else               mode = CONTINUOUS;

	double v1 = 10.0;//光源
	double v2 = 1.0;//カメラ操作

	//動画操作
	if(control == START_ID) {
		flagStart = true;
		flagFreeze = false;
		initObject();
		elapseTime2 = 0.0;//start後の総経過時間
	}
	else if(control == FREEZE_ID) {
		if(flagFreeze == false) flagFreeze = true;
		else {flagFreeze = false; flagOneStep = false;}
	}
	else if(control == STEP_ID) {
		flagOneStep = true;
		flagFreeze = false;
	}
	else if(control == RESET_ID) {
		flagStart = false;
		flagFreeze = false;
		initObject();
	}
	//光源操作
	else if(control == LIGHTX_P_ID) { lightPos[0] += v1;}
	else if(control == LIGHTX_M_ID) { lightPos[0] -= v1;}
	else if(control == LIGHTY_P_ID) { lightPos[1] += v1;}
	else if(control == LIGHTY_M_ID) { lightPos[1] -= v1;}
	else if(control == LIGHTZ_P_ID) { lightPos[2] += v1;}
	else if(control == LIGHTZ_M_ID) { lightPos[2] -= v1;}
	else if(control == LIGHT_RESET_ID) 
	{
		for(int i = 0; i < 3; i++) lightPos[i] = lightPos0[i];
	}
	
	else if ( control == SHOW_ID )
	{
		glui2->show();
	}
	else if ( control == HIDE_ID )
	{
		glui2->hide();
	}

	//カメラ操作
	else if(control == DOLLY_P_ID || control == DOLLY_M_ID)//dolly
	{
		if(control == DOLLY_P_ID)  view.dist -= v2; //近づく
		else  view.dist += v2; //遠ざかる
		setCamera();
	}
	else if( control == PAN_P_ID || control == PAN_M_ID)//pan
	{
		if(control == PAN_P_ID) view.phi += v2;
		else view.phi -= v2;
		view.vCnt.x = view.vPos.x - view.dist * cos(DEG_TO_RAD * view.phi) * cos(DEG_TO_RAD * view.theta);
		view.vCnt.y = view.vPos.y - view.dist * sin(DEG_TO_RAD * view.phi) * cos(DEG_TO_RAD * view.theta);
	}
	else if(control == TILT_P_ID || control == TILT_M_ID)//tilt
	{
	  if(control == TILT_P_ID)
		  view.theta += v2; 
	  else
		  view.theta -= v2;
	  
	  view.vCnt.x = view.vPos.x - view.dist * (float)(cos(DEG_TO_RAD * view.theta) * cos(DEG_TO_RAD * view.phi));
	  view.vCnt.y = view.vPos.y - view.dist * (float)(cos(DEG_TO_RAD * view.theta) * sin(DEG_TO_RAD * view.phi));
	  view.vCnt.z = view.vPos.z - view.dist * (float)sin(DEG_TO_RAD * view.theta);
	}

	else if(control == ZOOM_P_ID || control == ZOOM_M_ID)//zoom
	{
		if(control == ZOOM_P_ID) view.fovY -= v2;//zoom in
		else view.fovY += v2;//zoom out	
		setCamera();
	}
	else if(control == TUMBLE_P_ID || control == TUMBLE_M_ID)//tumble
	{
		if( control == TUMBLE_P_ID) view.phi += v2;
		else                        view.phi -= v2;
		setCamera();
	}
	else if(control == CRANE_P_ID || control == CRANE_M_ID)//crane
	{
		if( control == CRANE_P_ID) view.theta += v2;
		else                        view.theta -= v2;
		setCamera();
	}
	else if(control == CAMERA_RESET_ID) view = view0;
}

void setupGLUI()
{
	//*******glui***********
	glui = GLUI_Master.create_glui("GLUI-MAIN", 0, win_x0 + win_width + 10, win_y0);

	animePanel = new GLUI_Panel(glui, "Animation" );
	startBtn = new GLUI_Button(animePanel, "Start", START_ID, control_cb);
	freezeBtn = new GLUI_Button(animePanel, "Freeze", FREEZE_ID, control_cb);
	stepBtn = new GLUI_Button(animePanel, "Step", STEP_ID, control_cb);
	resetBtn = new GLUI_Button(animePanel, "Reset", RESET_ID, control_cb);
	timeRate_edit = new GLUI_EditText(animePanel,"timeRate", &timeRate);
	timeRate_edit->set_float_limits( 0.01, 10.0 );

	objectPanel = new GLUI_Panel(glui, "Object" );
	kind_edit = new GLUI_EditText(objectPanel,"kind", &kind0);
	kind_edit->set_int_limits( 0, 2 );
	sizeX_edit = new GLUI_EditText(objectPanel, "sizeX[m]", &sizeX0);
	sizeY_edit = new GLUI_EditText(objectPanel, "sizeY[m]", &sizeY0);
	sizeZ_edit = new GLUI_EditText(objectPanel, "sizeZ[m]", &sizeZ0);
	height_edit = new GLUI_EditText(objectPanel, "height[m]", &height0);
	cInertial_edit = new GLUI_EditText(objectPanel, "cInertial", &cInertial);
	specificG_edit = new GLUI_EditText(objectPanel, "specificG", &specificG0);
	forceY_edit = new GLUI_EditText(objectPanel, "forcrY", &forceY0);

	stroboPanel = new GLUI_Panel(glui, "Strobo" );
	new GLUI_Checkbox(stroboPanel, "Strobo", &flagStrobo, 1);
	numStrobo_edit = new GLUI_EditText(stroboPanel,"numStrobo", &numStrobo);
	numStrobo_edit->set_int_limits( 50, 200 );
	periodStrobo_edit = new GLUI_EditText(stroboPanel,"periodStrobo", &periodStrobo);
	periodStrobo_edit->set_float_limits( 0.01, 1.0 );

	new GLUI_Checkbox(glui, "Wireframe", &flagWireframe, 1, control_cb );
 	new GLUI_Checkbox(glui, "Parameter", &flagParamShow, 1);
 	new GLUI_Checkbox(glui, "Coordinates", &flagCoordShow, 1);

	//光源
	GLUI_Rollout *light = new GLUI_Rollout(glui, "Light", false );
	lightXP_btn = new GLUI_Button(light, "LightX+", LIGHTX_P_ID, control_cb);
	lightXM_btn = new GLUI_Button(light, "LightX-", LIGHTX_M_ID, control_cb);
	lightYP_btn = new GLUI_Button(light, "LightY+", LIGHTY_P_ID, control_cb);
	lightYM_btn = new GLUI_Button(light, "LightY-", LIGHTY_M_ID, control_cb);
	lightZP_btn = new GLUI_Button(light, "LightZ+", LIGHTZ_P_ID, control_cb);
	lightZM_btn = new GLUI_Button(light, "LightZ-", LIGHTZ_M_ID, control_cb);
	resetLight_btn = new GLUI_Button(light, "ResetLight", LIGHT_RESET_ID, control_cb);


	glui->add_column(true);
	//オブジェクト初期設定
	wavePanel = new GLUI_Panel(glui, "Wave" );
	
	size_edit  = new GLUI_EditText(wavePanel, "size[m]", &size0);
	nSlice_edit  = new GLUI_EditText(wavePanel, "nSlice0", &nSlice0);
	nSlice_edit->set_int_limits(10, MAX_SLICE);
	nSource_edit = new GLUI_EditText(wavePanel, "nSource", &nSource0);
	nSource_edit->set_int_limits(0, MAX_SOURCE);
	sourceY_edit  = new GLUI_EditText(wavePanel, "sourceY[m]", &sourceY0);
	interval_edit  = new GLUI_EditText(wavePanel, "interval[m]", &interval0);
	amp_edit = new GLUI_EditText(wavePanel, "amp[m/s]", &amp0);
	freq_edit = new GLUI_EditText(wavePanel, "freq[Hz]", &freq0);
	waveVel_edit = new GLUI_EditText(wavePanel, "waveVel[m/s]", &waveVel0);
	drag_edit = new GLUI_EditText(wavePanel, "drag[1/s]", &drag0);
	coefB_edit = new GLUI_EditText(wavePanel, "coefB", &coefB0);
	coefDown_edit = new GLUI_EditText(wavePanel, "coefDown", &coefDown0);
	coefUp_edit = new GLUI_EditText(wavePanel, "coefUp", &coefUp0);

	glui->add_separator_to_panel(wavePanel);
	radioMode = new GLUI_RadioGroup(wavePanel, &valueMode, 2, control_cb);
	new GLUI_RadioButton(radioMode, "Single");
	new GLUI_RadioButton(radioMode, "Continuous");

	cameraOpe_panel = new GLUI_Panel(glui, "CameraOperation");
	hide_btn = new GLUI_Button(cameraOpe_panel, "Hide", HIDE_ID, control_cb);
	show_btn = new GLUI_Button(cameraOpe_panel, "Show", SHOW_ID, control_cb);
	
	glui->set_main_gfx_window( main_window );

	new GLUI_StaticText( glui, "" );//空白

	//****** 終了 *****
	new GLUI_Button( glui, "Quit", 0,(GLUI_Update_CB)exit );


	//********glui2****************
	glui2 = GLUI_Master.create_glui("GLUI-CAMERA", 0, win_x0 - 50, win_y0 + win_height + 30);
	//カメラ操作
	dollyP_btn = new GLUI_Button(glui2, "Dolly+", DOLLY_P_ID, control_cb);
	dollyM_btn = new GLUI_Button(glui2, "Dolly-", DOLLY_M_ID, control_cb);
	glui2->add_column(false);
	panP_btn = new GLUI_Button(glui2, "Pan+", PAN_P_ID, control_cb);
	panM_btn = new GLUI_Button(glui2, "Pan-", PAN_M_ID, control_cb);
	glui2->add_column(false);
	tiltP_btn = new GLUI_Button(glui2, "Tilt+", TILT_P_ID, control_cb);
	tiltM_btn = new GLUI_Button(glui2, "Tilt-", TILT_M_ID, control_cb);
	glui2->add_column(false);
	tumbleP_btn = new GLUI_Button(glui2, "Tumble+", TUMBLE_P_ID, control_cb);
	tumbleM_btn = new GLUI_Button(glui2, "Tumble-", TUMBLE_M_ID, control_cb);
	glui2->add_column(false);
	craneP_btn = new GLUI_Button(glui2, "Crane+", CRANE_P_ID, control_cb);
	craneM_btn = new GLUI_Button(glui2, "Crane-", CRANE_M_ID, control_cb);
	glui2->add_column(false);
	zoomP_btn = new GLUI_Button(glui2, "Zoom+", ZOOM_P_ID, control_cb);
	zoomM_btn = new GLUI_Button(glui2, "Zoom-", ZOOM_M_ID, control_cb);
	glui2->add_column(false);
	resetCamera_btn = new GLUI_Button(glui2, "ResetCamera", CAMERA_RESET_ID, control_cb);

	glui2->set_main_gfx_window( main_window );

}