
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

#include <cmath>
#include <vector>

# include <sigogl/ws_run.h>
int i = 1;

static void my_polyed_callback ( SnPolyEditor* pe, enum SnPolyEditor::Event e, int pid )
{
	MyViewer* v = (MyViewer*)pe->userdata();
	if ( e==SnPolyEditor::PostMovement || e==SnPolyEditor::PostEdition || e==SnPolyEditor::PostInsertion )
	{	v->update_scene ();
	}
}

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	rootg()->add ( _polyed = new SnPolyEditor );
	rootg()->add ( _curveA = new SnLines2 );
	rootg()->add( _curveB = new SnLines2 );
	rootg()->add(_curveC = new SnLines2);
	rootg()->add(_curveD = new SnLines2);

	_curveA->color ( GsColor(20,200,25) );
	_curveA->line_width ( 2.0f );

	// you may add new curves here

	_curveB->color(GsColor::magenta);
	_curveB->line_width(2.0f);

	_curveC->color(GsColor::blue);
	_curveC->line_width(2.0f);


	_curveD->color(GsColor::red);
	_curveD->line_width(2.0f);


	// set initial control polygon:
	_polyed->callback ( my_polyed_callback, this );
	_polyed->max_polygons (1);
	_polyed->solid_drawing (0);
	GsPolygon& P = _polyed->polygons()->push();
	P.setpoly ( "-2 -2  -1 1  1 0  2 -2" );
	P.open(true);

	// start:
	build_ui ();
	update_scene ();
	message() = "Click on polygon to edit, use Esc to switch edition mode, Del deletes selected points. Enjoy!";
}

