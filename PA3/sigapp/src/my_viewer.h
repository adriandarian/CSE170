#pragma once

#include <vector>
#include <string>

#include <sig/sn_poly_editor.h>
#include <sig/sn_lines2.h>

#include <sigogl/ui_button.h>
#include <sigogl/ws_viewer.h>

#include <sigogl/ui_button.h>
#include <sigogl/ui_radio_button.h>
#include <sig/sn_primitive.h>
#include <sig/sn_transform.h>
#include <sig/sn_manipulator.h>

#include <sigogl/ws_run.h>

// Viewer for this example:
class MyViewer : public WsViewer
{
protected:
	enum MenuEv
	{
		EvNormals,
		EvAnimate,
		EvExit
	};
	UiCheckButton *_nbut;
	bool _animating;
	SnGroup *g;		   // to hold all the models and transformation
	SnModel *model[3]; // to hold all the models
	SnTransform *t[3]; // to hold all the transformation
	GsMat m[3];		   // to hold all the transformation matrices
	GsBox b0, b1, b2;
	std::vector<GsBox> b;
	std::vector<std::string> pieces;
	float shoulder = (float)(GS_PI / 4.0f), shoulderOrigin, elbow = (float)(-GS_PI / 3.0f), elbowOrigin, hand = (float)(GS_PI / 12.0f), handOrigin;

public:
	MyViewer(int x, int y, int w, int h, const char *l);
	void build_ui();
	void add_model(SnShape *s, GsVec p);
	void build_scene(float shoulder, float elbow, float hand);
	void show_normals(bool view);
	void run_animation();
	virtual int handle_keyboard(const GsEvent &e) override;
	virtual int uievent(int e) override;
};
