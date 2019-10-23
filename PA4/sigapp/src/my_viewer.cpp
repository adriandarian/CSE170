
#include "my_viewer.h"

#include <sigogl/ui_button.h>
#include <sigogl/ui_radio_button.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>
#include <sig/sn_group.h>

#include <sigogl/ws_run.h>
bool flat;

int numFaces = 10;


double pie = 6.28f; // GS_2PI; //2pi
double theta;		//outer circle
double phi;			// inner circle

MyViewer::MyViewer(int x, int y, int w, int h, const char *l) : WsViewer(x, y, w, h, l)
{
	 r = 0.1f;
	 R = 0.5f;
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
	////gsout << cos(3.14) << gsnl;
	//gsout << cos(60) << gsnl;

	delta = pie / n;
	for (theta = 0.0; theta < pie; theta += delta) //nested for loop
	{
		for (phi = 0.0; phi < pie; phi += delta)
		{
			//create 6 vectors for the two triangles
			//Triangle 1 ACD
			//Triangle 2 ADB
			GsVec A((R + r * cos(theta)) * cos(phi), (R + r * cos(theta)) * sin(phi), r * sin(theta));
			GsVec B((R + r * cos(theta + delta)) * cos(phi), (R + r * cos(theta + delta)) * sin(phi), r * sin(theta + delta));
			GsVec C((R + r * cos(theta)) * cos(phi + delta), (R + r * cos(theta)) * sin(phi + delta), r * sin(theta));
			GsVec D((R + r * cos(theta + delta)) * cos(phi + delta), (R + r * cos(theta + delta)) * sin(phi + delta), r * sin(theta + delta));

			torus->V.push() = A; //0
			torus->V.push() = B; //1
			torus->V.push() = C; //2
			torus->V.push() = D; //3

			/*torus->V.push() = A;
			torus->V.push() = D;
			torus->V.push() = C;

			torus->V.push() = A;
			torus->V.push() = D;
			torus->V.push() = B;*/

			GsModel::Face f1 = GsModel::Face(i, i + 2, i + 3);
			GsModel::Face f2 = GsModel::Face(i, i + 3, i + 1);
			/*	GsModel::Face f1 = GsModel::Face(i, i + 1, i + 2);
			GsModel::Face f2 = GsModel::Face(i + 3, i + 4, i + 5);
*/

			torus->F.push() = f1;
			torus->F.push() = f2;

			if (flat == false)
			{

				//Calculating normals
				GsVec orgin1((R * cos(phi)), (R * sin(phi)), (double)0);
				GsVec orgin2((R * cos(phi + delta)), (R * sin(phi + delta)), (double)0);

				GsVec nA = (A - orgin1);
				GsVec nB = (B - orgin1);
				GsVec nC = (C - orgin2);
				GsVec nD = (D - orgin2);

				torus->N.push() = nA;
				torus->N.push() = nB;
				torus->N.push() = nC;
				torus->N.push() = nD;

				torus->set_mode(GsModel::Smooth, GsModel::NoMtl);
			}
			else
			{
				GsPnt nx;
				nx.cross(C - A, D - C);
				GsPnt ny;
				ny.cross(D - A, B - D);

				torus->N.push() = nx;
				torus->N.push() = ny;

				torus->set_mode(GsModel::Flat, GsModel::NoMtl);
			}
			i += 4;
		}
	}

	sn = new SnModel(torus);
	//SnGroup *T = new SnGroup;
	//T->add(sn);
	rootg()->add(sn);
}

void MyViewer::compute_segments(bool flat)
{

	SnLines *l = new SnLines;
	l->init();
	l->color(GsColor::blue);

	if (flat == true)
	{
		GsModel &m = *torus;
		for (int i = 0; i < m.F.size(); ++i)
		{
			const GsVec &a = m.V[m.F[i].a];
			const GsVec &b = m.V[m.F[i].b];
			const GsVec &c = m.V[m.F[i].c];
			GsVec fcenter = (a + b + c) / 3.0f;
			l->push(fcenter, fcenter + m.N[i] * 50.0f);
		}
	}
	else
	{ //smooth
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
	gsout << "start: " << n << gsnl;
	if (ret)
		return ret;

	switch (e.key)
	{
	case 'q':
	{
		n += 1.0f;
		gsout << "increase: " << n << gsnl;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	}
	case 'a':
	{
		if (n - 1.0f == 3.0f)
			return 1;
		n -= 1.0f;
		gsout << "decrease: " << n << gsnl;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	}
	case 'w':
	{
		r += 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	}
	case 's':
	{
		r -= 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	}
	case 'e':
	{
		R += 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	}
	case 'd':
	{
		R -= 0.1f;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	}
	case 'z':
	{
		flat = true;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	} // Flat
	case 'x':
	{
		flat = false;
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	} // Smooth
	case 'c':
	{
		compute_segments(flat);
		build_scene();
		render();
		return 1;
	} //normal
	case 'v':
	{
		rootg()->remove_all();
		build_scene();
		render();
		return 1;
	} // clear all

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
