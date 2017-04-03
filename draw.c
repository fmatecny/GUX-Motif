/*
 * draw.c - projekt 1 do predmetu GUX
 * autor: Frantisek Matecny, xmatec00
 */

/*
 * Standard XToolkit and OSF/Motif include files.
 */
#include <X11/Intrinsic.h>
#include <Xm/Xm.h> 

/*
 * Public include files for widgets used in this file.
 */
#include <Xm/MainW.h> 
#include <Xm/Form.h> 
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <Xm/Protocols.h>
#include <X11/Xmu/Editres.h>
#include <Xm/MessageB.h>

/*
 * Common C library include files
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * Shared variables
 */
#define DRAW_ALLOC_STEP	10	/* memory allocation stepping */
int maxdraw = 0;		/* space allocated for max drawing object */
int ndraw = 0;			/* current number of drawing object  */

GC inputGC = 0;			/* GC used for drawing current position */

int x1, y1, x2, y2;		/* input points */ 
int cur_x,cur_y;
int button_pressed = 0;		/* input state */

/* colors */
XColor black, white, red, green, blue;
Pixel lineBg_pixel, lineFg_pixel, fillBg_pixel, fillFg_pixel, last_pixel, drawAreaBg;

/* drawing objekt style */
int line_width = 0;
int line_style = LineSolid;
bool transparent = true;
bool solid = true;
unsigned int width, height;

/* type drawing object */
typedef enum t_mode {POINT, LINE, SQUARE, CIRCLE} t_mode;
t_mode mode = POINT;

/* struct for save drawing object */
struct draw_struct {
	
	GC gc;
	t_mode mode;
	int x1;
	int x2;
	int y1;
	int y2;
	Pixel fillFg_pixel;
	bool transparent;	
};
struct draw_struct *draw = NULL;


/*
 * Set rectangle coordinates
 */
void setCoordinates(int sur_x1, int sur_x2, int sur_y1, int sur_y2)
{	
    if (sur_x1 < sur_x2) {
	width = sur_x2 - sur_x1;
	cur_x = sur_x1;
    } else {
	width = sur_x1 - sur_x2;
	cur_x = sur_x2;
    }
    
    if (sur_y1 < sur_y2) {
	height = sur_y2 - sur_y1;
	cur_y = sur_y1;
    } else {
	height = sur_y1 - sur_y2;
	cur_y = sur_y2;
    }  
}


/*
 * draw point
 */
void XmyDrawPoint (Display *display, Drawable d, GC gc, int x, int y, int lineW)
{	
	if (lineW == 0){
		XDrawPoint(display, d, gc, x, y);}
	else{
		XFillArc(display, d, gc,  x-(lineW/2), y-(lineW/2), lineW, lineW, 0, 360*64);}
}


/*
 * save drawing object
 */
void saveDraw (Widget w)
{
	if (++ndraw > maxdraw) {
		maxdraw += DRAW_ALLOC_STEP;
		draw = (struct draw_struct*) realloc(draw, sizeof(struct draw_struct) * maxdraw);
		if (draw == NULL){
			fprintf(stderr, "Error (re)allocating memory\n");}
	}

	/* press without moving */
	if (!inputGC) {
		Pixel fg, bg;
	    inputGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, NULL);
	    XSetFunction(XtDisplay(w), inputGC, GXxor);
	    XSetPlaneMask(XtDisplay(w), inputGC, ~0);
	    XtVaGetValues(w, XmNforeground, &fg, XmNbackground, &bg, NULL);
	    XSetForeground(XtDisplay(w), inputGC, fg ^ bg);
	}

	draw[ndraw - 1].gc = XCreateGC(XtDisplay(w), XtWindow(w), 0, NULL);
	XSetForeground(XtDisplay(w), draw[ndraw - 1].gc, lineFg_pixel);
	XSetBackground(XtDisplay(w), draw[ndraw - 1].gc, lineBg_pixel);
	XCopyGC(XtDisplay(w), inputGC, GCLineWidth | GCLineStyle, draw[ndraw - 1].gc);
	draw[ndraw - 1].mode = mode;
	draw[ndraw - 1].x1 = x1;
	draw[ndraw - 1].y1 = y1;
	draw[ndraw - 1].x2 = x2;
	draw[ndraw - 1].y2 = y2;
	draw[ndraw - 1].fillFg_pixel = fillFg_pixel;
	draw[ndraw - 1].transparent = transparent;	
}


