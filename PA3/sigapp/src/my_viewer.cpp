
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>


MyViewer::MyViewer(int x, int y, int w, int h, const char* l) : WsViewer(x, y, w, h, l)
{
	_nbut = 0;
	_animating = false;

	build_ui();
	build_scene();
}

void MyViewer::build_ui()
{
	UiPanel* p, * sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel("", UiPanel::HorizLeft);
	p->add(new UiButton("View", sp = new UiPanel()));
	{	UiPanel* p = sp;
	p->add(_nbut = new UiCheckButton("Normals", EvNormals));
	}
	p->add(new UiButton("Animate", EvAnimate));
	p->add(new UiButton("Exit", EvExit)); p->top()->separate();
}

void MyViewer::add_model(SnShape* s, GsVec p)
{
	// This method demonstrates how to add some elements to our scene graph: lines,
	// and a shape, and all in a group under a SnManipulator.
	// Therefore we are also demonstrating the use of a manipulator to allow the user to
	// change the position of the object with the mouse. If you do not need mouse interaction,
	// you can just use a SnTransform to apply a transformation instead of a SnManipulator.
	// You would then add the transform as 1st element of the group, and set g->separator(true).
	// Your scene graph should always be carefully designed according to your application needs.

	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation(p);
	manip->initial_mat(m);

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);
	// manip->visible(false); // call this to turn off mouse interaction

	rootg()->add(manip);
}

void MyViewer::build_scene()
{	
	SnModel* model[3];
	model[0] = new SnModel;
	t[0] = new SnTransform;
	if (!model[0]->model()->load("../../arm/rupperarm.m")) {
		gsout << "rupperarm.m was not loaded" << gsnl;
	}
	model[0]->model()->smooth();
	model[0]->model()->get_bounding_box(b0);
	gsout << -b0.dz() / 2.0f << gsnl;
	m[0].translation(GsVec(0, 0, -b0.dz() / 2.0f));
	m[0].rotx(float(GS_PI / 12));
	t[0]->set(m[0]);

	model[1] = new SnModel;
	t[1] = new SnTransform;
	if (!model[1]->model()->load("../../arm/rlowerarm.m")) {
		gsout << "rlowerarm.m was not loaded" << gsnl;
	}
	model[1]->model()->smooth();
	model[1]->model()->get_bounding_box(b1);
	m[1].translation(GsVec(0, 0, (b1.dz() / 2.0f) * 2));
	mt[1].rotx(float(-GS_PI / 12));
	m[1] = m[1] * mt[1];
	t[1]->set(m[1]);


	model[2] = new SnModel;
	t[2] = new SnTransform;
	if (!model[2]->model()->load("../../arm/rhand.m")) {
		gsout << "rhand.m was not loaded" << gsnl;
	}
	model[2]->model()->get_bounding_box(b1);
	m[2].translation(GsVec(0, 0, -b1.dz() + b1.dz() + 25.0f));
	t[2]->set(m[2]);

	for (int i = 0; i < 3; i++) {
		g->add(t[i]);
		g->add(model[i]);
	}
	rootg()->add(g);
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation()
{
	if (_animating) return; // avoid recursive calls
	_animating = true;

	double frdt = 1.0 / 30.0; // delta time to reach given number of frames per second
	double time = 0, lt = 0, t0 = gs_time();
	do // run for a while:
	{
		while (time - lt < frdt) { ws_check(); time = gs_time() - t0; } // wait until it is time for next frame
		lt = time;
		if (time < 2) {
			if (time < 1) {
				mt[0].rotz(upperarm);
				mt[1].rotx(-lowerarm);
				mt[2].roty(hand);

				m[0] = m[0] * mt[0];
				m[1] = m[1] * mt[1];
				m[2] = m[2] * mt[2];

				t[0]->set(m[0]);
				t[1]->set(m[1]);
				t[2]->set(m[2]);

				redraw();
			}
		}
		if (time > 3 && time < 4) {
			mt[0].rotx(-upperarm);
			mt[1].rotx(-lowerarm + 0.01f);
			mt[2].roty(-hand);

			m[0] = m[0] * mt[0];
			m[1] = m[1] * mt[1];
			m[2] = m[2] * mt[2];
			
			t[0]->set(m[0]);
			t[1]->set(m[1]);
			t[2]->set(m[2]);

			redraw();
		}

		if (time > 6 && time < 8) {
			mt[0].rotx(upperarm);
			mt[1].rotx(lowerarm);
			mt[2].roty(hand);

			m[0] = m[0] * mt[0];
			m[1] = m[1] * mt[1];
			m[2] = m[2] * mt[2];

			t[0]->set(m[0]);
			t[1]->set(m[1]);
			t[2]->set(m[2]);

			redraw();
		}
	} while (time < 11);
	_animating = false;
}

void MyViewer::show_normals(bool view)
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for (int k = 0; k < r->size(); k++)
	{
		SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if (!view) { l->visible(false); continue; }
		l->visible(true);
		if (!l->empty()) continue; // build only once
		l->init();
		if (s->instance_name() == SnPrimitive::class_name)
		{
			GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face(fn);
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for (int i = 0; i < m.F.size(); i++)
			{
				const GsVec& a = m.V[m.F[i].a]; l->push(a, a + (*n++) * f);
				const GsVec& b = m.V[m.F[i].b]; l->push(b, b + (*n++) * f);
				const GsVec& c = m.V[m.F[i].c]; l->push(c, c + (*n++) * f);
			}
		}
	}
}

void MyViewer::upperArmUp() {

	gsout << "q" << gsnl;
	mt[0].rotx(-upperarm);
	m[0] = m[0] * mt[0];
	t[0]->set(m[0]);
	redraw();
}

void MyViewer::upperArmDown() {
	gsout << "a" << gsnl;
	mt[0].rotx(upperarm);
	m[0] = m[0] * mt[0];
	t[0]->set(m[0]);
	redraw();
}

void MyViewer::lowerArmUp() {
gsout << "w" << gsnl;
	mt[1].rotx(-lowerarm);
	m[1] = m[1] * mt[1];
	t[1]->set(m[1]);
	redraw();
}

void MyViewer::lowerArmDown() {
	gsout << "s" << gsnl;
	mt[1].rotx(lowerarm);
	m[1] = m[1] * mt[1];
	t[1]->set(m[1]);
	redraw();
}

void MyViewer::handUp() {
	gsout << "e" << gsnl;
	mt[2].rotx(-hand);
	m[2] = m[2] * mt[2];
	t[2]->set(m[2]);
	redraw();
}

void MyViewer::handDown() {
	gsout << "d" << gsnl;
	mt[2].rotx(hand);
	m[2] = m[2] * mt[2];
	t[2]->set(m[2]);
	redraw();
}

int MyViewer::handle_keyboard(const GsEvent& e)
{	
	int ret = WsViewer::handle_keyboard(e); // 1st let system check events
	if (ret) return ret;

	switch (e.key)
	{
	case GsEvent::KeyEsc: gs_exit(); return 1;
	case 'q': upperArmUp(); return 1; 
	case 'a': upperArmDown(); return 1;
	case 'w': lowerArmUp(); return 1; 
	case 's': lowerArmDown(); return 1;
	case 'e': handUp(); return 1;
	case 'd': handDown(); return 1;

	case 'n': { bool b = !_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
	default: gsout << "Key pressed: " << e.key << gsnl;
	}

	return 0;
}

int MyViewer::uievent(int e)
{
	switch (e)
	{
	case EvNormals: show_normals(_nbut->value()); return 1;
	case EvAnimate: run_animation(); return 1;
	case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
