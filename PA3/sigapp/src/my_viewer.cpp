#include <vector>
#include <string>
#include "my_viewer.h"

#include <sigogl/ui_button.h>
#include <sigogl/ui_radio_button.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>

#include <sigogl/ws_run.h>

MyViewer::MyViewer(int x, int y, int w, int h, const char *l) : WsViewer(x, y, w, h, l)
{
	_nbut = 0;
	_animating = false;
	build_ui();
	build_scene();
}

void MyViewer::build_ui()
{
	UiPanel *p, *sp;
	UiManager *uim = WsWindow::uim();
	p = uim->add_panel("", UiPanel::HorizLeft);
	p->add(new UiButton("View", sp = new UiPanel()));
	{
		UiPanel *p = sp;
		p->add(_nbut = new UiCheckButton("Normals", EvNormals));
	}
	p->add(new UiButton("Animate", EvAnimate));
	p->add(new UiButton("Exit", EvExit));
	p->top()->separate();
}

void MyViewer::add_model(SnShape *s, GsVec p)
{
	// This method demonstrates how to add some elements to our scene graph: lines,
	// and a shape, and all in a group under a SnManipulator.
	// Therefore we are also demonstrating the use of a manipulator to allow the user to
	// change the position of the object with the mouse. If you do not need mouse interaction,
	// you can just use a SnTransform to apply a transformation instead of a SnManipulator.
	// You would then add the transform as 1st element of the group, and set g->separator(true).
	// Your scene graph should always be carefully designed according to your application needs.

	SnManipulator *manip = new SnManipulator;
	GsMat m;
	m.translation(p);
	manip->initial_mat(m);

	SnGroup *g = new SnGroup;
	SnLines *l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);
	// manip->visible(false); // call this to turn off mouse interaction

	rootg()->add(manip);
}

void MyViewer::build_scene()
{
	g = new SnGroup; // to hold all the models and transformation
	b = {b0, b1, b2};
	pieces = {"rupperarm.m", "rlowerarm.m", "rhand.m"};

	for (int i = 0; i < 3; i++)
	{
		model[i] = new SnModel;
		t[i] = new SnTransform;

		if (!model[i]->model()->load(("../../arm/" + pieces[i]).c_str()))
		{
			gsout << (pieces[i] + " was not loaded").c_str() << gsnl;
		}

		// model[i]->model()->centralize();
		model[i]->model()->get_bounding_box(b0);
		if (i == 0)
		{
			float x = 0.0f;
			float y = 0.0f;
			float z = -(b[i].dz() / 2.0f) + 24.0f;
			m[i].translation(GsVec(x, y, z));
			gsout << z << gsnl;
			shoulderOrigin = z;
		}
		else if (i == 1)
		{
			float x = 0.0f;
			float y = 0.0f;
			float z = b[i].dz() / 2.0f;
			m[i].translation(GsVec(x, y, z));
			gsout << z << gsnl;
			elbowOrigin = z;
		}
		else if (i == 2)
		{
			float x = 0.0f;
			float y = 0.0f;
			float z = -((b[1].dz() / 2.0f) + b2.dz());
			m[i].translation(GsVec(x, y, z));
			gsout << z << gsnl;
			handOrigin = z;
		}
		t[i]->set(m[0]);

		g->add(t[i]);
		g->add(model[i]);
	}

	rootg()->add(g);
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation()
{
	if (_animating)
		return; // avoid recursive calls
	_animating = true;

	int ind = gs_random(0, rootg()->size() - 1);			 // pick one child
	SnManipulator *manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
	double v = 4;			  // target velocity is 1 unit per second
	double t = 0, lt = 0, t0 = gs_time();
	do // run for a while:
	{
		while (t - lt < frdt)
		{
			ws_check();
			t = gs_time() - t0;
		} // wait until it is time for next frame
		double yinc = (t - lt) * v;
		if (t > 2)
			yinc = -yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if (m.e24 < 0)
			m.e24 = 0; // make sure it does not go below 0
		manip->initial_mat(m);
		render();   // notify it needs redraw
		ws_check(); // redraw now
	} while (m.e24 > 0);
	_animating = false;
}

void MyViewer::show_normals(bool view)
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup *r = (SnGroup *)root();
	for (int k = 0; k < r->size(); k++)
	{
		SnManipulator *manip = r->get<SnManipulator>(k);
		SnShape *s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines *l = manip->child<SnGroup>()->get<SnLines>(1);
		if (!view)
		{
			l->visible(false);
			continue;
		}
		l->visible(true);
		if (!l->empty())
			continue; // build only once
		l->init();
		if (s->instance_name() == SnPrimitive::class_name)
		{
			GsModel &m = *((SnModel *)s)->model();
			m.get_normals_per_face(fn);
			const GsVec *n = fn.pt();
			float f = 0.33f;
			for (int i = 0; i < m.F.size(); i++)
			{
				const GsVec &a = m.V[m.F[i].a];
				l->push(a, a + (*n++) * f);
				const GsVec &b = m.V[m.F[i].b];
				l->push(b, b + (*n++) * f);
				const GsVec &c = m.V[m.F[i].c];
				l->push(c, c + (*n++) * f);
			}
		}
	}
}

int MyViewer::handle_keyboard(const GsEvent &e)
{
	int ret = WsViewer::handle_keyboard(e); // 1st let system check events
	if (ret)
		return ret;

	switch (e.key)
	{
	case GsEvent::KeyEsc:
		gs_exit();
		return 1;
	case 'n':
	{
		bool b = !_nbut->value();
		_nbut->value(b);
		show_normals(b);
		return 1;
	}
	case GsEvent::KeyQ:
	{
		shoulder += (float)(GS_PI / 6);
		m[0].rotz(shoulder);
	
		t[0]->set(m[0]);
		render();
		ws_check();
		return 1;
	}
	case GsEvent::KeyA:
	{
			shoulder -= (float)(GS_PI / 6);
		m[0].rotz(shoulder);
	
		// gsout << m[0].getrans << gsnl;
		t[0]->set(m[0]);
		// g->add(t[0]);
		
		// rootg()->add(g);
		render();
		ws_check();
		return 1;
	}
	case GsEvent::KeyW:
	{
		redraw();
		return 1;
	}
	case GsEvent::KeyS:
	{
		redraw();
		return 1;
	}
	case GsEvent::KeyE:
	{
		redraw();
		return 1;
	}
	case GsEvent::KeyD:
	{
		redraw();
		return 1;
	}
	default:
		gsout << "Key pressed: " << e.key << gsnl;
	}

	return 0;
}

int MyViewer::uievent(int e)
{
	switch (e)
	{
	case EvNormals:
		show_normals(_nbut->value());
		return 1;
	case EvAnimate:
		run_animation();
		return 1;
	case EvExit:
		gs_exit();
	}
	return WsViewer::uievent(e);
}
