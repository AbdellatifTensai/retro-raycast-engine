#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t u8;
typedef size_t usize;
typedef float f32;
typedef enum{ false=0, true=!false } bool;

typedef struct{ f32 x, y;} v2f;
typedef struct{ u32 x, y;} v2u;
typedef struct{ u8 r, g, b;} v3u;

typedef struct{
	Display *Display;
	Window Window;
	u32 WindowWidth, WindowHeight;
	XImage *Image;
	Atom WMDeleteWindow;
	GC GC;
} x11app;
static x11app App;

typedef struct{
	u32 Width, Height, *Pixels; 
} frame_buffer;

#ifdef PROFILE
static struct timespec BeginTime, EndTime;
static usize ElapsedTime;
#endif

void InitWindow(frame_buffer Frame, const char *Title){
	App.Display = XOpenDisplay(NULL);
	XSetWindowAttributes SetAttributes = {0};
	App.Window = XCreateWindow(App.Display, XRootWindow(App.Display, 0), 0, 0, Frame.Width, Frame.Height, 0, 0, InputOutput, CopyFromParent, CWOverrideRedirect, &SetAttributes);
	App.WindowWidth = Frame.Width;
	App.WindowHeight = Frame.Height;
	XWindowAttributes WindowAttributes = {0};
	XGetWindowAttributes(App.Display, App.Window, &WindowAttributes);
	App.Image = XCreateImage(App.Display, WindowAttributes.visual, WindowAttributes.depth, ZPixmap, 0, (char *)Frame.Pixels, Frame.Width, Frame.Height, 32, Frame.Width*4);  
	App.WMDeleteWindow = XInternAtom(App.Display, "WM_DELETE_WINDOW", false);
	App.GC = XDefaultGC(App.Display, 0);

	XSizeHints Hints = {.min_width = Frame.Width, .min_height = Frame.Height, .max_height = Frame.Height, .max_width = Frame.Width, .flags = PMinSize | PMaxSize};
	XSetSizeHints(App.Display, App.Window, &Hints, XA_WM_NORMAL_HINTS);

	XSetWMProtocols(App.Display, App.Window, &App.WMDeleteWindow, 1);
	XStoreName(App.Display, App.Window, Title);
	XSelectInput(App.Display, App.Window, KeyPressMask | ExposureMask | ButtonPressMask | Button1MotionMask);
	XMapWindow(App.Display, App.Window);
	XSync(App.Display, false);
}

void DrawImage(){
	XPutImage(App.Display, App.Window, App.GC, App.Image, 0, 0, 0, 0, App.WindowWidth, App.WindowHeight);
	XSync(App.Display, false);
}

#define ABS(a) ((a) > 0? (a): (-(a)))
#define MAX(a, b) ((a) > (b)? (a): (b))
#define MIN(a, b) ((a) < (b)? (a): (b))
#define CLAMP(x, a, b) MAX(a, MIN(x, b))

f32 Ceil(f32 a){ return (f32)(s32)(a+1); }
f32 Floor(f32 a){ return (f32)(s32) a; }
f32 Fract(f32 a){ return a >= 0? a - Floor(a): a - Ceil(a-2); }

