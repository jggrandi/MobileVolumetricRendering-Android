/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
//#include "esUtils/esUtil.h"
#include "PinholeCamera.h"
#include "LookUpTable.h"
#include "wsg.h"

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

PinholeCamera m_camera;
ESMatrix viewMatrix;
ESMatrix modelView;
ESMatrix modelViewInverse;
ESMatrix projection;
ESMatrix mvp;
GLfloat *volumeVertices;
GLuint textureLookUp;
GLuint textureId;
GLuint uniformLookUp;
GLuint uniformId;

float timeCounter, prevTime = 0.0, FPS;
int			frame = 1, framesPerFPS;
struct timeval tv;
struct timeval tv0;


void InitTimeCounter() {
 gettimeofday(&tv0, NULL);
 framesPerFPS = 1; }

void UpdateTimeCounter() {
 gettimeofday(&tv, NULL);
 timeCounter = (float)(tv.tv_sec-tv0.tv_sec) + 0.000001*((float)(tv.tv_usec-tv0.tv_usec));
}

void CalculateFPS() {
 frame ++;
 if((frame%framesPerFPS) == 0)
 {
	FPS = (timeCounter-prevTime);
	prevTime = timeCounter;
 }
}

short minV = 32768;
short maxV = -32768;

GLuint loadRAWTexture(int inWidth, int inHeight, int inDepth) {

short **imageData;
FILE* inFile;
int width = inWidth;
int height = inHeight;
int depth = inDepth;

// allocate memory for the image matrix
imageData = (short**)malloc((depth) * sizeof(short*));

for (int i=0; i < depth; i++)
	imageData[i] = (short*)malloc(sizeof(short) * width * height);

int widthxheight = width*height;
//inFileName
if( inFile = fopen("sdcard/body01.raw", "r" )  ){

	// read file into image matrix
	for( int i = 0; i < depth; i++ ){
		for( int j = 0; j < widthxheight; j++ ){
			short value;
			fread( &value, 1, sizeof(short), inFile );
			imageData[i][j] = value;
			//LOG("short data:%d",value);
		}
	}
	fclose(inFile);
	LOGE("+++ Volume successfully loaded ");
}else
{
	LOGE("Error when loading volume");
	abort();
}

float *textureData;
int supportedSize;
glGetIntegerv(GL_MAX_TEXTURE_SIZE, &supportedSize);
//	short *textureData;
int texWidth = supportedSize;
int texHeight = supportedSize;

//	int texWidth = 512;
//	int texHeight = 512;


// allocate memory for the image matrix
textureData = (float*)malloc(sizeof(float) * texWidth * texHeight);
//	textureData = (short*)malloc(sizeof(short) * texWidth * texHeight);
float minValue = -1024.0;
float maxValue = 3071.0;

for (int i=0; i< texHeight * texWidth ; i++){
	textureData[i]=0.0f;
}

int octaveSupSize = supportedSize/8;
int stepSize = (width*8)/supportedSize;
for( int i = 0; i < depth; i++ ){
	for( int j = 0; j < widthxheight; j+=stepSize ){
		//int j2 = j/2;
		//int coord = ((i/8)*256+((j/512)/2))*2048 + (i%8)*256 + ((j%512)/2);
		int coord = ((i/8)*octaveSupSize+(j/width)/stepSize)*supportedSize + (i%8)*octaveSupSize + ((j%width)/stepSize);
		textureData[coord] = ((float)imageData[i][j] - minValue)/(maxValue-minValue);
		if (textureData[coord] < 0.0f)
			textureData[coord] = 0.0f;
		if (textureData[coord] > 1.0f )
			textureData[coord] = 1.0f;
	}
}


//glEnable(GL_TEXTURE_2D);
//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
GLuint textureID;
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_2D, textureID);

//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, texWidth, texHeight, 0, GL_LUMINANCE, GL_FLOAT, textureData);

glGenerateMipmap(GL_TEXTURE_2D);
//glDisable(GL_TEXTURE_2D);
return textureID;
}

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

