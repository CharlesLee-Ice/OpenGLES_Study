//
// Created by liqilin on 2016/4/25.
//

#include "CubeRenderer.h"

static const char gVertexShader[] = 
    "attribute highp vec3 myVertex;\n"
    "uniform highp vec3 facVertex;\n"
    "uniform highp mat4 uPMatrix;\n"
    "void main() {\n"
    "  gl_Position = vec4(facVertex, 1) * vec4(myVertex,1);\n"
    "}\n";

static const char gFragmentShader[] = 
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

static const float g_vertexs[] = {-0.5f, -0.5f, -0.5f,
                                 0.5f, -0.5f, -0.5f,
                                 0.5f,  0.5f, -0.5f,
                                -0.5f,  0.5f, -0.5f,
                                -0.5f, -0.5f,  0.5f,
                                 0.5f, -0.5f,  0.5f,
                                 0.5f,  0.5f,  0.5f,
                                -0.5f,  0.5f,  0.5f,};

static const short g_indices[] =  { 0, 4, 5,
                                    0, 5, 1,
                                    1, 5, 6,
                                    1, 6, 2,
                                    2, 6, 7,
                                    2, 7, 3,
                                    3, 7, 4,
                                    3, 4, 0,
                                    4, 7, 6,
                                    4, 6, 5,
                                    3, 0, 1,
                                    3, 1, 2, };

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

void CubeRenderer::init(int32_t width, int32_t heigth)
{
    viewWidth = width;
    viewHeigth = heigth;

    glFrontFace(GL_CCW);

    createProgram();

    gvPositionHandle = glGetAttribLocation(program, "myVertex");
    gvFactorVertex = glGetUniformLocation(program, "facVertex");
    gmFactorMatrix = glGetUniformLocation(program, "uPMatrix");

    num_vertices = sizeof(g_vertexs)/sizeof(g_vertexs[0])/3;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertexs), g_vertexs, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    num_indices = sizeof(g_indices)/sizeof(g_indices[0]);
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_indices), g_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool CubeRenderer::Bind( ndk_helper::TapCamera* camera )
{
    camera_ = camera;
    return true;
}

void CubeRenderer::Update( float fTime )
{
    const float CAM_X = 0.f;
    const float CAM_Y = 0.f;
    const float CAM_Z = 0.f;

    mat_view_ = ndk_helper::Mat4::LookAt( ndk_helper::Vec3( CAM_X, CAM_Y, CAM_Z ),
            ndk_helper::Vec3( 0.f, 0.f, 0.f ), ndk_helper::Vec3( 0.f, 1.f, 0.f ) );

    if( camera_ )
    {
        camera_->Update();
        mat_view_ = camera_->GetRotationMatrix();
    }
}

void CubeRenderer::render()
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(gvPositionHandle);

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo);
    
    glUseProgram(program);

    glUniform3f(gvFactorVertex, 1.0f, (float)viewWidth / (float)viewHeigth, 1.0f);
    glUniformMatrix4fv( gmFactorMatrix, 1, GL_FALSE, mat_view_.Ptr() );
    mat_view_.Dump();

    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, 0);

    //glBindBuffer( GL_ARRAY_BUFFER, 0 );
    //glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

GLuint CubeRenderer::createProgram()
{
    GLuint vert_shader, frag_shader;

    /*Create Vertexs Shader*/
    vert_shader = loadShader(GL_VERTEX_SHADER, gVertexShader);
    if (!vert_shader) {
        return 0;
    }

    frag_shader = loadShader(GL_FRAGMENT_SHADER, gFragmentShader);
    if (!frag_shader) {
        return 0;
    }

    program = glCreateProgram();

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);

    glLinkProgram(program);

    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLint bufLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLen);
        if (bufLen > 0) {
            char* buf = (char*)malloc(bufLen);
            if (buf) {
                glGetProgramInfoLog(program, bufLen, NULL, buf);
                LOGE("Could not link program %s", buf);
                free(buf);
            }
        }
        glDeleteProgram(program);
        program = 0;
    }

    return program;
}

GLuint CubeRenderer::loadShader(GLenum shaderType, const char* pSource)
{
   GLuint shader = glCreateShader(shaderType);
   if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);

        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 0) {
                char* buf = (char*)malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader type: %d %s", shaderType, buf);
                    free(buf);
                }
            }
            glDeleteShader(shader);
            shader = 0;
        }
   }
   return shader;
}