void MyViewer::build_ui ()
{
	UiPanel *p;
	p = uim()->add_panel ( "", UiPanel::HorizLeft );

	p->add ( _viewA=new UiCheckButton ( "CurveA", EvViewCurveA, true ) );
	p->add ( _viewB=new UiCheckButton ( "CurveB", EvViewCurveB, true ) );
	p->add(_viewC = new UiCheckButton("CurveC", EvViewCurveC, true) );
	p->add(_viewC = new UiCheckButton("CurveD", EvViewCurveD, true));


	p->add ( _slider=new UiSlider ( " dt:", EvDeltaT, 0,0,150 ) );
	_slider->separate();
	_slider->range ( 0.01f, 0.25f );

	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

// Use one function for each curve type. Feel free to update/adapt parameter list etc.
static GsPnt2 eval_curveX ( float t, const GsArray<GsPnt2>& P )
{
	GsPnt2 point = P[0]*(1.0f-t) + P.top()*t; // add the correct equations here

	return point;
}

static GsPnt2 eval_curveY ( float t, const GsArray<GsPnt2>& P )
{
	GsPnt2 point;

	// point = ...

	return point;
}
static GsPnt2 eval_bezier(float t, const GsArray < GsPnt2> & P) {
	int n = P.size() - 1;
	GsPnt2 point;
	GsPnt2 result;

	for (int j = 0; j <= n; j++) {
		point = P[j] * ((gs_fact(n) / (gs_fact(j) * gs_fact(n - j)) * gs_pow(t, j) * gs_pow(1 - t, n - j)));
		result += point;

	}
	return result;
}

static float numN(int i, int k, float t) { // k always = 3
	//Code taken from bSpline Lecture slides
	float ti = float(i);

	if (k == 1) {
		return ti <= t && t < ti + 1 ? 1.0f : 0;
	}
	else {
		return ((t - ti) / (k - 1)) * numN(i, k - 1, t) +
			((ti + k - t) / (k - 1)) * numN(i + 1, k - 1, t);
	}
}

static GsPnt2 eval_bSpline(float t, int k, const GsArray<GsPnt2>& P) {
	GsPnt2 curvePoint;

	for (int i = 0; i < P.size(); i++) {
		curvePoint += (P[i] * numN(i, 3, t));
	}

	return curvePoint;
}
static GsPnt2 crspline(float t, const GsArray <GsPnt2>& P) {
	int ti = (int)t;
	GsPnt2 ip = (P[ti + 3] - P[ti + 1]) / 2.0f;
	GsPnt2 im = (P[ti + 2] - P[ti]) / 2.0f;

	GsArray<GsPnt2> Pi =GsArray<GsPnt2>(4);
	Pi[0] = P[ti + 1];
	Pi[1] = P[ti + 1] + im / 3.0f;
	Pi[2] = P[ti + 2] - ip / 3.0f;
	Pi[3] = P[ti + 2];

	return eval_bezier(t - ti, Pi);

}

static GsPnt2 function1(float t, const GsArray<GsPnt2>& P) {
	int ti = (int)t;
	float dmm = (P[ti + 1] - P[ti]).len();
	float dmp = (P[ti + 2] - P[ti + 1]).len();
	GsPnt2 vmm = (P[ti + 1] - P[ti]) / dmm;
	GsPnt2 vmp = (P[ti + 2] - P[ti + 1]) / dmp;
	GsPnt2 vm = ((vmp * dmm) + (vmm * dmp)) / (dmm + dmp);
	return (vm * dmp);
}

static GsPnt2 function2(float t, const GsArray<GsPnt2>& P) {
	int ti = int(t);
	float dpm = (P[ti + 2] - P[ti + 1]).len();
	float dpp = (P[ti + 3] - P[ti + 2]).len();
	GsPnt2 vpm = (P[ti + 2] - P[ti + 1]) / dpm;
	GsPnt2 vpp = (P[ti + 3] - P[ti + 2]) / dpp;
	GsPnt2 vp = ((vpp * dpm) + (vpm * dpp)) / (dpm + dpp);
	return (vp * dpm);
}


static GsPnt2 bospline(float t, const GsArray<GsPnt2>& P)
{
	GsPnt2 point;
	int ti = (int)t;

	GsPnt2 ip = (P[ti + 3] - P[ti + 1]) / 2.0f;
	GsPnt2 im = (P[ti + 2] - P[ti]) / 2.0f;

	GsArray<GsPnt2> Pi = GsArray<GsPnt2>(4);
	Pi[0] = P[ti + 1];
	Pi[1] = P[ti + 1] + function1(t, P) / 3.0f;
	Pi[2] = P[ti + 2] - function2(t, P) / 3.0f;
	Pi[3] = P[ti + 2];

	return eval_bezier(t - ti, Pi);

}

void MyViewer::update_scene ()
{
	// Initializations:
	_curveA->init();
	_curveB->init();
	_curveC->init();
	_curveD->init();


	// Access the control polygon:
	GsPolygon& P = _polyed->polygon(0);
	float deltat = _slider->value();
	if ( _viewA->value() ) // show curve
	{
		_curveA->begin_polyline();
		for (float t = 0.0f; t < 1.0f; t += deltat)
		{
			_curveA->push(eval_bezier(t, P));
		}
		_curveA->end_polyline();
	}
	if (_viewB->value())
	{
		_curveB->begin_polyline();
		for (float t = 2.0f; t < P.size(); t += deltat) 
		{
			_curveB->push(eval_bSpline(t, 3, P));
		}
		_curveB->end_polyline();
	}
	if (_viewC->value())
	{
		_curveC->begin_polyline();
		for(float t= 0.0f; t<P.size()-3.0f;t += deltat)
		{
			_curveC->push(crspline(t, P));
		}
		_curveC->push(crspline(P.size()-3.0f, P));
		_curveC->end_polyline();

	}
	if (_viewD->value())
	{
		_curveD->begin_polyline();
		for (float t = 0.0f; t < 1.0F; t += deltat)
		{
			_curveD->push(bospline(t, P));
		}
		_curveD->push(bospline(P.size() - 3.0f, P));
		_curveD->end_polyline();

	}

}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{
		case EvViewCurveA:
		case EvViewCurveB:
		case EvViewCurveC:
		case EvViewCurveD:
		case EvDeltaT: update_scene(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