/*
	static const char gVertexShader[] =
    "attribute vec3 vertexPos;\n"
    "attribute vec3 texcoord;\n"
    "uniform mat4 mvp;\n"
    "varying mediump vec3 v_texcoord;\n"
    "void main() {\n"
    "   gl_Position = mvp * vec4(vertexPos,1.0);\n"
    "   v_texcoord = texcoord;\n"
    //"  gl_Position = vPosition;\n"
    "}\n";
*/


static const char gVertexShader[] =
    "attribute vec3 vertexPos;\n"
    "attribute vec3 texCoord;\n"
	"uniform mat4 modelViewInverse;\n"
    "uniform mat4 mvp;\n"
	"uniform float sliceDistance;\n"
    "varying mediump vec3 v_texcoord;\n"
	"varying mediump vec4 v_texcoord2;\n"
    "void main() {\n"
    "   gl_Position = mvp * vec4(vertexPos,1.0);\n"
    "   v_texcoord = texCoord;\n"
	"	vec4 vPosition = vec4(0.0, 0.0, 0.0, 1.0);\n"
	"	vPosition = modelViewInverse*vPosition;\n"
	"	vec4 vDir = vec4(0.0, 0.0, -1.0, 1.0);\n"
	"	vDir = normalize(modelViewInverse*vDir);\n"
	"	vec4 eyeToVert = normalize(vec4(vertexPos,1.0) - vPosition);\n"
	"   float t = sliceDistance/dot(vDir, eyeToVert);\n"
	"   eyeToVert = eyeToVert*t;\n"
	"	vec4 sB = vec4(texCoord,1.0) - eyeToVert;\n"
	"   v_texcoord2 = sB;\n"
    "}\n";
/*
static const char gFragmentShader[] =
    // Low Quality Fragment Shader
	"uniform sampler2D volumeSampler;\n"
	"uniform sampler2D lookupTableSampler;\n"
	"varying mediump vec3 v_texcoord;\n"
	"varying mediump vec4 v_texcoord2;\n"
    "void main() {\n"
    "   mediump vec2 texcoord;\n"
    "   texcoord.x = v_texcoord.x*0.125;\n"
    "   texcoord.y = v_texcoord.y*0.125;\n"
    "   int slice = int(v_texcoord.z);\n"
    "   int line = int(slice/8);\n"
    "   int row = slice-line*8;\n"
    "   texcoord.x = texcoord.x + float(row)*0.125;\n"
    "   texcoord.y = texcoord.y + float(line)*0.125;\n"
    "   mediump vec2 texCol;\n"
    "   texCol.x = texture2D(volumeSampler,texcoord.xy).x;\n"
    "   texCol.y = texCol.x;\n"
    "   mediump vec4 lookup = texture2D(lookupTableSampler,texCol.xy);\n"
    "   gl_FragColor = lookup;\n"
    "}\n";
*/

/*
static const char gFragmentShader[] =
	// Medium Quality Fragment Shader
	"uniform sampler2D volumeSampler;\n"
	"uniform sampler2D lookupTableSampler;\n"
	"varying mediump vec3 v_texcoord;\n"
	"varying mediump vec4 v_texcoord2;\n"
	"void main()\n"
	"{\n"
	// code for a 8x8 slice img pre-integration classification
	"mediump vec2 texcoord;\n"
	"texcoord.x = v_texcoord.x*0.125;\n"
	"texcoord.y = v_texcoord.y*0.125;\n"
	"int slice = int(v_texcoord.z);\n"
	"int line = int(slice/8);\n"
	"int row = slice-line*8;\n"
	"texcoord.x = texcoord.x + float(row)*0.125;\n"
	"texcoord.y = texcoord.y + float(line)*0.125;\n"
	"mediump vec2 texcoord2;\n"
	"texcoord2.x = v_texcoord2.x*0.125;\n"
	"texcoord2.y = v_texcoord2.y*0.125;\n"
	"slice = int(v_texcoord2.z);\n"
	"line = int(slice/8);\n"
	"row = slice-line*8;\n"
	"texcoord2.x = texcoord2.x + float(row)*0.125;\n"
	"texcoord2.y = texcoord2.y + float(line)*0.125;\n"
	// code with pre-integration table and no filtering in z
	"mediump vec2 texCol;\n"
	"texCol.x = texture2D(volumeSampler,texcoord.xy).x;\n"
	"texCol.y = texture2D(volumeSampler,texcoord2.xy).x;\n"
	"mediump vec4 lookup = texture2D(lookupTableSampler,texCol.xy);\n"
	"gl_FragColor = lookup;\n"
	"}\n";
*/