/*
 * "InputObject" event handler
 */
void InputObjectEH(Widget w, XtPointer client_data, XEvent *event, Boolean *cont)
{
    Pixel fg, bg;

    if (button_pressed) {
	if (!inputGC) {
	    inputGC = XCreateGC(XtDisplay(w), XtWindow(w), 0, NULL);
	    XSetFunction(XtDisplay(w), inputGC, GXxor);
	    XSetPlaneMask(XtDisplay(w), inputGC, ~0);
	    XtVaGetValues(w, XmNforeground, &fg, XmNbackground, &bg, NULL);
	    XSetForeground(XtDisplay(w), inputGC, fg ^ bg);
	}
	
	XSetBackground(XtDisplay(w), inputGC, drawAreaBg ^ lineBg_pixel);
	XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ lineFg_pixel);
	
	if (line_width == 0){
		XSetLineAttributes(XtDisplay(w), inputGC, line_width, LineSolid, CapButt, JoinMiter);}
	else{
	XSetLineAttributes(XtDisplay(w), inputGC, line_width, line_style, CapButt, JoinMiter);}

	if (button_pressed > 1) {
	    /* erase previous position */
		switch(mode)
		{	
			case POINT: break;		
			case LINE:  XDrawLine(XtDisplay(w), XtWindow(w), inputGC, x1, y1, x2, y2);
						break;
			
			case SQUARE: setCoordinates(x1, x2, y1, y2);
						 if(!transparent){
							XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ fillFg_pixel);
							XFillRectangle(XtDisplay(w), XtWindow(w), inputGC, cur_x, cur_y, width, height);
							XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ lineFg_pixel);
						 }
						 XDrawRectangle(XtDisplay(w), XtWindow(w), inputGC, cur_x, cur_y, width, height);
						 break;
			
			case CIRCLE: setCoordinates(x1, x2, y1, y2);
						 if(!transparent){
							XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ fillFg_pixel);
							XFillArc(XtDisplay(w), XtWindow(w), inputGC, x1 - width, y1 - height, width*2, height*2, 0, 360*64);
							XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ lineFg_pixel);
						 }	 
						 XDrawArc(XtDisplay(w), XtWindow(w), inputGC, x1 - width, y1 - height, width*2, height*2, 0, 360*64);
						 break;
			
		}
	} else {
	    /* remember first MotionNotify */
	    button_pressed = 2;
	}

	x2 = event->xmotion.x;
	y2 = event->xmotion.y;

	switch(mode)
	{
		case POINT: saveDraw(w); /* save drawing object */
					XmyDrawPoint(XtDisplay(w), XtWindow(w), inputGC, x2, y2, line_width);
					break;
		
		case LINE: XDrawLine(XtDisplay(w), XtWindow(w), inputGC, x1, y1, x2, y2);
				   break;
		
		case SQUARE: setCoordinates(x1, x2, y1, y2);
					 if(!transparent){
						XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ fillFg_pixel);
						XFillRectangle(XtDisplay(w), XtWindow(w), inputGC, cur_x, cur_y, width, height);
						XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ lineFg_pixel);
					 }
					 XDrawRectangle(XtDisplay(w), XtWindow(w), inputGC, cur_x, cur_y, width, height);			
					 break;
		
		case CIRCLE: setCoordinates(x1, x2, y1, y2);
					 if(!transparent){
						XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ fillFg_pixel);
						XFillArc(XtDisplay(w), XtWindow(w), inputGC, x1 - width, y1 - height, width*2, height*2, 0, 360*64);
						XSetForeground(XtDisplay(w), inputGC, drawAreaBg ^ lineFg_pixel);
					 } 
					 XDrawArc(XtDisplay(w), XtWindow(w), inputGC, x1 - width, y1 - height, width*2, height*2, 0, 360*64);
					 break;		
	}
	}		
}


/*
 * "DrawObject" callback function
 */
void DrawObjectCB(Widget w, XtPointer client_data, XtPointer call_data)
{
    XmDrawingAreaCallbackStruct *d = (XmDrawingAreaCallbackStruct*) call_data;

    switch (d->event->type) 
    {
		case ButtonPress:
			if (d->event->xbutton.button == Button1) {
			button_pressed = 1;
			x1 = d->event->xbutton.x;
			y1 = d->event->xbutton.y;
			}
			break;

		case ButtonRelease:
			if (d->event->xbutton.button == Button1) {

			button_pressed = 0;
			
			x2 = d->event->xbutton.x;
			y2 = d->event->xbutton.y;
			
			/* save drawing object */
			saveDraw(w);	
				
			/* call up expose */
			XClearArea(XtDisplay(w), XtWindow(w), 0, 0, 0, 0, True);
			}	
			break;
    }
}

