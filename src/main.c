#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef uint64_t u64;
typedef uint32_t u32;
typedef int32_t s32;
typedef uint8_t u8;
typedef size_t usize;
typedef float f32;
typedef enum{ false=0, true=!false } bool;

typedef struct{ f32 x, y;} v2f;
typedef struct{ s32 x, y;} v2s;
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
	s32 Width, Height;
	u32 *Pixels; 
} frame_buffer;

//from: https://nakst.gitlab.io/tutorial/ui-part-7.html
#define GLYPH_WIDTH (8)
#define GLYPH_HEIGHT (16)
#define GLYPH_GAP (1)
#define GLYPH_SIZE (GLYPH_WIDTH*GLYPH_HEIGHT/sizeof(u8)) // = 16
u64 BitmapFont[] = {
	0x0000000000000000UL, 0x0000000000000000UL, 0xBD8181A5817E0000UL, 0x000000007E818199UL, 0xC3FFFFDBFF7E0000UL, 0x000000007EFFFFE7UL, 0x7F7F7F3600000000UL, 0x00000000081C3E7FUL, 0x7F3E1C0800000000UL, 0x0000000000081C3EUL, 0xE7E73C3C18000000UL, 0x000000003C1818E7UL, 0xFFFF7E3C18000000UL, 0x000000003C18187EUL, 0x3C18000000000000UL, 0x000000000000183CUL,  0xC3E7FFFFFFFFFFFFUL, 0xFFFFFFFFFFFFE7C3UL, 0x42663C0000000000UL, 0x00000000003C6642UL, 0xBD99C3FFFFFFFFFFUL, 0xFFFFFFFFFFC399BDUL, 0x331E4C5870780000UL, 0x000000001E333333UL, 0x3C666666663C0000UL, 0x0000000018187E18UL, 0x0C0C0CFCCCFC0000UL, 0x00000000070F0E0CUL, 0xC6C6C6FEC6FE0000UL, 0x0000000367E7E6C6UL, 0xE73CDB1818000000UL, 0x000000001818DB3CUL,  0x1F7F1F0F07030100UL, 0x000000000103070FUL, 0x7C7F7C7870604000UL, 0x0000000040607078UL, 0x1818187E3C180000UL, 0x0000000000183C7EUL, 0x6666666666660000UL, 0x0000000066660066UL,  0xD8DEDBDBDBFE0000UL, 0x00000000D8D8D8D8UL, 0x6363361C06633E00UL, 0x0000003E63301C36UL, 0x0000000000000000UL, 0x000000007F7F7F7FUL, 0x1818187E3C180000UL, 0x000000007E183C7EUL,  0x1818187E3C180000UL, 0x0000000018181818UL, 0x1818181818180000UL, 0x00000000183C7E18UL, 0x7F30180000000000UL, 0x0000000000001830UL, 0x7F060C0000000000UL, 0x0000000000000C06UL, 0x0303000000000000UL, 0x0000000000007F03UL, 0xFF66240000000000UL, 0x0000000000002466UL, 0x3E1C1C0800000000UL, 0x00000000007F7F3EUL, 0x3E3E7F7F00000000UL, 0x0000000000081C1CUL,  0x0000000000000000UL, 0x0000000000000000UL, 0x18183C3C3C180000UL, 0x0000000018180018UL, 0x0000002466666600UL, 0x0000000000000000UL, 0x36367F3636000000UL, 0x0000000036367F36UL,  0x603E0343633E1818UL, 0x000018183E636160UL, 0x1830634300000000UL, 0x000000006163060CUL, 0x3B6E1C36361C0000UL, 0x000000006E333333UL, 0x000000060C0C0C00UL, 0x0000000000000000UL,  0x0C0C0C0C18300000UL, 0x0000000030180C0CUL, 0x30303030180C0000UL, 0x000000000C183030UL, 0xFF3C660000000000UL, 0x000000000000663CUL, 0x7E18180000000000UL, 0x0000000000001818UL, 0x0000000000000000UL, 0x0000000C18181800UL, 0x7F00000000000000UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000000018180000UL, 0x1830604000000000UL, 0x000000000103060CUL,  0xDBDBC3C3663C0000UL, 0x000000003C66C3C3UL, 0x1818181E1C180000UL, 0x000000007E181818UL, 0x0C183060633E0000UL, 0x000000007F630306UL, 0x603C6060633E0000UL, 0x000000003E636060UL,  0x7F33363C38300000UL, 0x0000000078303030UL, 0x603F0303037F0000UL, 0x000000003E636060UL, 0x633F0303061C0000UL, 0x000000003E636363UL, 0x18306060637F0000UL, 0x000000000C0C0C0CUL,  0x633E6363633E0000UL, 0x000000003E636363UL, 0x607E6363633E0000UL, 0x000000001E306060UL, 0x0000181800000000UL, 0x0000000000181800UL, 0x0000181800000000UL, 0x000000000C181800UL, 0x060C183060000000UL, 0x000000006030180CUL, 0x00007E0000000000UL, 0x000000000000007EUL, 0x6030180C06000000UL, 0x00000000060C1830UL, 0x18183063633E0000UL, 0x0000000018180018UL,  0x7B7B63633E000000UL, 0x000000003E033B7BUL, 0x7F6363361C080000UL, 0x0000000063636363UL, 0x663E6666663F0000UL, 0x000000003F666666UL, 0x03030343663C0000UL, 0x000000003C664303UL,  0x66666666361F0000UL, 0x000000001F366666UL, 0x161E1646667F0000UL, 0x000000007F664606UL, 0x161E1646667F0000UL, 0x000000000F060606UL, 0x7B030343663C0000UL, 0x000000005C666363UL,  0x637F636363630000UL, 0x0000000063636363UL, 0x18181818183C0000UL, 0x000000003C181818UL, 0x3030303030780000UL, 0x000000001E333333UL, 0x1E1E366666670000UL, 0x0000000067666636UL, 0x06060606060F0000UL, 0x000000007F664606UL, 0xC3DBFFFFE7C30000UL, 0x00000000C3C3C3C3UL, 0x737B7F6F67630000UL, 0x0000000063636363UL, 0x63636363633E0000UL, 0x000000003E636363UL,  0x063E6666663F0000UL, 0x000000000F060606UL, 0x63636363633E0000UL, 0x000070303E7B6B63UL, 0x363E6666663F0000UL, 0x0000000067666666UL, 0x301C0663633E0000UL, 0x000000003E636360UL,  0x18181899DBFF0000UL, 0x000000003C181818UL, 0x6363636363630000UL, 0x000000003E636363UL, 0xC3C3C3C3C3C30000UL, 0x00000000183C66C3UL, 0xDBC3C3C3C3C30000UL, 0x000000006666FFDBUL,  0x18183C66C3C30000UL, 0x00000000C3C3663CUL, 0x183C66C3C3C30000UL, 0x000000003C181818UL, 0x0C183061C3FF0000UL, 0x00000000FFC38306UL, 0x0C0C0C0C0C3C0000UL, 0x000000003C0C0C0CUL, 0x1C0E070301000000UL, 0x0000000040607038UL, 0x30303030303C0000UL, 0x000000003C303030UL, 0x0000000063361C08UL, 0x0000000000000000UL, 0x0000000000000000UL, 0x0000FF0000000000UL,  0x0000000000180C0CUL, 0x0000000000000000UL, 0x3E301E0000000000UL, 0x000000006E333333UL, 0x66361E0606070000UL, 0x000000003E666666UL, 0x03633E0000000000UL, 0x000000003E630303UL,  0x33363C3030380000UL, 0x000000006E333333UL, 0x7F633E0000000000UL, 0x000000003E630303UL, 0x060F0626361C0000UL, 0x000000000F060606UL, 0x33336E0000000000UL, 0x001E33303E333333UL,  0x666E360606070000UL, 0x0000000067666666UL, 0x18181C0018180000UL, 0x000000003C181818UL, 0x6060700060600000UL, 0x003C666660606060UL, 0x1E36660606070000UL, 0x000000006766361EUL, 0x18181818181C0000UL, 0x000000003C181818UL, 0xDBFF670000000000UL, 0x00000000DBDBDBDBUL, 0x66663B0000000000UL, 0x0000000066666666UL, 0x63633E0000000000UL, 0x000000003E636363UL,  0x66663B0000000000UL, 0x000F06063E666666UL, 0x33336E0000000000UL, 0x007830303E333333UL, 0x666E3B0000000000UL, 0x000000000F060606UL, 0x06633E0000000000UL, 0x000000003E63301CUL,  0x0C0C3F0C0C080000UL, 0x00000000386C0C0CUL, 0x3333330000000000UL, 0x000000006E333333UL, 0xC3C3C30000000000UL, 0x00000000183C66C3UL, 0xC3C3C30000000000UL, 0x0000000066FFDBDBUL,  0x3C66C30000000000UL, 0x00000000C3663C18UL, 0x6363630000000000UL, 0x001F30607E636363UL, 0x18337F0000000000UL, 0x000000007F63060CUL, 0x180E181818700000UL, 0x0000000070181818UL, 0x1800181818180000UL, 0x0000000018181818UL, 0x18701818180E0000UL, 0x000000000E181818UL, 0x000000003B6E0000UL, 0x0000000000000000UL, 0x63361C0800000000UL, 0x00000000007F6363UL, 
};            

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

