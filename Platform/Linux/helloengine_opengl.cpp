//
// Created by creator on 19-3-21.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

//显卡是被XServer屏蔽的，所以需要通过GLX的X扩展库，
//将3D绘图指令以X协议发给XServer，然后XServer发给显卡
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0X2092

//函数指针
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display *, GLXFBConfig, GLXContext, Bool, const int *);


static bool isExtensionSupported(const char *extList, const char *extension)
{
    //指向常量的指针，指针本身可以变
    const char *start;
    const char *where, *terminator;

    //Extension name should not have spaces
    //strchr找某字符首次出现的位置
    where = strchr(extension, ' ');
    if (where||*extension == '\0')
        return false;

    //It takes a bit of care to be fool-proof about parsing the
    //OpenGl extensions string. Don't be fooled by sub-strings
    for (start = extList;;)
    {
        //寻找extension是否是start的子串，返回首次的地址
        where = strstr(start, extension);

        if (!where) break;

        terminator = where + strlen(extension);

        if (where == start||*(where - 1) == ' ')
            if (*terminator == ' '||*terminator == '\0')
                return true;

        start = terminator;
    }

    return false;
}

static bool ctxErrorOccurred = false;
static int ctxErrorHandle(Display *dpy, XErrorEvent *ev)
{
    ctxErrorOccurred=true;
    return 0;
}

void DrawAQuad()
{
    glClearColor(1.0,1.0,1.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    //投影模式
    glMatrixMode(GL_PROJECTION);
    //单位矩阵
    glLoadIdentity();
    glOrtho(-1.,1.,-1.,1.,1.,20.);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.,0.,10.,0.,0.,0.,0.,1.,0.);

    glBegin(GL_QUADS);
    glColor3f(1.,0.,0.);
    glVertex3d(-.75,-.75,0.);
    glColor3f(0.,1.,0.);
    glVertex3d(.75,-.75,0.);
    glColor3f(0.,0.,1.);
    glVertex3d(.75,.75,0.);
    glColor3f(1.,1.,0.);
    glVertex3d(-.75,.75,0.);
    glEnd();
}

