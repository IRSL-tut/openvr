//========= Copyright Valve Corporation ============//

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#if defined( OSX )
#include <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#include <OpenGL/glu.h>
// Apple's version of glut.h #undef's APIENTRY, redefine it
#define APIENTRY
#else
#include <GL/glu.h>
#endif
#include <stdio.h>
#include <string>
#include <cstdlib>
#include <vector>

#include <openvr.h>

//#include "shared/lodepng.h"
#include "shared/Matrices.h"
#include "shared/pathtools.h"

#if defined(POSIX)
#include "unistd.h"
#endif

#ifndef _WIN32
#define APIENTRY
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof((x)[0]))
#endif

void ThreadSleep( unsigned long nMilliseconds )
{
#if defined(_WIN32)
    ::Sleep( nMilliseconds );
#elif defined(POSIX)
    usleep( nMilliseconds * 1000 );
#endif
}

static bool g_bPrintf = true;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
    CMainApplication( int argc, char *argv[] );
    virtual ~CMainApplication();

    bool BInit();
    bool BInitGL();
    bool BInitCompositor();

    void Shutdown();

    void RunMainLoop();
    //bool HandleInput();
    //void ProcessVREvent( const vr::VREvent_t & event );
    void RenderFrame();

    //bool SetupTexturemaps();

    //void SetupScene();
    //void AddCubeToScene( Matrix4 mat, std::vector<float> &vertdata );
    //void AddCubeVertex( float fl0, float fl1, float fl2, float fl3, float fl4, std::vector<float> &vertdata );

    //void RenderControllerAxes();

    bool SetupStereoRenderTargets();
    void SetupCompanionWindow();
    //void SetupCameras();

    //void RenderStereoTargets();
    //void RenderCompanionWindow();
    //void RenderScene( vr::Hmd_Eye nEye );

    Matrix4 GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye );
    Matrix4 GetHMDMatrixPoseEye( vr::Hmd_Eye nEye );
    Matrix4 GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye );
    void UpdateHMDMatrixPose();

    Matrix4 ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose );

    //GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader );
    //bool CreateAllShaders();

    //CGLRenderModel *FindOrLoadRenderModel( const char *pchRenderModelName );

private: 
    bool m_bDebugOpenGL;
    bool m_bVerbose;
    bool m_bPerf;
    bool m_bVblank;
    bool m_bGlFinishHack;

    vr::IVRSystem *m_pHMD;
    std::string m_strDriver;
    std::string m_strDisplay;
    vr::TrackedDevicePose_t m_rTrackedDevicePose[ vr::k_unMaxTrackedDeviceCount ];
    Matrix4 m_rmat4DevicePose[ vr::k_unMaxTrackedDeviceCount ];

    enum EHand
    {
        Left = 0,
        Right = 1,
    };

private: // SDL bookkeeping
    SDL_Window *m_pCompanionWindow;
    uint32_t m_nCompanionWindowWidth;
    uint32_t m_nCompanionWindowHeight;

    SDL_GLContext m_pContext;

private: // OpenGL bookkeeping
    int m_iTrackedControllerCount;
    int m_iTrackedControllerCount_Last;
    int m_iValidPoseCount;
    int m_iValidPoseCount_Last;
    bool m_bShowCubes;
    Vector2 m_vAnalogValue;

    std::string m_strPoseClasses;                            // what classes we saw poses for this frame
    char m_rDevClassChar[ vr::k_unMaxTrackedDeviceCount ];   // for each device, a character representing its class

    int m_iSceneVolumeWidth;
    int m_iSceneVolumeHeight;
    int m_iSceneVolumeDepth;
    float m_fScaleSpacing;
    float m_fScale;
    
    int m_iSceneVolumeInit;                                  // if you want something other than the default 20x20x20
    
    float m_fNearClip;
    float m_fFarClip;

    GLuint m_iTexture;

    unsigned int m_uiVertcount;

    GLuint m_glSceneVertBuffer;
    GLuint m_unSceneVAO;
    GLuint m_unCompanionWindowVAO;
    GLuint m_glCompanionWindowIDVertBuffer;
    GLuint m_glCompanionWindowIDIndexBuffer;
    unsigned int m_uiCompanionWindowIndexSize;

    GLuint m_glControllerVertBuffer;
    GLuint m_unControllerVAO;
    unsigned int m_uiControllerVertcount;

    Matrix4 m_mat4HMDPose;
    Matrix4 m_mat4eyePosLeft;
    Matrix4 m_mat4eyePosRight;

    Matrix4 m_mat4ProjectionCenter;
    Matrix4 m_mat4ProjectionLeft;
    Matrix4 m_mat4ProjectionRight;

    struct VertexDataScene
    {
        Vector3 position;
        Vector2 texCoord;
    };

    struct VertexDataWindow
    {
        Vector2 position;
        Vector2 texCoord;

        VertexDataWindow( const Vector2 & pos, const Vector2 tex ) :  position(pos), texCoord(tex) {    }
    };

    GLuint m_unSceneProgramID;
    GLuint m_unCompanionWindowProgramID;
    GLuint m_unControllerTransformProgramID;
    GLuint m_unRenderModelProgramID;

    GLint m_nSceneMatrixLocation;
    GLint m_nControllerMatrixLocation;
    GLint m_nRenderModelMatrixLocation;

    struct FramebufferDesc
    {
        GLuint m_nDepthBufferId;
        GLuint m_nRenderTextureId;
        GLuint m_nRenderFramebufferId;
        GLuint m_nResolveTextureId;
        GLuint m_nResolveFramebufferId;
    };
    FramebufferDesc leftEyeDesc;
    FramebufferDesc rightEyeDesc;

    bool CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc );
    
    uint32_t m_nRenderWidth;
    uint32_t m_nRenderHeight;

    //std::vector< CGLRenderModel * > m_vecRenderModels;

    vr::VRActionHandle_t m_actionHideCubes = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionHideThisController = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionTriggerHaptic = vr::k_ulInvalidActionHandle;
    vr::VRActionHandle_t m_actionAnalongInput = vr::k_ulInvalidActionHandle;

    vr::VRActionSetHandle_t m_actionsetDemo = vr::k_ulInvalidActionSetHandle;
};