/*
 * "Expose" callback function
 */
void ExposeCB(Widget w, XtPointer client_data, XtPointer call_data)
{
	XGCValues val;
    if (ndraw <= 0)
	return;
	
	for (int i = 0; i < ndraw; i++){
					
		XGetGCValues(XtDisplay(w), draw[i].gc, GCForeground|GCLineWidth, &val);
		
		switch(draw[i].mode)
		{
			case POINT: XmyDrawPoint(XtDisplay(w), XtWindow(w), draw[i].gc, draw[i].x2, draw[i].y2, val.line_width);
						break;
			
			case LINE: XDrawLine(XtDisplay(w), XtWindow(w), draw[i].gc, draw[i].x1, draw[i].y1, draw[i].x2, draw[i].y2);	
					   break;
			
			case SQUARE: setCoordinates(draw[i].x1, draw[i].x2, draw[i].y1, draw[i].y2);
						 if(!draw[i].transparent){
							XSetForeground(XtDisplay(w), draw[i].gc, draw[i].fillFg_pixel);
							XFillRectangle(XtDisplay(w), XtWindow(w), draw[i].gc, cur_x, cur_y, width, height);
							XSetForeground(XtDisplay(w), draw[i].gc, val.foreground);}
						 XDrawRectangle(XtDisplay(w), XtWindow(w), draw[i].gc, cur_x, cur_y, width, height);								
						 break;
			
			case CIRCLE: setCoordinates(draw[i].x1, draw[i].x2, draw[i].y1, draw[i].y2);
						 if(!draw[i].transparent){
							XSetForeground(XtDisplay(w), draw[i].gc, draw[i].fillFg_pixel);
							XFillArc(XtDisplay(w), XtWindow(w), draw[i].gc, 
										draw[i].x1 - width, draw[i].y1 - height, 
										width*2, height*2, 0, 360*64);
							XSetForeground(XtDisplay(w), draw[i].gc, val.foreground);
						 }
						 XDrawArc(XtDisplay(w), XtWindow(w), draw[i].gc,
									draw[i].x1 - width, draw[i].y1 - height, 
									width*2, height*2, 0, 360*64);
						break;			
		}	
	}
}

/*
 * Option menu - set drawing mode
 */ 
void setDrawMode (Widget w, XtPointer client_data, XtPointer call_data)
{
	mode = (int) client_data;
    
    /*switch(mode)
    {
		case 0: printf("Point\n");
				break;
		case 1: printf("Line\n");
				break;
		case 2: printf("Square\n");
				break;
		case 3: printf("Elipse\n");
				break;
	}*/
}

/*
 * Option menu - set line width
 */ 
void setWidth (Widget w, XtPointer client_data, XtPointer call_data)
{
    
    switch((int) client_data)
    {	
		case 0: line_width = 0;
				//printf("Width: 0\n");
				break;
				
		case 1: line_width = 3;
				//printf("Width: 3\n");
				break;
				
		case 2: line_width = 8;
				//printf("Width: 8\n");
				break;
	}
}


/*
 * return next color
 */
Pixel getNextPixelColor()
{	
	if (last_pixel == black.pixel){
		return red.pixel;
	}
	else if (last_pixel == red.pixel){
		return green.pixel;
	}
	else if (last_pixel == green.pixel){
		return blue.pixel;
	}
	else if (last_pixel == blue.pixel){
		return white.pixel;
	}
	
	return black.pixel;	
}


/*
 * Set foreground color
 */
void setLineFg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtVaGetValues(client_data, XmNforeground, &last_pixel, NULL);	
	lineFg_pixel = getNextPixelColor();
	XtVaSetValues(w, XtNforeground, lineFg_pixel, NULL);
}

/*
 * Set background color
 */
void setLineBg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtVaGetValues(client_data, XmNforeground, &last_pixel, NULL);
	lineBg_pixel = getNextPixelColor();
	XtVaSetValues(w, XtNforeground, lineBg_pixel, NULL);
}

/*
 * Set fill foreground color
 */