void RenderLine(frame_buffer Frame, v2s P0, v2s P1, s32 LineThickness, u32 Color){
	s32 StartX = MAX((P0.x - LineThickness), 0);
	s32 EndX = MIN(P1.x+LineThickness, Frame.Width);
	if(P0.x > P1.x){
		StartX = MAX((P1.x - LineThickness), 0);
		EndX = MIN(P0.x+LineThickness, Frame.Width);
	}

	s32 StartY = MAX((P0.y - LineThickness), 0);
	s32 EndY = MIN(P1.y+LineThickness, Frame.Width);
	if(P0.y > P1.y){
		StartY = MAX((P1.y - LineThickness), 0);
		EndY = MIN(P0.y+LineThickness, Frame.Width);
	}

	v2f P0N = (v2f){ .x = (f32)P0.x/ Frame.Width, .y = (f32)P0.y / Frame.Height };
	v2f P1N = (v2f){ .x = (f32)P1.x/ Frame.Width, .y = (f32)P1.y / Frame.Height };
	v2f P1N_P0N = (v2f){ .x = P1N.x - P0N.x, .y = P1N.y - P0N.y };
	f32 MagP1N_P0N = sqrt(P1N_P0N.x * P1N_P0N.x + P1N_P0N.y * P1N_P0N.y);
	f32 Thickness = (f32)LineThickness / sqrt(Frame.Width*Frame.Width + Frame.Height*Frame.Height);

	for(s32 y=StartY; y<=EndY; y++)
		for(s32 x=StartX; x<=EndX; x++){
			v2f P = (v2f){ .x = (f32)x / Frame.Width, .y = (f32)y / Frame.Height };
			v2f P_P0N = (v2f){ .x = P0N.x - P.x, .y = P0N.y - P.y };
			f32 Dist = (P_P0N.x * P1N_P0N.y - P_P0N.y * P1N_P0N.x) / MagP1N_P0N;
			Dist = ABS(Dist);
			if(Dist <= Thickness)
				Frame.Pixels[y * Frame.Width + x] = Color;
		}
}