void RenderLine(frame_buffer Frame, u32 x0, u32 y0, u32 x1, u32 y1, u32 LineThickness, u32 Color){
	//NOTE: Simplify this
	//use CLAMP
	//u32 StartX = x0 < LineThickness? x0: x0 - LineThickness;
	//u32 EndX = x1 + LineThickness > Frame.Width? x1: x1 + LineThickness;
	u32 StartX = CLAMP(x0-LineThickness, 0, x0);
	u32 EndX = CLAMP(x1+LineThickness, x1, WindowHeight);
	if(x0 > x1){
		StartX = x1 < LineThickness? x1: x1 - LineThickness;
		EndX = x0 + LineThickness > Frame.Width? x0: x0 + LineThickness;
	}
	u32 StartY = y0 < LineThickness? y0: y0 - LineThickness;
	u32 EndY = y1 + LineThickness > Frame.Height? y1: y1 + LineThickness;
	if(y0 > y1){
		StartY = y1 < LineThickness? y1: y1 - LineThickness;
		EndY = y0 + LineThickness > Frame.Height? y0: y0 + LineThickness;
	}

	v2f P1 = (v2f){ .x = (f32)x0/ Frame.Width, .y = (f32)y0 / Frame.Height };
	v2f P2 = (v2f){ .x = (f32)x1/ Frame.Width, .y = (f32)y1 / Frame.Height };
	v2f P2_P1 = (v2f){ .x = P2.x - P1.x, .y = P2.y - P1.y };
	f32 MagP2_P1 = sqrt(P2_P1.x * P2_P1.x + P2_P1.y * P2_P1.y);
	f32 Thickness = (f32)LineThickness / sqrt(Frame.Width*Frame.Width + Frame.Height*Frame.Height);

	for(u32 y=StartY; y<=EndY; y++)
		for(u32 x=StartX; x<=EndX; x++){
			v2f P = (v2f){ .x = (f32)x / Frame.Width, .y = (f32)y / Frame.Height };
			v2f P_P1 = (v2f){ .x = P1.x - P.x, .y = P1.y - P.y };
			f32 Dist = (P_P1.x * P2_P1.y - P_P1.y * P2_P1.x) / MagP2_P1;
			Dist = ABS(Dist);
			if(Dist <= Thickness)
				Frame.Pixels[y * Frame.Width + x] = Color;
		}
}

void RenderBackground(frame_buffer Frame, u32 Color){
	memset(Frame.Pixels, Color, Frame.Width * Frame.Height * sizeof(*Frame.Pixels));
}

void RenderGrid(frame_buffer Frame){
	u32 GridColumns = 10;
	u32 GridRows = 10;
	u32 GridWidth = Frame.Width/GridColumns;
	u32 GridHeight = Frame.Height/GridRows;
	u32 LineThickness = 2;

	for(u32 x=0; x<GridColumns; x++)
		RenderLine(Frame, x*GridWidth, 0, x*GridWidth, Frame.Height, LineThickness, 0x00282828);
	
	for(u32 y=0; y<GridRows; y++)
		RenderLine(Frame, 0, y*GridHeight, Frame.Width, y*GridHeight, LineThickness, 0x00282828);
	
}

int main(){

	#define WINDOW_WIDTH 800
	#define WINDOW_HEIGHT 800
	u32 Buffer[ WINDOW_HEIGHT * WINDOW_WIDTH ];
	frame_buffer Frame = { .Width = WINDOW_HEIGHT, .Height = WINDOW_WIDTH, .Pixels = Buffer };

	InitWindow(Frame, "Raycast Engine");

	XEvent Event = {0};
	bool WindowShouldClose = false;
	//v2u MousePos = {0,0};
	while(!WindowShouldClose){

#ifdef PROFILE
		clock_gettime(CLOCK_REALTIME, &BeginTime);
#endif

		while(XPending(App.Display)){
			XNextEvent(App.Display, &Event);
			switch(Event.type){
			case ClientMessage:{
				if((Atom)Event.xclient.data.l[0] == App.WMDeleteWindow)
					WindowShouldClose = true;
			}break;

			case ButtonPress:
			case MotionNotify:{
				//MousePos = (v2u){ Event.xbutton.x, Event.xbutton.y };
			}break;

			case Expose: DrawImage(); break;
			}
		}

		//clock_gettime(CLOCK_REALTIME, &BeginTime);

		//RenderGradient(Pixels, WindowWidth, WindowHeight, OffsetX++, OffsetY);
		RenderBackground(Frame, 0x00181818);
		RenderGrid(Frame);
		RenderLine(Frame, 250, 250, 350, 350, 1, 0x00FF00FF);
		DrawImage();

#ifdef PROFILE
		clock_gettime(CLOCK_REALTIME, &EndTime);
		ElapsedTime = ((usize)EndTime.tv_sec - (usize)BeginTime.tv_sec) + ((usize)EndTime.tv_nsec - (usize)BeginTime.tv_nsec);
		printf("%luns   %.2fms   %dFPS\n", ElapsedTime, ElapsedTime*1e-6, (s32)(1/(ElapsedTime*1e-9)));
#endif

	}

	XFree(App.Image);
	XCloseDisplay(App.Display);

	return 0;
}
