#include "my_viewer.h"

#include <sigogl/ui_button.h>
#include <sigogl/ui_radio_button.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>

#include <sigogl/ws_run.h>

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

	_curveA->color(GsColor(20, 200, 25));
	_curveA->line_width(2.0f);

	// you may add new curves here

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

void MyViewer::update_scene()
{
	// Initializations:
	_curveA->init();

	// Access the control polygon:
	GsPolygon &P = _polyed->polygon(0);
	float deltat = _slider->value();
	if (_viewA->value()) // show curve
	{
		// Add your curves below and select the correct one(s) to be displayed.
		// As an example, below we have a linear interpolation between endpoints:
		_curveA->begin_polyline();
		for (float t = 0; t < 1.0f; t += deltat) // note: the t range may change according to the curve
		{
			_curveA->push(eval_curveX(t, P));
		}
		_curveA->push(P.top()); // ensure final point is there
		_curveA->end_polyline();
	}
}

int MyViewer::uievent(int e)
{
	switch (e)
	{
	case EvViewCurveA:
	case EvViewCurveB:
		update_scene();
		return 1;
	case EvDeltaT:
		update_scene();
		return 1;
	case EvExit:
		gs_exit();
	}
	return WsViewer::uievent(e);
}
