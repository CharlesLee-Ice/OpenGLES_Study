//
// Created by liqilin on 2016/4/25.
//

#ifndef NATIVECUBE_CUBERENDERER_H
#define NATIVECUBE_CUBERENDERER_H

#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <android_native_app_glue.h>
#include <android/native_window_jni.h>

#include "JNIHelper.h"

class CubeRenderer {
public:
    CubeRenderer() {}
    ~CubeRenderer() {}

    void init(int32_t width, int32_t heigth);
    void render();

    GLuint createProgram();
    GLuint loadShader(GLenum shaderType, const char* pSource);

    int32_t num_indices;
    int32_t num_vertices;

    GLuint ibo;
    GLuint vbo;

    GLuint gvPositionHandle;
    GLuint gvFactorVertex;

    GLuint program;

    int32_t viewWidth;
    int32_t viewHeigth;
};


#endif //NATIVECUBE_CUBERENDERER_H
