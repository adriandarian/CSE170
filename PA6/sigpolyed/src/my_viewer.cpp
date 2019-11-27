#include "my_viewer.h"

#include <sigogl/ui_button.h>
#include <sigogl/ui_radio_button.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>

#include <sigogl/ws_run.h>
#include <vector>
#include <functional>

static void my_polyed_callback(SnPolyEditor *pe, enum SnPolyEditor::Event e, int pid)
{
	MyViewer *v = (MyViewer *)pe->userdata();
	if (e == SnPolyEditor::PostMovement || e == SnPolyEditor::PostEdition || e == SnPolyEditor::PostInsertion)
	{
		v->update_scene();
	}
}

MyViewer::MyViewer(int x, int y, int w, int h, const char *l) : WsViewer(x, y, w, h, l)
{
	rootg()->add(_polyed = new SnPolyEditor);
	rootg()->add(_curveA = new SnLines2);
	rootg()->add(_curveB = new SnLines2);
	rootg()->add(_curveC = new SnLines2);
	rootg()->add(_curveD = new SnLines2);

	_curveA->color(GsColor(20, 200, 25));
	_curveA->line_width(2.0f);

	// you may add new curves here
	_curveB->color(GsColor::magenta);
	_curveB->line_width(2.0f);

	_curveC->color(GsColor::orange);
	_curveC->line_width(2.0f);

	_curveD->color(GsColor::brown);
	_curveD->line_width(2.0f);

	// set initial control polygon:
	_polyed->callback(my_polyed_callback, this);
	_polyed->max_polygons(1);
	_polyed->solid_drawing(0);
	GsPolygon &P = _polyed->polygons()->push();
	P.setpoly("-2 -2  -1 1  1 0  2 -2");
	P.open(true);

	// start:
	build_ui();
	update_scene();
	message() = "Click on polygon to edit, use Esc to switch edition mode, Del deletes selected points. Enjoy!";
}

void MyViewer::build_ui()
{
	UiPanel *p;
	p = uim()->add_panel("", UiPanel::HorizLeft);

	p->add(_viewA = new UiCheckButton("CurveA", EvViewCurveA, true));
	p->add(_viewB = new UiCheckButton("CurveB", EvViewCurveB, true));
	p->add(_viewC = new UiCheckButton("CurveC", EvViewCurveC, true));
	p->add(_viewD = new UiCheckButton("CurveD", EvViewCurveD, true));

	p->add(_slider = new UiSlider(" dt:", EvDeltaT, 0, 0, 150));
	_slider->separate();
	_slider->range(0.01f, 0.25f);

	p->add(new UiButton("Exit", EvExit));
	p->top()->separate();
}

// Use one function for each curve type. Feel free to update/adapt parameter list etc.
static GsPnt2 eval_curveX(float t, const GsArray<GsPnt2> &P)
{
	GsPnt2 point = P[0] * (1.0f - t) + P.top() * t; // add the correct equations here

	return point;
}

static GsPnt2 eval_curveY(float t, const GsArray<GsPnt2> &P)
{
	GsPnt2 point;

	// point = ...

	return point;
}

/*! Beziar of order n for given n=P.size()-1 control points, t in [0,1] */
static GsPnt2 eval_bezier(float t, const GsArray<GsPnt2> &P)
{
	// Bezier functions
	std::vector<float> B = {
		gs_pow((1.0f - t), 3),
		gs_pow((1.0f - t), 2) * 3.0f *t,
		3.0f * (1.0f - t) * gs_pow(t, 2),
		gs_pow(t, 3)};

	// Bezier Formulation
	GsPnt2 point;
	int n = P.size() - 1;
	for (int i = 0; i <= n; i++)
	{
		point += P[i] * B[i];
	}
	return point;
}

/*! B-Spline order k, n=P.size()-1. For order k=3 (degree 2, quadratic case): t in [2,n+1] */
static GsPnt2 eval_bspline(float t, int k, const GsArray<GsPnt2> &P)
{
	// Recursive lambda function to calculate N
	std::function<float(int, int, float)> N = [&](int i, int k, float u) -> float {
		float intU = float(i);

		if (k == 1)
		{
			return (intU <= u && u < intU + 1) ? 1.0f : 0;
		}
		else
		{
			return ((u - intU) / (k - 1)) * N(i, k - 1, u) + ((intU + k - u) / (k - 1)) * N(i + 1, k - 1, u);
		}
	};

	// B-Spline Formulation
	GsPnt2 point;
	int n = P.size() - 1;
	for (int i = 0; i <= n; i++)
	{
		point += P[i] * N(i, 3, t);
	}
	return point;
}