//-----------------------------------------------------------------------------
// Purpose: Outputs a set of optional arguments to debugging output, using
//          the printf format setting specified in fmt*.
//-----------------------------------------------------------------------------
void dprintf( const char *fmt, ... )
{
    va_list args;
    char buffer[ 2048 ];

    va_start( args, fmt );
    vsprintf_s( buffer, fmt, args );
    va_end( args );

    if ( g_bPrintf )
        printf( "%s", buffer );

    OutputDebugStringA( buffer );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication( int argc, char *argv[] )
    : m_pCompanionWindow(NULL)
    , m_pContext(NULL)
    , m_nCompanionWindowWidth( 640 )
    , m_nCompanionWindowHeight( 320 )
    , m_unSceneProgramID( 0 )
    , m_unCompanionWindowProgramID( 0 )
    , m_unControllerTransformProgramID( 0 )
    , m_unRenderModelProgramID( 0 )
    , m_pHMD( NULL )
    , m_bDebugOpenGL( false )
    , m_bVerbose( false )
    , m_bPerf( false )
    , m_bVblank( false )
    , m_bGlFinishHack( true )
    , m_glControllerVertBuffer( 0 )
    , m_unControllerVAO( 0 )
    , m_unSceneVAO( 0 )
    , m_nSceneMatrixLocation( -1 )
    , m_nControllerMatrixLocation( -1 )
    , m_nRenderModelMatrixLocation( -1 )
    , m_iTrackedControllerCount( 0 )
    , m_iTrackedControllerCount_Last( -1 )
    , m_iValidPoseCount( 0 )
    , m_iValidPoseCount_Last( -1 )
    , m_iSceneVolumeInit( 20 )
    , m_strPoseClasses("")
    , m_bShowCubes( true )
{

    for( int i = 1; i < argc; i++ )
    {
        if( !stricmp( argv[i], "-gldebug" ) )
        {
            m_bDebugOpenGL = true;
        }
        else if( !stricmp( argv[i], "-verbose" ) )
        {
            m_bVerbose = true;
        }
        else if( !stricmp( argv[i], "-novblank" ) )
        {
            m_bVblank = false;
        }
        else if( !stricmp( argv[i], "-noglfinishhack" ) )
        {
            m_bGlFinishHack = false;
        }
        else if( !stricmp( argv[i], "-noprintf" ) )
        {
            g_bPrintf = false;
        }
        else if ( !stricmp( argv[i], "-cubevolume" ) && ( argc > i + 1 ) && ( *argv[ i + 1 ] != '-' ) )
        {
            m_iSceneVolumeInit = atoi( argv[ i + 1 ] );
            i++;
        }
    }
    // other initialization tasks are done in BInit
    memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
    // work is done in Shutdown
    dprintf( "Shutdown" );
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//            into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString( vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
    uint32_t unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
    if( unRequiredBufferLen == 0 )
        return "";

    char *pchBuffer = new char[ unRequiredBufferLen ];
    unRequiredBufferLen = vr::VRSystem()->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
    std::string sResult = pchBuffer;
    delete [] pchBuffer;
    return sResult;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{
    if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
    {
        printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
        return false;
    }

    // Loading the SteamVR Runtime
    vr::EVRInitError eError = vr::VRInitError_None;
    m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

    if ( eError != vr::VRInitError_None )
    {
        m_pHMD = NULL;
        char buf[1024];
        sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
        SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
        return false;
    }


    int nWindowPosX = 700;
    int nWindowPosY = 100;
    Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
    //SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
    if( m_bDebugOpenGL )
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );

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

    if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 )
    {
        printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
        return false;
    }


    m_strDriver = "No Driver";
    m_strDisplay = "No Display";

    m_strDriver = GetTrackedDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
    m_strDisplay = GetTrackedDeviceString( vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

    std::string strWindowTitle = "hellovr - " + m_strDriver + " " + m_strDisplay;
    SDL_SetWindowTitle( m_pCompanionWindow, strWindowTitle.c_str() );
    
    // cube array
     m_iSceneVolumeWidth = m_iSceneVolumeInit;
     m_iSceneVolumeHeight = m_iSceneVolumeInit;
     m_iSceneVolumeDepth = m_iSceneVolumeInit;
         
     m_fScale = 0.3f;
     m_fScaleSpacing = 4.0f;
 
     m_fNearClip = 0.1f;
     m_fFarClip = 30.0f;
 
     m_iTexture = 0;
     m_uiVertcount = 0;
 
//         m_MillisecondsTimer.start(1, this);
//         m_SecondsTimer.start(1000, this);
    
    if (!BInitGL())
    {
        printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
        return false;
    }

    if (!BInitCompositor())
    {
        printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
        return false;
    }
#if 0
    vr::VRInput()->SetActionManifestPath( Path_MakeAbsolute( "../hellovr_actions.json", Path_StripFilename( Path_GetExecutablePath() ) ).c_str() );

    vr::VRInput()->GetActionHandle( "/actions/demo/in/HideCubes", &m_actionHideCubes );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/HideThisController", &m_actionHideThisController);
    vr::VRInput()->GetActionHandle( "/actions/demo/in/TriggerHaptic", &m_actionTriggerHaptic );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/AnalogInput", &m_actionAnalongInput );

    vr::VRInput()->GetActionSetHandle( "/actions/demo", &m_actionsetDemo );

    vr::VRInput()->GetActionHandle( "/actions/demo/out/Haptic_Left", &m_rHand[Left].m_actionHaptic );
    vr::VRInput()->GetInputSourceHandle( "/user/hand/left", &m_rHand[Left].m_source );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/Hand_Left", &m_rHand[Left].m_actionPose );

    vr::VRInput()->GetActionHandle( "/actions/demo/out/Haptic_Right", &m_rHand[Right].m_actionHaptic );
    vr::VRInput()->GetInputSourceHandle( "/user/hand/right", &m_rHand[Right].m_source );
    vr::VRInput()->GetActionHandle( "/actions/demo/in/Hand_Right", &m_rHand[Right].m_actionPose );
#endif
    return true;
}


//-----------------------------------------------------------------------------
// Purpose: Outputs the string in message to debugging output.
//          All other parameters are ignored.
//          Does not return any meaningful value or reference.
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
    dprintf( "GL Error: %s\n", message );
}


