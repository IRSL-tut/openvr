#include <stdio.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <iostream>

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

// g++ -c test_sdl.cpp -O $(pkg-config --cflags sdl2) $(pkg-config --cflags glew) $(pkg-config --cflags opencv4)
// g++ -o test_sdl test_sdl.o $(pkg-config --libs sdl2) $(pkg-config --libs glew) $(pkg-config --libs opencv4)
////

/// OpenVR
#include <openvr.h>
vr::IVRSystem *m_pHMD;
///

SDL_Window *m_pCompanionWindow;
uint32_t m_nCompanionWindowWidth;
uint32_t m_nCompanionWindowHeight;
SDL_GLContext m_pContext;

GLuint m_unCompanionWindowVAO;
GLuint m_glCompanionWindowIDVertBuffer;
GLuint m_glCompanionWindowIDIndexBuffer;
unsigned int m_uiCompanionWindowIndexSize;

GLuint m_unCompanionWindowProgramID;

GLuint m_nResolveTextureId; //
GLuint m_nResolveFramebufferId;//

GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader )
{
    GLuint unProgramID = glCreateProgram();
    GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource( nSceneVertexShader, 1, &pchVertexShader, NULL);
    glCompileShader( nSceneVertexShader );

    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
    if ( vShaderCompiled != GL_TRUE)
    {
        printf("%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);
        glDeleteProgram( unProgramID );
        glDeleteShader( nSceneVertexShader );
        return 0;
    }
    glAttachShader( unProgramID, nSceneVertexShader);
    glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

    GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, NULL);
    glCompileShader( nSceneFragmentShader );

    GLint fShaderCompiled = GL_FALSE;
    glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
    if (fShaderCompiled != GL_TRUE)
    {
        printf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
        glDeleteProgram( unProgramID );
        glDeleteShader( nSceneFragmentShader );
        return 0;
    }

    glAttachShader( unProgramID, nSceneFragmentShader );
    glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

    glLinkProgram( unProgramID );

    GLint programSuccess = GL_TRUE;
    glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
    if ( programSuccess != GL_TRUE )
    {
        printf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
        glDeleteProgram( unProgramID );
        return 0;
    }

    glUseProgram( unProgramID );
    glUseProgram( 0 );

    return unProgramID;
}
struct Vector2
{
    float x;
    float y;

    // ctors
    Vector2() : x(0), y(0) {};
    Vector2(float x, float y) : x(x), y(y) {};
};
struct VertexDataWindow
{
    Vector2 position;
    Vector2 texCoord;
    VertexDataWindow( const Vector2 & pos, const Vector2 tex ) :  position(pos), texCoord(tex) { }
};

void SetupCompanionWindow()
{
    std::vector<VertexDataWindow> vVerts;

    // left eye verts
    vVerts.push_back( VertexDataWindow( Vector2(-1, -1), Vector2(0, 1)) );
    vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(1, 1)) );
    vVerts.push_back( VertexDataWindow( Vector2(-1, 1), Vector2(0, 0)) );
    vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(1, 0)) );

    // right eye verts
    vVerts.push_back( VertexDataWindow( Vector2(0, -1), Vector2(0, 1)) );
    vVerts.push_back( VertexDataWindow( Vector2(1, -1), Vector2(1, 1)) );
    vVerts.push_back( VertexDataWindow( Vector2(0, 1), Vector2(0, 0)) );
    vVerts.push_back( VertexDataWindow( Vector2(1, 1), Vector2(1, 0)) );

    GLushort vIndices[] = { 0, 1, 3,   0, 3, 2,   4, 5, 7,   4, 7, 6};
    m_uiCompanionWindowIndexSize = _countof(vIndices);

    glGenVertexArrays( 1, &m_unCompanionWindowVAO );
    glBindVertexArray( m_unCompanionWindowVAO );

    glGenBuffers( 1, &m_glCompanionWindowIDVertBuffer );
    glBindBuffer( GL_ARRAY_BUFFER, m_glCompanionWindowIDVertBuffer );
    glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataWindow), &vVerts[0], GL_STATIC_DRAW );

    glGenBuffers( 1, &m_glCompanionWindowIDIndexBuffer );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glCompanionWindowIDIndexBuffer );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, m_uiCompanionWindowIndexSize*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, position ) );

    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataWindow), (void *)offsetof( VertexDataWindow, texCoord ) );

    glBindVertexArray( 0 );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void render()
{
  glDisable(GL_DEPTH_TEST);
  glViewport( 0, 0, m_nCompanionWindowWidth, m_nCompanionWindowHeight );

  glBindVertexArray( m_unCompanionWindowVAO );
  glUseProgram( m_unCompanionWindowProgramID );

  // render left eye (first half of index array )
  glBindTexture(GL_TEXTURE_2D, m_nResolveTextureId );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, 0 );

  // render right eye (second half of index array )
  glBindTexture(GL_TEXTURE_2D, m_nResolveTextureId  );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glDrawElements( GL_TRIANGLES, m_uiCompanionWindowIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(uintptr_t)(m_uiCompanionWindowIndexSize) );

  glBindVertexArray( 0 );
  glUseProgram( 0 );
}


