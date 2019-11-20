
This example program uses the old API of OpenGL, so you CANNOT have access to the same glu functions in your projects. This is why we are not giving to you the source code of this example program.

However, our support code replicates "gluLookAt" and "gluPerspective" with transformation matrices, and you can find in gs_mat.h methods to create the equivalent matrices. You can also access the GsCamera class using WsViewer::camera() and then manipulate the same camera parameters.