/*! Evaluates a Catmull-Rom cubic spline, n=P.size()-1, t in [0,n-2] */
static GsPnt2 crspline(float t, const GsArray<GsPnt2> &P)
{
	int ti = (int)t;
	// auto I = [&](int i) -> GsPnt2 {
	// 	return (P[i + 1] - P[i - 1]) / 2.0f;
	// };

	GsArray<GsPnt2> Pi = GsArray<GsPnt2>(4);
	Pi[0] = P[ti + 1];
	Pi[1] = P[ti + 1] + GsPnt2((P[ti + 2] - P[ti]) / 2.0f) / 3.0f;
	Pi[2] = P[ti + 2] - GsPnt2((P[ti + 3] - P[ti + 1]) / 2.0f) / 3.0f;
	Pi[3] = P[ti + 2];

	return eval_bezier(t - ti, Pi);
}

/*! Evaluates a Bessel-Overhauser spline, n=P.size()-1, t in [0,n-2] */
static GsPnt2 bospline(float t, const GsArray<GsPnt2>& P)
{
	int ti = (int)t;
	GsPnt2 point;
	GsPnt2 ip = (P[ti + 3] - P[ti + 1]) / 2.0f;
	GsPnt2 im = (P[ti + 2] - P[ti]) / 2.0f;
	auto bosplinePos = [&](float t, const GsArray<GsPnt2>& P) -> GsPnt2 {
		int ti = (int)t;
		float dmm = (P[ti + 1] - P[ti]).len();
		float dmp = (P[ti + 2] - P[ti + 1]).len();
		GsPnt2 vmm = (P[ti + 1] - P[ti]) / dmm;
		GsPnt2 vmp = (P[ti + 2] - P[ti + 1]) / dmp;
		GsPnt2 vm = ((vmp * dmm) + (vmm * dmp)) / (dmm + dmp);
		return (vm * dmp);
	};
	auto bosplineNeg = [&](float t, const GsArray<GsPnt2>& P) -> GsPnt2 {
		int ti = int(t);
		float dpm = (P[ti + 2] - P[ti + 1]).len();
		float dpp = (P[ti + 3] - P[ti + 2]).len();
		GsPnt2 vpm = (P[ti + 2] - P[ti + 1]) / dpm;
		GsPnt2 vpp = (P[ti + 3] - P[ti + 2]) / dpp;
		GsPnt2 vp = ((vpp * dpm) + (vpm * dpp)) / (dpm + dpp);
		return (vp * dpm);
	};

	GsArray<GsPnt2> Pi = GsArray<GsPnt2>(4);
	Pi[0] = P[ti + 1];
	Pi[1] = P[ti + 1] + bosplinePos(t, P) / 3.0f;
	Pi[2] = P[ti + 2] - bosplineNeg(t, P) / 3.0f;
	Pi[3] = P[ti + 2];

	return eval_bezier(t - ti, Pi);
}

void MyViewer::update_scene()
{
	// Initializations:
	_curveA->init();
	_curveB->init();
	_curveC->init();
	_curveD->init();

	// Access the control polygon:
	GsPolygon &P = _polyed->polygon(0);
	float deltat = _slider->value();
	if (_viewA->value()) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		// As an example, below we have a linear interpolation between endpoints:
		_curveA->begin_polyline();
		for (float t = 0.0f; t < 1.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveA->push(eval_bezier(t, P));
		}
		// _curveA->push(P.top()); // ensure final point is there
		_curveA->end_polyline();
	}
	if (_viewB->value()) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		// As an example, below we have a linear interpolation between endpoints:
		_curveB->begin_polyline();
		for (float t = 2.0f; t < P.size(); t += deltat) // note: the t range may change according to the curve
		{
			_curveB->push(eval_bspline(t, 3, P));
		}
		// _curveB->push(P.top()); // ensure final point is there
		_curveB->end_polyline();
	}
	if (_viewC->value()) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		// As an example, below we have a linear interpolation between endpoints:
		_curveC->begin_polyline();
		for (float t = 0.0f; t < P.size() - 3.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveC->push(crspline(t, P));
		}
		_curveC->push(crspline(P.size() - 3.0f, P)); // ensure final point is there
		_curveC->end_polyline();
	}
	if (_viewD->value()) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		// As an example, below we have a linear interpolation between endpoints:
		_curveD->begin_polyline();
		for (float t = 0.0f; t < 1.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveD->push(bospline(t, P));
		}
		_curveD->push(bospline(P.size() - 3.0f, P)); // ensure final point is there
		_curveD->end_polyline();
	}
}

int MyViewer::uievent(int e)
{
	switch (e)
	{
	case EvViewCurveA:
	case EvViewCurveB:
	case EvViewCurveC:
	case EvViewCurveD:
	case EvDeltaT:
	{
		update_scene();
		return 1;
	}
	case GsEvent::KeyEsc:
	case EvExit:
	{
		gs_exit();
	}
	case GsEvent::KeySpace:
	{

		return 1;
	}
	}
	return WsViewer::uievent(e);
}