int main(void)
{
    xcb_connection_t *pConn;
    xcb_screen_t *pScreen;
    xcb_window_t window;
    xcb_gcontext_t foreground;
    xcb_gcontext_t background;
    xcb_generic_event_t *pEvent;
    xcb_colormap_t colormap;
    uint32_t mask = 0;
    uint32_t values[3];
    uint8_t isQuit = 0;

    char title[] = "Hello, Engine![OpenGL]";
    char title_icon[] = "Hello, Engine! (iconified)";

    Display *display;
    int default_screen;
    GLXContext context;
    GLXFBConfig *fb_configs;
    GLXFBConfig fb_config;
    int num_fb_configs=0;
    XVisualInfo *vi;
    GLXDrawable  drawable;
    GLXWindow glxwindow;
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB;
    const char *glxExts;

    //Get a matching FB config
    static int visual_attribs[]=
            {
                    GLX_X_RENDERABLE,True,
                    GLX_DRAWABLE_TYPE,GLX_WINDOW_BIT,
                    GLX_RENDER_TYPE,GLX_RGBA_BIT,
                    GLX_X_VISUAL_TYPE,GLX_TRUE_COLOR,
                    GLX_RED_SIZE,8,
                    GLX_GREEN_SIZE,8,
                    GLX_BLUE_SIZE,8,
                    GLX_ALPHA_SIZE,8,
                    GLX_DEPTH_SIZE,24,
                    GLX_STENCIL_SIZE,8,
                    GLX_DOUBLEBUFFER,True,
                    None
            };

    int glx_major,glx_minor;

    //Open Xlib Display
    display=XOpenDisplay(NULL);
    if(!display)
    {
        fprintf(stderr,"Can't open display\n");
        return -1;
    }

    //FBConfigs were added in GLX version 1.3
    if(!glXQueryVersion(display,&glx_major,&glx_minor)||
            ((glx_major==1)&&(glx_minor<3))||(glx_major<1))
    {
        fprintf(stderr, "Invalid GLX version\n");
        return -1;
    }

    default_screen=DefaultScreen(display);

    //Query framebuffer configurations
    fb_configs=glXChooseFBConfig(display,default_screen,visual_attribs,&num_fb_configs);
    if(!fb_configs||num_fb_configs==0)
    {
        fprintf(stderr,"glXGetFBConfigs failed!\n");
        return -1;
    }

    //Pick the FB config/visual with the most samples per pixel
    {
        int best_fbc =-1,worst_fbc=-1,best_num_samp=-1,worst_num_samp=999;

        for(int i=0;i<num_fb_configs;++i)
        {
            XVisualInfo *vi=glXGetVisualFromFBConfig(display,fb_configs[i]);
            if(vi)
            {
                int samp_buf,samples;
                glXGetFBConfigAttrib(display,fb_configs[i],GLX_SAMPLE_BUFFERS,&samp_buf);
                glXGetFBConfigAttrib(display,fb_configs[i],GLX_SAMPLES,&samples);
                printf("Matching fbconfig %d, visual ID 0x%lx: SAMPLE_BUFFERS = %d,"
                "SAMPLES = %d\n",
                i, vi->visualid,samp_buf,samples);

                if(best_fbc<0||(samp_buf&&samples>best_num_samp))
                    best_fbc=i,best_num_samp=samples;
                if(worst_fbc<0||!samp_buf||samples<worst_num_samp)
                    worst_fbc=i,worst_num_samp=samples;
            }
            XFree(vi);
        }

        fb_config=fb_configs[best_fbc];
    }

    //Get a visual
    vi=glXGetVisualFromFBConfig(display,fb_config);
    printf("Chosen visual ID = 0x%lx\n",vi->visualid);

    /* establish connection to X server */
    pConn = XGetXCBConnection(display);
    if(!pConn)
    {
        XCloseDisplay(display);
        fprintf(stderr, "Can't get xcb connection from display\n");
        return -1;
    }

    //Acquire event queue ownership
    XSetEventQueueOwner(display,XCBOwnsEventQueue);

    //Find XCB screen
    xcb_screen_iterator_t screen_iter=
            xcb_setup_roots_iterator(xcb_get_setup(pConn));
    for(int screen_num=vi->screen;screen_iter.rem&&screen_num>0;
        --screen_num,xcb_screen_next(&screen_iter));
    pScreen=screen_iter.data;

    /* get the root window */
    window = pScreen->root;

    //Create SID's for colormap
    colormap=xcb_generate_id(pConn);

    /* create white (background) graphic context */
    xcb_create_colormap(pConn,XCB_COLORMAP_ALLOC_NONE,colormap,window,vi->visualid);

    /* create window */
    window = xcb_generate_id(pConn);
    mask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
    values[0] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
    values[1] = colormap;
    values[2]=0;
    xcb_create_window(pConn,                       /* connection */
                      XCB_COPY_FROM_PARENT,        /* depth */
                      window,                      /* window ID */
                      pScreen->root,               /* parent window */
                      20, 20,                      /* x, y */
                      640, 480,                    /* width, height */
                      10,                          /* boarder width */
                      XCB_WINDOW_CLASS_INPUT_OUTPUT, /* class */
                      vi->visualid,        /* visual */
                      mask, values);               /* masks */

    XFree(vi);

    /* set the title of the window */
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, window,
                        XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
                        strlen(title), title);

    /* set the title of the window icon */
    xcb_change_property(pConn, XCB_PROP_MODE_REPLACE, window,
                        XCB_ATOM_WM_ICON_NAME, XCB_ATOM_STRING, 8,
                        strlen(title_icon), title_icon);

    /* map the window on the screen */
    xcb_map_window(pConn, window);

    xcb_flush(pConn);

    //Get the default screen's GLX extension list
    glxExts=glXQueryExtensionsString(display,default_screen);

    glXCreateContextAttribsARB=(glXCreateContextAttribsARBProc)
            glXGetProcAddressARB((const GLubyte *)"glXCreateContextAtrribsARB");

    //create OpenGL context
    ctxErrorOccurred=false;
    int (*oldHandler)(Display *, XErrorEvent *)=
        XSetErrorHandler(&ctxErrorHandle);

    if(!isExtensionSupported(glxExts, "GLX_ARB_create_context")||!glXCreateContextAttribsARB)
    {
        printf("glXCreateContextAttribsARB() not found");
        context=glXCreateNewContext(display,fb_config,GLX_RGBA_TYPE,0,True);
        if(!context)
        {
            fprintf(stderr,"glXCreateNewContext failed\n");
            return -1;
        }
    } else{
        int context_attribs[]=
                {
                        GLX_CONTEXT_MAJOR_VERSION_ARB,3,
                        GLX_CONTEXT_MINOR_VERSION_ARB,0,
                        None
                };

        printf("Create context\n");
        context=glXCreateContextAttribsARB(display,fb_config,0,True,context_attribs);

        XSync(display,False);
        if(!ctxErrorOccurred&&context)
            printf("Create GL 3.0 context!\n");
        else{
            context_attribs[1]=1;
            context_attribs[3]=0;

            ctxErrorOccurred=false;

            printf("Failed to create GL 3.0 context\n");

            context=glXCreateContextAttribsARB(display,fb_config,0,True,context_attribs);
        }
    }

    XSync(display,False);

    XSetErrorHandler(oldHandler);

    if(ctxErrorOccurred||!context)
    {
        printf("Failed to create an OpenGL context\n");
        return -1;
    }

    //Verifying that context is a direct context
    if(!glXIsDirect(display,context))
    {
        printf("Indirect GLX rendering context obtained\n");
    }
    else
    {
        printf("Direct GLX rendering context obtained\n");
    }

    //create GLX window
    glxwindow=glXCreateWindow(display,fb_config,window,0);

    if(!window)
    {
        xcb_destroy_window(pConn,window);
        glXDestroyContext(display,context);

        fprintf(stderr,"glXDestroyContext failed\n");
        return -1;
    }

    drawable=glxwindow;

    //make OpenGL context current
    if(!glXMakeContextCurrent(display,drawable,drawable,context))
    {
        xcb_destroy_window(pConn,window);
        glXDestroyContext(display,context);

        fprintf(stderr,"glXMakeContextCurrent failed\n");
        return -1;
    }

    while(!isQuit&&(pEvent=xcb_wait_for_event(pConn)))
    {
        switch (pEvent->response_type & ~0x80)
        {
            case XCB_EXPOSE:
            {
                DrawAQuad();
                glXSwapBuffers(display,drawable);
            }
                break;
            case XCB_KEY_PRESS:
                isQuit = 1;
                break;
        }
        free(pEvent);
    }

    xcb_disconnect(pConn);
    return 0;
}