int main(int argc, char **argv)
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 ) {
        printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
        return false;
    }
    int nWindowPosX = 700;
    int nWindowPosY = 100;
    Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    uint32_t nWidth, nHeight;

    // >> OpenVR
    // Loading the SteamVR Runtime
    vr::EVRInitError eError = vr::VRInitError_None;
    m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

    if ( eError != vr::VRInitError_None ) {
        m_pHMD = NULL;
        printf( "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
        return 1;
    }

    if ( !vr::VRCompositor() ) {
        printf( "Compositor initialization failed. See log file for details\n" );
        return 2;
    }

    m_pHMD->GetRecommendedRenderTargetSize( &nWidth, &nHeight );
    printf("width x height = %d x %d\n", nWidth, nHeight);

    vr::HmdMatrix44_t l_mat = m_pHMD->GetProjectionMatrix( vr::Eye_Left,  0.01f, 15.0f );
    vr::HmdMatrix44_t r_mat = m_pHMD->GetProjectionMatrix( vr::Eye_Right, 0.01f, 15.0f );
    vr::HmdMatrix34_t l_eye = m_pHMD->GetEyeToHeadTransform( vr::Eye_Left );
    vr::HmdMatrix34_t r_eye = m_pHMD->GetEyeToHeadTransform( vr::Eye_Right );

    std::cout << "projection(left)" << std::endl;
    std::cout << l_mat.m[0][0] << ", " << l_mat.m[1][0] << ", " <<  l_mat.m[2][0] << ", " << l_mat.m[3][0] << std::endl;
    std::cout << l_mat.m[0][1] << ", " << l_mat.m[1][1] << ", " <<  l_mat.m[2][1] << ", " << l_mat.m[3][1] << std::endl;
    std::cout << l_mat.m[0][2] << ", " << l_mat.m[1][2] << ", " <<  l_mat.m[2][2] << ", " << l_mat.m[3][2] << std::endl;
    std::cout << l_mat.m[0][3] << ", " << l_mat.m[1][3] << ", " <<  l_mat.m[2][3] << ", " << l_mat.m[3][3] << std::endl;
    std::cout << "projection(right)" << std::endl;
    std::cout << r_mat.m[0][0] << ", " << r_mat.m[1][0] << ", " <<  r_mat.m[2][0] << ", " << r_mat.m[3][0] << std::endl;
    std::cout << r_mat.m[0][1] << ", " << r_mat.m[1][1] << ", " <<  r_mat.m[2][1] << ", " << r_mat.m[3][1] << std::endl;
    std::cout << r_mat.m[0][2] << ", " << r_mat.m[1][2] << ", " <<  r_mat.m[2][2] << ", " << r_mat.m[3][2] << std::endl;
    std::cout << r_mat.m[0][3] << ", " << r_mat.m[1][3] << ", " <<  r_mat.m[2][3] << ", " << r_mat.m[3][3] << std::endl;
    std::cout << "eye(left)" << std::endl;
    std::cout << l_eye.m[0][0] << ", " << l_eye.m[1][0] << ", " <<  l_eye.m[2][0] << ", 0.0" << std::endl;
    std::cout << l_eye.m[0][1] << ", " << l_eye.m[1][1] << ", " <<  l_eye.m[2][1] << ", 0.0" << std::endl;
    std::cout << l_eye.m[0][2] << ", " << l_eye.m[1][2] << ", " <<  l_eye.m[2][2] << ", 0.0" << std::endl;
    std::cout << l_eye.m[0][3] << ", " << l_eye.m[1][3] << ", " <<  l_eye.m[2][3] << ", 1.0" << std::endl;
    std::cout << "eye(right)" << std::endl;
    std::cout << r_eye.m[0][0] << ", " << r_eye.m[1][0] << ", " <<  r_eye.m[2][0] << ", 0.0" << std::endl;
    std::cout << r_eye.m[0][1] << ", " << r_eye.m[1][1] << ", " <<  r_eye.m[2][1] << ", 0.0" << std::endl;
    std::cout << r_eye.m[0][2] << ", " << r_eye.m[1][2] << ", " <<  r_eye.m[2][2] << ", 0.0" << std::endl;
    std::cout << r_eye.m[0][3] << ", " << r_eye.m[1][3] << ", " <<  r_eye.m[2][3] << ", 1.0" << std::endl;
    // << OpenVR

    //m_nCompanionWindowWidth = nWidth;
    //m_nCompanionWindowHeight = nHeight;
    m_nCompanionWindowWidth = 1200;
    m_nCompanionWindowHeight = 800;

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );

    m_pCompanionWindow = SDL_CreateWindow( "hellovr", nWindowPosX, nWindowPosY, m_nCompanionWindowWidth, m_nCompanionWindowHeight, unWindowFlags );
    if (m_pCompanionWindow == NULL)
    {
        printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    m_pContext = SDL_GL_CreateContext(m_pCompanionWindow);
    if (m_pContext == NULL)
    {
        printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    glewExperimental = GL_TRUE;
    GLenum nGlewError = glewInit();
    if (nGlewError != GLEW_OK)
    {
        printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
        return false;
    }
    glGetError(); // to clear the error caused deep in GLEW

    bool m_bVblank = true;
    if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 )
    {
        printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }

    SDL_SetWindowTitle( m_pCompanionWindow, "HOGE" );

    m_unCompanionWindowProgramID = CompileGLShader(
        "CompanionWindow",
        // vertex shader
        "#version 410 core\n"
        "layout(location = 0) in vec4 position;\n"
        "layout(location = 1) in vec2 v2UVIn;\n"
        "noperspective out vec2 v2UV;\n"
        "void main()\n"
        "{\n"
        "	v2UV = v2UVIn;\n"
        "	gl_Position = position;\n"
        "}\n",
        // fragment shader
        "#version 410 core\n"
        "uniform sampler2D mytexture;\n"
        "noperspective in vec2 v2UV;\n"
        "out vec4 outputColor;\n"
        "void main()\n"
        "{\n"
        "		outputColor = texture(mytexture, v2UV);\n"
        "}\n"
        );

    // frame buffer
    // create frame buffer
    glGenFramebuffers(1, &m_nResolveFramebufferId );
    glBindFramebuffer(GL_FRAMEBUFFER, m_nResolveFramebufferId);

    glGenTextures(1, &m_nResolveTextureId );
    glBindTexture(GL_TEXTURE_2D, m_nResolveTextureId );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nResolveTextureId, 0);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        return false;
    }
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    // frame buffer

    // copy frame buffer
    SetupCompanionWindow();

    long cntr = 0;
    while(true) {
        int gl_w = nWidth;
        int gl_h = nHeight;

        unsigned char red = cntr++ % 0xFF;
        std::vector<unsigned char> img(nWidth * nHeight * 3);
        unsigned char *ptr = img.data();

        for(int i = 0; i < nHeight; i++) {
            for(int j = 0; j < nWidth; j++) {
                ptr[3*(i*nWidth + j)] = red;
                ptr[3*(i*nWidth + j)+1] = 0;
                ptr[3*(i*nWidth + j)+2] = 0;
            }
        }
        glBindTexture( GL_TEXTURE_2D, m_nResolveTextureId );
        glGetTexLevelParameteriv( GL_TEXTURE_2D , 0 , GL_TEXTURE_WIDTH , &gl_w );
        glGetTexLevelParameteriv( GL_TEXTURE_2D , 0 , GL_TEXTURE_HEIGHT, &gl_h );
        glTexSubImage2D( GL_TEXTURE_2D, 0,
                         gl_w/2 - nWidth/2, gl_h/2 - nHeight/2,
                         nWidth, nHeight,
                         GL_RGB, GL_UNSIGNED_BYTE, ptr );
        glBindTexture( GL_TEXTURE_2D, 0 );

        glFinish();

        SDL_GL_SwapWindow( m_pCompanionWindow );
        render();

        //glClearColor( 0, 0, 0, 1 );
        //glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        // >> OpenVR
        vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
        // << OpenVR

        glFlush();
        glFinish();
    }

    return 0;
}