static const char gFragmentShader[] =
	"uniform sampler2D volumeSampler;\n"
	"uniform sampler2D lookupTableSampler;\n"
	"varying mediump vec3 v_texcoord;\n"
	"varying mediump vec4 v_texcoord2;\n"
	"void main(){\n"
	"mediump vec2 texcoord3;\n"
	"mediump vec2 texcoord4;\n"
	"mediump vec2 texcoord;\n"
	"texcoord.x = v_texcoord.x*0.125;\n"
	"texcoord.y = v_texcoord.y*0.125;\n"
    "int slice = int(v_texcoord.z);\n"
	"int line = int(slice/8);\n"
	"int row = slice-line*8;\n"
	"texcoord.x = texcoord.x + float(row)*0.125;\n"
	"texcoord.y = texcoord.y + float(line)*0.125;\n"


	"mediump float weight1;\n"
	"if (float(slice)>v_texcoord.z){\n"
	"	weight1 = float(slice)-v_texcoord.z;\n"
	"	slice -=1;\n"
	"}\n"
	"else {\n"
	"	weight1 = v_texcoord.z-float(slice);\n"
	"	slice+=1;\n"
	"}\n"
	"line = int(slice/8);\n"
	"row = slice-line*8;\n"
	"texcoord3.x = v_texcoord.x*0.125 + float(row)*0.125;\n"
	"texcoord3.y = v_texcoord.y*0.125 + float(line)*0.125;\n"


	"mediump vec2 texcoord2;\n"
	"texcoord2.x = v_texcoord2.x*0.125;\n"
	"texcoord2.y = v_texcoord2.y*0.125;\n"
	"slice = int(v_texcoord2.z);\n"
	"line = int(slice/8);\n"
	"row = slice-line*8;\n"
	"texcoord2.x = texcoord2.x + float(row)*0.125;\n"
	"texcoord2.y = texcoord2.y + float(line)*0.125;\n"

	"mediump float weight2;\n"
	"if (float(slice)>v_texcoord2.z){\n"
	"	weight2 = float(slice)-v_texcoord2.z;\n"
	"	slice -=1;\n"
	"}\n"
	"else {\n"
	"	weight2 = v_texcoord2.z-float(slice);\n"
	"	slice+=1;\n"
	"}\n"
	"line = int(slice/8);\n"
	"row = slice-line*8;\n"
	"texcoord4.x = v_texcoord2.x*0.125 + float(row)*0.125;\n"
	"texcoord4.y = v_texcoord2.y*0.125 + float(line)*0.125;\n"

	// code with pre-integration table and filtering on Z\n"

	"mediump vec2 texCol;\n"
	"texCol.x = texture2D(volumeSampler,texcoord.xy).x;\n"
	"texCol.y = texture2D(volumeSampler,texcoord2.xy).x;\n"
	"mediump vec2 texCol2;\n"
	"texCol2.x = texture2D(volumeSampler,texcoord3.xy).x;\n"
	"texCol2.y = texture2D(volumeSampler,texcoord4.xy).x;\n"

	"mediump vec2 lerpTex; \n"
	"lerpTex.x = mix(texCol.x,texCol2.x,weight1);\n"
	"lerpTex.y = mix(texCol.y,texCol2.y,weight2);\n"

	"mediump vec4 lookup = texture2D(lookupTableSampler,lerpTex.xy);\n"

	"gl_FragColor = lookup;\n"
	"}\n";


GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
		LOGE("Vertex Error");
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
		LOGE("Fragment Error");
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionHandle;

bool setupGraphics(int w, int h/*, const char* apkPath*/) {

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	checkGlError("glClearColor");
	printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);


    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);

	vertexHandle        = glGetAttribLocation(gProgram,"vertexPos");
    textureCoordHandle  = glGetAttribLocation(gProgram,"texCoord");
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    m_camera.Create(45.0, 1.0, 600.0, w, h);

    m_camera.MoveFront(0.0);
    projection = m_camera.GetProjectionMatrix();

    glViewport(0, 0, 320, 480);

    InitializeLookUpTable(m_windowCenter, m_windowWidth);
    textureLookUp=createPreintegrationTable (m_lookUpTable);
    textureId=loadRAWTexture(512,512,43);

    uniformLookUp = glGetUniformLocation(gProgram, "lookupTableSampler");
    uniformId = glGetUniformLocation(gProgram, "volumeSampler");

    checkGlError("glViewport");

    InitTimeCounter();

    return true;
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f};

float fpsOld;
const int k=20;
int values[k];
int iii=0;


void renderFrame() {

	UpdateTimeCounter();

	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	checkGlError("glClear");

	glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);

    //m_camera.Yaw(5.0);
    //m_camera.Pitch(5.0);
    viewMatrix = m_camera.ApplyTransform();
    // reset our modelview to identity
    esMatrixLoadIdentity(&modelView);
    // translate back into the screen

    esTranslate(&modelView, 0.05f , 0.0f, -2.5f);
    //esTranslate(&modelView, 0.05f , 0.0f, -4.0f);
	esMatrixMultiply(&modelView,&viewMatrix,&modelView);
    esRotate(&modelView, 180, 0, 1, 0);
    esInverseMatrix(&modelViewInverse, &modelView);
	esMatrixMultiply(&mvp, &modelView, &projection );

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glUniformMatrix4fv(glGetUniformLocation(gProgram, "mvp"), 1, GL_FALSE, (GLfloat*) &mvp.m[0][0] );
	glUniformMatrix4fv(glGetUniformLocation(gProgram, "modelViewInverse"), 1, GL_FALSE, (GLfloat*)&modelViewInverse.m[0][0]);
	glUniform1fv(glGetUniformLocation(gProgram, "sliceDistance"), 1, (GLfloat*) &SPACING );


    checkGlError("glUniformMatrix");


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(uniformId, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureLookUp);
    glUniform1i(uniformLookUp, 1);

    float eyeVector[3];
    eyeVector[0] = -modelView.m[0][2];
    eyeVector[1] = -modelView.m[1][2];
    eyeVector[2] = -modelView.m[2][2];
    //normalize
    float eyeVectorMagnitude = sqrt((eyeVector[0] * eyeVector[0]) + (eyeVector[1] * eyeVector[1]) + (eyeVector[2] * eyeVector[2]));
    eyeVector[0] = eyeVector[0]/eyeVectorMagnitude;
    eyeVector[1] = eyeVector[1]/eyeVectorMagnitude;
    eyeVector[2] = eyeVector[2]/eyeVectorMagnitude;
    //plot from wsg
    plot(eyeVector, &volumeVertices);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	CalculateFPS();

	values[iii] = FPS*1000;
	iii++;
	if(iii==k)
	{
		iii=0;
		float result;
		for(int i=0;i<k;i++)
		{
			result = result+values[i];
		}
		result=result/k;
		LOGE("TimePerFrame:%f",result);
	}


}

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_step(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height/*, jstring apkPath*/)
{
/*    (JNIEnv * env, jclass cls, jstring apkPath) {
    const char* str;
    jboolean isCopy;
    str = env->GetStringUTFChars(apkPath, &isCopy);*/
    //loadAPK(str);
    setupGraphics(width, height/*, str*/);
}

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_step(JNIEnv * env, jobject obj)
{
    renderFrame();
}
