
#include "my_viewer.h"

#include <sigogl/ui_button.h>
#include <sigogl/ui_radio_button.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>
#include <sig/sn_group.h>

#include <sigogl/ws_run.h>

MyViewer::MyViewer(int x, int y, int w, int h, const char *l) : WsViewer(x, y, w, h, l)
{
	float r = 0.1f;
	float R = 0.5f;
	n = 60.0F;
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

	rootg()->add(manip);
}

void MyViewer::build_scene()
{

	torus = new GsModel;
	SnModel *sn;

	int i = 0;
	for (float theta = 0.0f; theta < GS_2PI; theta += (float)(GS_2PI / n))
	{
		for (float phi = 0.0f; phi < GS_2PI; phi += (float)(GS_2PI / n))
		{
			torus->V.push() = GsVec((R + r * cos(theta)) * cos(phi), (R + r * cos(theta)) * sin(phi), r * sin(theta));
			torus->V.push() = GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi), r * sin(theta + (float)(GS_2PI / n)));
			torus->V.push() = GsVec((R + r * cos(theta)) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta)) * sin(phi + (float)(GS_2PI / n)), r * sin(theta));
			torus->V.push() = GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi + (float)(GS_2PI / n)), r * sin(theta + (float)(GS_2PI / n)));

			GsModel::Face f1 = GsModel::Face(i, i + 2, i + 3);

			torus->F.push() = f1;
			torus->F.push() = GsModel::Face(i, i + 3, i + 1);

			if (smooth)
			{
				GsVec k((float)(R * cos(phi)), (float)(R * sin(phi)), 0.0f);
				GsVec h((float)(R * cos(phi + (float)(GS_2PI / n))), (float)(R * sin(phi + (float)(GS_2PI / n))), 0.0f);
				
				torus->N.push() = (GsVec((R + r * cos(theta)) * cos(phi), (R + r * cos(theta)) * sin(phi), r * sin(theta)) - k);
				torus->N.push() = (GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi), r * sin(theta + (float)(GS_2PI / n))) - k);
				torus->N.push() = (GsVec((R + r * cos(theta)) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta)) * sin(phi + (float)(GS_2PI / n)), r * sin(theta)) - h);
				torus->N.push() = (GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi + (float)(GS_2PI / n)), r * sin(theta + (float)(GS_2PI / n))) - k);

				torus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			}
			else
			{
				GsPnt nx;
				nx.cross(GsVec((R + r * cos(theta)) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta)) * sin(phi + (float)(GS_2PI / n)), r * sin(theta)) - GsVec((R + r * cos(theta)) * cos(phi), (R + r * cos(theta)) * sin(phi), r * sin(theta)), GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi + (float)(GS_2PI / n)), r * sin(theta + (float)(GS_2PI / n))) - GsVec((R + r * cos(theta)) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta)) * sin(phi + (float)(GS_2PI / n)), r * sin(theta)));
				torus->N.push() = nx;

				GsPnt ny;
				ny.cross(GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi + (float)(GS_2PI / n)), r * sin(theta + (float)(GS_2PI / n))) - GsVec((R + r * cos(theta)) * cos(phi), (R + r * cos(theta)) * sin(phi), r * sin(theta)), GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi), r * sin(theta + (float)(GS_2PI / n))) - GsVec((R + r * cos(theta + (float)(GS_2PI / n))) * cos(phi + (float)(GS_2PI / n)), (R + r * cos(theta + (float)(GS_2PI / n))) * sin(phi + (float)(GS_2PI / n)), r * sin(theta + (float)(GS_2PI / n))));
				torus->N.push() = ny;

				torus->set_mode(GsModel::Flat, GsModel::NoMtl);
			}
			i += 4;
		}
	}

	sn = new SnModel(torus);
	rootg()->add(sn);
}

void MyViewer::compute_segments(bool smooth)
{

	SnLines *l = new SnLines;
	l->init();
	l->color(GsColor::blue);

	if (!smooth)
	{
		GsModel &m = *torus;
		for (int i = 0; i < m.F.size(); ++i)
		{
			const GsVec &a = m.V[m.F[i].a];
			const GsVec &b = m.V[m.F[i].b];
			const GsVec &c = m.V[m.F[i].c];
			GsVec fcenter = (a + b + c) / 3.0f;
			l->push(fcenter, fcenter + normalize(m.N[i]) * 0.1f);
		}
	}
	else
	{
		GsModel &m = *torus;
		for (int i = 0; i < m.V.size(); ++i)
		{
			const GsVec &a = m.V[i];

			l->push(a, a + m.N[i] * 0.5f);
		}
	}

	rootg()->add(l);
}

void MyViewer::run_animation()
{
	if (_animating)
		return;
	_animating = true;

	int ind = gs_random(0, rootg()->size() - 1);
	SnManipulator *manip = rootg()->get<SnManipulator>(ind);
	GsMat m = manip->mat();

	double frdt = 1.0 / 30.0;
	double v = 4;
	double t = 0, lt = 0, t0 = gs_time();
	do
	{
		while (t - lt < frdt)
		{
			ws_check();
			t = gs_time() - t0;
		}
		double yinc = (t - lt) * v;
		if (t > 2)
			yinc = -yinc;
		lt = t;
		m.e24 += (float)yinc;
		if (m.e24 < 0)
			m.e24 = 0;
		manip->initial_mat(m);
		render();
		ws_check();
	} while (m.e24 > 0);
	_animating = false;
}

void MyViewer::show_normals(bool view)
{
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
			continue;
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
	int ret = WsViewer::handle_keyboard(e);

	if (ret)
		return ret;

	switch (e.key)
	{
	case 'q':
		n += 1.0f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'a':
		if (n - 1.0f == 3.0f)
			return 1;
		n -= 1.0f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'w':
		r += 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 's':
		r -= 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'e':
		R += 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'd':
		R -= 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'z':
		smooth = false;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'x':
		smooth = true;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case 'c':
		compute_segments(smooth);
		build_scene();
		render();
		return 1;
	case 'v':
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	case GsEvent::KeyEsc:
		gs_exit();
		return 1;
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