void RenderBackground(frame_buffer Frame, u32 Color){
	memset(Frame.Pixels, Color, Frame.Width * Frame.Height * sizeof(*Frame.Pixels));
}

void RenderCircle(frame_buffer Frame, v2s Center, s32 Radius, u32 Color){
	for(s32 y=Center.y-Radius; y<Center.y+Radius; y++)
		for(s32 x=Center.x-Radius; x<Center.x+Radius; x++)
			if((y-Center.y)*(y-Center.y) + (x-Center.x)*(x-Center.x) <= Radius*Radius)
				Frame.Pixels[y * Frame.Width + x] = Color;
}

void RenderIntersections(frame_buffer Frame, v2s P0, v2s P1, s32 GridCount){
	s32 GridWidth = Frame.Width / GridCount;
	s32 GridHeight = Frame.Height / GridCount;
	f32 Slope = (f32)(P1.y - P0.y) / (P1.x - P0.x);
	s32 Offset = P1.y - Slope * P1.x;

	if(P1.y > P0.y)
		for(f32 GridY = ((f32)P1.y/GridHeight); GridY > ((f32)P0.y/GridHeight); GridY--){
			s32 ClosestY = Floor(GridY)*GridHeight;
			s32 ClosestX = (ClosestY - Offset) / Slope;
			if(P0.y <= ClosestY && ClosestY <= P1.y)
				RenderCircle(Frame, (v2s){ ClosestX, ClosestY }, 4, 0x00FF0000);
		}
	else
		for(f32 GridY = ((f32)P1.y/GridHeight); GridY < ((f32)P0.y/GridHeight); GridY++){
			s32 ClosestY = Ceil(GridY)*GridHeight;
			s32 ClosestX = (ClosestY - Offset) / Slope;
			if(P1.y <= ClosestY && ClosestY <= P0.y)
				RenderCircle(Frame, (v2s){ ClosestX, ClosestY }, 4, 0x00FF0000);
		}

	if(P1.x > P0.x)
		for(f32 GridX = ((f32)P1.x/GridWidth); GridX > ((f32)P0.x/GridHeight); GridX--){
			s32 ClosestX = Floor(GridX)*GridHeight;
			s32 ClosestY = ClosestX * Slope + Offset;
			if(P0.x <= ClosestX && ClosestX <= P1.x)
				RenderCircle(Frame, (v2s){ ClosestX, ClosestY }, 4, 0x00FF0000);
		}
	else
		for(f32 GridX = ((f32)P1.x/GridWidth); GridX < ((f32)P0.x/GridHeight); GridX++){
			s32 ClosestX = Ceil(GridX)*GridHeight;
			s32 ClosestY = ClosestX * Slope + Offset;
			if(P1.x <= ClosestX && ClosestX <= P0.x)
				RenderCircle(Frame, (v2s){ ClosestX, ClosestY }, 4, 0x00FF0000);
		}
}

