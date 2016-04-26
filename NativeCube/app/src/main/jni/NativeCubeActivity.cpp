#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android_native_app_glue.h>
#include <android/native_window_jni.h>

#include "gestureDetector.h"  //Tap/Doubletap/Pinch detector

#include "CubeRenderer.h"


class Engine {
public:
    Engine() {}
    ~Engine() {}

    void initDisplay();
    void drawFrame();
    void termDisplay();

    void initEGL();

    struct android_app* app;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    CubeRenderer renderer;

    ndk_helper::DragDetector drag_detector_;
};

Engine g_engine;

void Engine::initDisplay()
{
    LOGE("initDisplay~~~");
    
    initEGL();

    renderer.init(width, height);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glViewport(0, 0, width, height);
}

void Engine::drawFrame()
{
    LOGE("drawFrame~~~");

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer.render();

    bool b = eglSwapBuffers(display, surface);
    if (!b) {
        EGLint err = eglGetError();
        LOGE("Failed to swap buffers error: %d", err);
    }
}

void Engine::termDisplay()
{
    LOGE("termDisplay~~~");
}

void Engine::initEGL()
{
    EGLint format;
    EGLint numConfigs;
    EGLConfig config;

    const EGLint attribs[] = {EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_NONE};

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(g_engine.app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, g_engine.app->window, NULL);

    const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, //Request opengl ES2.0
            EGL_NONE };
    context = eglCreateContext(display, config, NULL, context_attribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("Unable to eglMakeCurrent");
        return;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    const char* versionStr = (const char*) glGetString( GL_VERSION );
    LOGE("GL version: %s", versionStr);
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    switch(cmd) {
    case APP_CMD_INIT_WINDOW:
        if (app->window != NULL) {
            g_engine.initDisplay();
            g_engine.drawFrame();
        }
        break;
    case APP_CMD_TERM_WINDOW:
        g_engine.termDisplay();
        break;
    }
}

static int32_t engine_handle_input(struct android_app* app, AInputEvent* event)
{
    Engine* eng = (Engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        ndk_helper::GESTURE_STATE dragState = eng->drag_detector_.Detect(event);

        // Handle drag state
        if (dragState & ndk_helper::GESTURE_STATE_START) {
            // Otherwise, start dragging
            ndk_helper::Vec2 v;
            eng->drag_detector_.GetPointer(v);
            eng->TransformPosition(v);
            eng->tap_camera_.BeginDrag(v);
        } else if (dragState & ndk_helper::GESTURE_STATE_MOVE) {
            ndk_helper::Vec2 v;
            eng->drag_detector_.GetPointer(v);
            eng->TransformPosition(v);
            eng->tap_camera_.Drag(v);
        } else if (dragState & ndk_helper::GESTURE_STATE_END) {
            eng->tap_camera_.EndDrag();
        }
        return 1;
    }
    return 0;
}

void android_main(struct android_app* state) {

    state->userData = &g_engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    g_engine.app = state;

    while(1) {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident = ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) {
            if (source != NULL) {
                source->process(state, source);
            }
        }
    }

}