void setFilFg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtVaGetValues(client_data, XmNforeground, &last_pixel, NULL);
	fillFg_pixel = getNextPixelColor();
	XtVaSetValues(w, XtNforeground, fillFg_pixel, NULL);
}

/*
 * Set fill background color
 */
void setFilBg(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtVaGetValues(client_data, XmNforeground, &last_pixel, NULL);
	fillBg_pixel = getNextPixelColor();
	XtVaSetValues(w, XtNforeground, fillBg_pixel, NULL);
}


/*
 * "Clear" button callback function
 */
void ClearCB(Widget w, XtPointer client_data, XtPointer call_data)
{ 
    Widget wcd = (Widget) client_data;

    ndraw = 0;
    XClearWindow(XtDisplay(wcd), XtWindow(wcd));
}

/*
 * "Quit" button callback function
 */
void questionCB(Widget w, XtPointer client_data, XtPointer call_data)
{
  switch ((int)client_data)
  {
    case 0: /* ok */
			/* free allocated memory */
			for (int i = 0; i < ndraw; i++)
			{
				XFreeGC(XtDisplay(w), draw[i].gc);
			}
			exit(0);
			break;
    case 1: /* cancel */
      break;
  }
}

/*
 * "Quit" button callback function
 */
void quitCB(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild(client_data);
}



/*
 * set transparent/filled mode
 */ 
void setTransparent (Widget w, XtPointer client_data, XtPointer call_data)
{
	XmString str;

	if (transparent){
		str = XmStringCreateLocalized("  Filled   ");
		transparent = false;}
	else{
		str = XmStringCreateLocalized("Transparent");
		transparent = true;}	
   
	XtVaSetValues(w, XmNlabelString, str, NULL);

	XmStringFree(str);
}


/*
 * set line style
 */ 
void setLineStyle (Widget w, XtPointer client_data, XtPointer call_data)
{
	XmString str;

	if (solid){
		line_style = LineDoubleDash;
		str = XmStringCreateLocalized("Double Dash");
		solid = false;
	}
	else{
		line_style = LineSolid;
		str = XmStringCreateLocalized("   Solid   ");
		solid = true;
	}	
   
	XtVaSetValues(w, XmNlabelString, str, NULL);

	XmStringFree(str);
}


/*
 * Main function
 */