void RenderGrid(frame_buffer Frame, s32 GridCount){
	s32 GridWidth = Frame.Width/GridCount;
	s32 GridHeight = Frame.Height/GridCount;
	s32 LineThickness = 2;

	for(s32 x=0; x<GridCount; x++)
		RenderLine(Frame, (v2s){ x*GridWidth, 0 }, (v2s){ x*GridWidth, Frame.Height }, LineThickness, 0x00282828);
	
	for(s32 y=0; y<GridCount; y++)
		RenderLine(Frame, (v2s){ 0, y*GridHeight }, (v2s){ Frame.Width, y*GridHeight }, LineThickness, 0x00282828);
}

void RenderText(frame_buffer Frame, char *Text, u32 Length, v2s Pos, u32 Color){
	s32 OffsetY = Pos.y, OffsetX = Pos.x;

	for(u32 i = 0; i < Length; i++){
		char c = Text[i];
		for(s32 y = 0; y < GLYPH_HEIGHT; y++)
			for(s32 x = 0; x < GLYPH_WIDTH; x++){
				u8 Row = ((u8 *)BitmapFont)[c*GLYPH_SIZE + y];
				if(Row & (1 << x))
					Frame.Pixels[(Pos.y + y)*Frame.Width + (x + OffsetX)] = Color;
			}
		
		OffsetX += GLYPH_WIDTH + GLYPH_GAP;
	}

}

int main(){

	#define WINDOW_WIDTH 400
	#define WINDOW_HEIGHT 400
	u32 Buffer[ WINDOW_HEIGHT * WINDOW_WIDTH ];
	frame_buffer Frame = { .Width = WINDOW_HEIGHT, .Height = WINDOW_WIDTH, .Pixels = Buffer };

	InitWindow(Frame, "Raycast Engine");

	XEvent Event = {0};
	bool WindowShouldClose = false;
	v2s MousePos = {0, 0};
	v2s PlayerPos = {150, 130};
	u32 GridCount = 10;
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
				MousePos = (v2s){ Event.xbutton.x, Event.xbutton.y };
			}break;

			case Expose: DrawImage(); break;
			}
		}

		RenderBackground(Frame, 0x00181818);
		RenderGrid(Frame, GridCount);
		RenderLine(Frame, PlayerPos, MousePos, 2, 0x00FF00FF);
		RenderIntersections(Frame, PlayerPos, MousePos, GridCount);
		//RenderText(Frame, "Test Test", sizeof("Test Test")-1, (v2s){ 0, 0 }, 0x00FF00FF);

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