//-----------------------------------------------------------------------------
// Purpose: Initialize OpenGL. Returns true if OpenGL has been successfully
//          initialized, false if shaders could not be created.
//          If failure occurred in a module other than shaders, the function
//          may return true or throw an error. 
//-----------------------------------------------------------------------------
bool CMainApplication::BInitGL()
{
    if( m_bDebugOpenGL )
    {
        glDebugMessageCallback( (GLDEBUGPROC)DebugCallback, nullptr);
        glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    }

#if 0
    if( !CreateAllShaders() )
        return false;
#endif
    //SetupTexturemaps();
    //SetupScene();
    //SetupCameras();
    SetupStereoRenderTargets();
    SetupCompanionWindow();

    return true;
}


//-----------------------------------------------------------------------------
// Purpose: Initialize Compositor. Returns true if the compositor was
//          successfully initialized, false otherwise.
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor()
{
    vr::EVRInitError peError = vr::VRInitError_None;

    if ( !vr::VRCompositor() )
    {
        printf( "Compositor initialization failed. See log file for details\n" );
        return false;
    }

    return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
    if( m_pHMD )
    {
        vr::VR_Shutdown();
        m_pHMD = NULL;
    }
#if 0
    for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
    {
        delete (*i);
    }
    m_vecRenderModels.clear();
#endif
    if( m_pContext )
    {
        if( m_bDebugOpenGL )
        {
            glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE );
            glDebugMessageCallback(nullptr, nullptr);
        }
        glDeleteBuffers(1, &m_glSceneVertBuffer);

        if ( m_unSceneProgramID )
        {
            glDeleteProgram( m_unSceneProgramID );
        }
        if ( m_unControllerTransformProgramID )
        {
            glDeleteProgram( m_unControllerTransformProgramID );
        }
        if ( m_unRenderModelProgramID )
        {
            glDeleteProgram( m_unRenderModelProgramID );
        }
        if ( m_unCompanionWindowProgramID )
        {
            glDeleteProgram( m_unCompanionWindowProgramID );
        }

        glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
        glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
        glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
        glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
        glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

        glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
        glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
        glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
        glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
        glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );

        if( m_unCompanionWindowVAO != 0 )
        {
            glDeleteVertexArrays( 1, &m_unCompanionWindowVAO );
        }
        if( m_unSceneVAO != 0 )
        {
            glDeleteVertexArrays( 1, &m_unSceneVAO );
        }
        if( m_unControllerVAO != 0 )
        {
            glDeleteVertexArrays( 1, &m_unControllerVAO );
        }
    }

    if( m_pCompanionWindow )
    {
        SDL_DestroyWindow(m_pCompanionWindow);
        m_pCompanionWindow = NULL;
    }

    SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
    bool bQuit = false;

    SDL_StartTextInput();
    SDL_ShowCursor( SDL_DISABLE );

    while ( !bQuit )
    {
      //bQuit = HandleInput();

        RenderFrame();
    }

    SDL_StopTextInput();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame()
{
    // for now as fast as possible
    if ( m_pHMD )
    {
      //RenderControllerAxes();
      //RenderStereoTargets();
      //RenderCompanionWindow();

        vr::Texture_t leftEyeTexture = {(void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
        vr::Texture_t rightEyeTexture = {(void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        auto res = vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
    //std::cout << "res = " << res << std::endl;
    }

    if ( m_bVblank && m_bGlFinishHack )
    {
        //$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
        // happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
        // appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
        // 1/29/2014 mikesart
        glFinish();
    }

    // SwapWindow
    {
        SDL_GL_SwapWindow( m_pCompanionWindow );
    }

    // Clear
    {
        // We want to make sure the glFinish waits for the entire present to complete, not just the submission
        // of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
        glClearColor( 0, 0, 0, 1 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    // Flush and wait for swap.
    if ( m_bVblank )
    {
        glFlush();
        glFinish();
    }

#if 1
    // Spew out the controller and pose count whenever they change.
    if ( m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last )
    {
        m_iValidPoseCount_Last = m_iValidPoseCount;
        m_iTrackedControllerCount_Last = m_iTrackedControllerCount;
        
        dprintf( "PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
    }
  // critical
    UpdateHMDMatrixPose();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Creates a frame buffer. Returns true if the buffer was set up.
//          Returns false if the setup failed.
//-----------------------------------------------------------------------------
bool CMainApplication::CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc )
{
    glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

    glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight );
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,    framebufferDesc.m_nDepthBufferId );

    glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId );
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

    glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
    glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

    glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
    glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        return false;
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets()
{
    if ( !m_pHMD )
        return false;

    m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

    CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, leftEyeDesc );
    CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, rightEyeDesc );
    
    return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCompanionWindow()
{
    if ( !m_pHMD )
        return;

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

//-----------------------------------------------------------------------------
// Purpose: Gets a Matrix Projection Eye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
    if ( !m_pHMD )
        return Matrix4();

    vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip );

    return Matrix4(
        mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
        mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
        mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
        mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
    );
}


//-----------------------------------------------------------------------------
// Purpose: Gets an HMDMatrixPoseEye with respect to nEye.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
    if ( !m_pHMD )
        return Matrix4();

    vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
    Matrix4 matrixObj(
        matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
        matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
        matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
        matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
        );

    return matrixObj.invert();
}


//-----------------------------------------------------------------------------
// Purpose: Gets a Current View Projection Matrix with respect to nEye,
//          which may be an Eye_Left or an Eye_Right.
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye )
{
    Matrix4 matMVP;
    if( nEye == vr::Eye_Left )
    {
        matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
    }
    else if( nEye == vr::Eye_Right )
    {
        matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
    }

    return matMVP;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::UpdateHMDMatrixPose()
{
    if ( !m_pHMD )
        return;
    vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );
}

//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose )
{
    Matrix4 matrixObj(
        matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
        matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
        matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
        matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
        );
    return matrixObj;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    CMainApplication *pMainApplication = new CMainApplication( argc, argv );

    if (!pMainApplication->BInit())
    {
        pMainApplication->Shutdown();
        return 1;
    }

    pMainApplication->RunMainLoop();

    pMainApplication->Shutdown();

    return 0;
}