int main(int argc, char **argv)
{

	char *fall[] = {
    "*question.dialogTitle: Malá otázka",
    "*question.messageString: Konec aplikace?",
    "*question.okLabelString: Ano",
    "*question.cancelLabelString: Ne",
    "*question.messageAlignment: XmALIGNMENT_CENTER",
    NULL
  };	

	Atom wm_delete;

    XtAppContext app_context;
    Widget topLevel, mainWin, frame, drawArea, 
			rowColumnArea, drawModes, colorModes, styleModes, buttonsArea, 
			option_menu1, option_menu2,
			lineBgBtn, lineFgBtn, fillBgBtn, fillFgBtn,
			transparentBtn, lineDashBtn,
			quitBtn, clearBtn, question;;

    //Register the default language procedure
    XtSetLanguageProc(NULL, (XtLanguageProc)NULL, NULL);

    topLevel = XtVaAppInitialize(
      &app_context,		 	/* Application context */
      "Draw",				/* Application class */
      NULL, 0,				/* command line option list */
      &argc, argv,			/* command line args */
      fall,
	  XmNheight, (int)400,
      XmNwidth, (int)400,
      XmNdeleteResponse, XmDO_NOTHING,
      NULL);

    mainWin = XtVaCreateManagedWidget(
      "mainWin",			/* widget name */
      xmMainWindowWidgetClass,		/* widget class */
      topLevel,				/* parent widget*/
      XmNcommandWindowLocation, XmCOMMAND_BELOW_WORKSPACE,
      NULL);				/* terminate varargs list */

    frame = XtVaCreateManagedWidget(
      "frame",				/* widget name */
      xmFrameWidgetClass,		/* widget class */
      mainWin,				/* parent widget */
      NULL);				/* terminate varargs list */

    drawArea = XtVaCreateManagedWidget(
      "drawingArea",			/* widget name */
      xmDrawingAreaWidgetClass,		/* widget class */
      frame,				/* parent widget*/
      XmNwidth, 200,			/* set startup width */
      XmNheight, 100,			/* set startup height */
      NULL);				/* terminate varargs list */

/*
    XSelectInput(XtDisplay(drawArea), XtWindow(drawArea), 
      KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask
      | Button1MotionMask );
*/

//--------------------- rowColumnArea --------------------------

    rowColumnArea = XtVaCreateManagedWidget(
      "rowColumnArea",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      mainWin,				/* parent widget */
      NULL);
       
    drawModes = XtVaCreateManagedWidget(
      "drawModes",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      rowColumnArea,				/* parent widget */
      XmNentryAlignment, XmALIGNMENT_CENTER,	/* alignment */
      XmNorientation, XmHORIZONTAL,	/* orientation */
      XmNpacking, XmPACK_COLUMN,	/* packing mode */
      NULL);				/* terminate varargs list */
        
    colorModes = XtVaCreateManagedWidget(
      "colorModes",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      rowColumnArea,				/* parent widget */
      XmNentryAlignment, XmALIGNMENT_CENTER,	/* alignment */
      XmNorientation, XmHORIZONTAL,	/* orientation */
      XmNpacking, XmPACK_COLUMN,	/* packing mode */
      NULL);

    styleModes = XtVaCreateManagedWidget(
      "styleModes",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      rowColumnArea,				/* parent widget */
      XmNentryAlignment, XmALIGNMENT_CENTER,	/* alignment */
      XmNorientation, XmHORIZONTAL,	/* orientation */
      XmNpacking, XmPACK_COLUMN,	/* packing mode */
      NULL);
        
    buttonsArea = XtVaCreateManagedWidget(
      "buttonsArea",			/* widget name */
      xmRowColumnWidgetClass,		/* widget class */
      rowColumnArea,				/* parent widget */
      XmNentryAlignment, XmALIGNMENT_CENTER,	/* alignment */
      XmNorientation, XmHORIZONTAL,	/* orientation */
      XmNpacking, XmPACK_COLUMN,	/* packing mode */
      NULL);

//--------------------- drawModes ----------------------------

/* option menu1 */
	XmString label, point, line, square, circle;

    label = XmStringCreateLocalized ("Draw Mode:");
    point = XmStringCreateLocalized ("Point");
    line = XmStringCreateLocalized ("Line");
    square = XmStringCreateLocalized ("Square");
    circle = XmStringCreateLocalized ("Circle");


    option_menu1 = XmVaCreateSimpleOptionMenu (drawModes,
                                "option_menu", label, 'D', 
                                0 /*initial menu selection*/, setDrawMode,
                                XmVaPUSHBUTTON, point, 'P', NULL, NULL,
                                XmVaPUSHBUTTON, line, 'L', NULL, NULL,
                                XmVaPUSHBUTTON, square, 'S', NULL, NULL,
                                XmVaPUSHBUTTON, circle, 'C', NULL, NULL,
                                NULL);
	XmStringFree (point);
	XmStringFree (line);
    XmStringFree (square);
    XmStringFree (circle);
    XmStringFree (label);
    XtManageChild (option_menu1);

/* option menu2 */
	XmString width0, width3, width8;

    label = XmStringCreateLocalized ("Line Width:");
    width0 = XmStringCreateLocalized ("  0  ");
    width3 = XmStringCreateLocalized ("  3  ");
    width8 = XmStringCreateLocalized ("  8  ");


    option_menu2 = XmVaCreateSimpleOptionMenu (drawModes,
                                "option_menu", label, 'D', 
                                0 /*initial menu selection*/, setWidth,
                                XmVaPUSHBUTTON, width0, 'L', NULL, NULL,
                                XmVaPUSHBUTTON, width3, 'S', NULL, NULL,
                                XmVaPUSHBUTTON, width8, 'C', NULL, NULL,
                                NULL);
	XmStringFree (width0);
    XmStringFree (width3);
    XmStringFree (width8);
    XmStringFree (label);
    XtManageChild (option_menu2);

//------------------ colorModes  ------------------

    lineFgBtn = XtVaCreateManagedWidget(
      "LineFg",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      colorModes,			/* parent widget*/
      NULL);				/* terminate varargs list */

    lineBgBtn = XtVaCreateManagedWidget(
      "LineBg",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      colorModes,			/* parent widget*/
      NULL);				/* terminate varargs list */

    fillFgBtn = XtVaCreateManagedWidget(
      "FillFg",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      colorModes,			/* parent widget*/
      NULL);				/* terminate varargs list */

    fillBgBtn = XtVaCreateManagedWidget(
      "FillBg",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      colorModes,			/* parent widget*/
      NULL);				/* terminate varargs list */


//--------------- styleModes -----------------------

    transparentBtn = XtVaCreateManagedWidget(
      "Transparent",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      styleModes,			/* parent widget*/
      NULL);				/* terminate varargs list */
      
    lineDashBtn = XtVaCreateManagedWidget(
      "   Solid   ",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      styleModes,			/* parent widget*/
      NULL);				/* terminate varargs list */

//----------------- buttonsArea -------------------------

    clearBtn = XtVaCreateManagedWidget(
      "Clear",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      buttonsArea,			/* parent widget*/
      NULL);				/* terminate varargs list */

    quitBtn = XtVaCreateManagedWidget(
      "Quit",				/* widget name */
      xmPushButtonWidgetClass,		/* widget class */
      buttonsArea,			/* parent widget*/
      NULL);				/* terminate varargs list */
//-----------------------------------------------------------

    XmMainWindowSetAreas(mainWin, NULL, rowColumnArea, NULL, NULL, frame);

	/* alloc color */
	Colormap cmap = DefaultColormap(XtDisplay(drawArea), DefaultScreen(XtDisplay(drawArea)));
	if (!XAllocNamedColor(XtDisplay(drawArea), cmap, "black", &black, &black) ||
		!XAllocNamedColor(XtDisplay(drawArea), cmap, "white", &white, &white) ||
		!XAllocNamedColor(XtDisplay(drawArea), cmap, "red", &red, &red) ||
		!XAllocNamedColor(XtDisplay(drawArea), cmap, "green", &green, &green) ||
		!XAllocNamedColor(XtDisplay(drawArea), cmap, "blue", &blue, &blue)) 
		{
		fprintf(stderr, "Can't alloc colors!\n");
		return 1;
		}
		
	/* farba pozadia dravArea */
	XtVaGetValues(drawArea, XmNbackground, &drawAreaBg, NULL);

	/* set button colors */
	lineFg_pixel = red.pixel;
	lineBg_pixel = green.pixel;
	fillFg_pixel = blue.pixel;
	XtVaSetValues(lineFgBtn, XtNforeground, lineFg_pixel, NULL);
	XtVaSetValues(lineBgBtn, XtNforeground, lineBg_pixel, NULL);
	XtVaSetValues(fillFgBtn, XtNforeground, fillFg_pixel, NULL);


	/* Callback functions */
	XtAddCallback(lineFgBtn, XmNactivateCallback, setLineFg, lineFgBtn);	
	XtAddCallback(lineBgBtn, XmNactivateCallback, setLineBg, lineBgBtn);
	XtAddCallback(fillFgBtn, XmNactivateCallback, setFilFg, fillFgBtn);
	XtAddCallback(fillBgBtn, XmNactivateCallback, setFilBg, fillBgBtn);
	
	XtAddCallback(transparentBtn, XmNactivateCallback, setTransparent, transparentBtn);
	XtAddCallback(lineDashBtn, XmNactivateCallback, setLineStyle, lineDashBtn);

    XtAddCallback(drawArea, XmNinputCallback, DrawObjectCB, drawArea);
    XtAddEventHandler(drawArea, ButtonMotionMask, False, InputObjectEH, NULL);
    XtAddCallback(drawArea, XmNexposeCallback, ExposeCB, drawArea);
    

	/* quit message */
	question = XmCreateQuestionDialog(topLevel, "question", NULL, 0);
	XtVaSetValues(question,	XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL, NULL);
	XtUnmanageChild(XmMessageBoxGetChild(question, XmDIALOG_HELP_BUTTON));
	XtAddCallback(question, XmNokCallback, questionCB, (XtPointer)0);
	XtAddCallback(question, XmNcancelCallback, questionCB, (XtPointer)1);

	wm_delete = XInternAtom(XtDisplay(topLevel), "WM_DELETE_WINDOW", False);
	XmAddWMProtocolCallback(topLevel, wm_delete, quitCB, question);
	XmActivateWMProtocol(topLevel, wm_delete);
	
	
	XtAddCallback(clearBtn, XmNactivateCallback, ClearCB, drawArea);
	XtAddCallback(quitBtn, XmNactivateCallback, quitCB, question);

	/* volitelne pro editres, musi byt doplneno pro sestaveni -lXmu */
	XtAddEventHandler(topLevel, 0, True, _XEditResCheckMessages, NULL);

    XtRealizeWidget(topLevel);

    XtAppMainLoop(app_context);

    return 0;
}
