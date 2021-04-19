#include "Raytracer.h"
#include "Console.h"
#include "Intersect.h"
#include "SceneGraph.h"
#include "Thread.h"


struct RaytraceThread : public Thread
{
public:
    int run()
    {
        Sleep(1);
        Console::Out() << "Raytracing started...";

        camera.raytrace(threadNo, numThreads, stop, scene, targetBuffer);

        Console::Out() << "Raytracing finished...";
        return 0;
    }

    void interrupt()
    {
        stop = 1;
    }

    RaytraceThread(
        int                        iThreadNo,
        int                        iNumThreads,
        SceneGraph3d              &iScene,
        Camera3d                  &iCamera,
        TargetBuffer<PixelRGBA32> &iTargetBuffer)
        : threadNo(iThreadNo)
        , numThreads(iNumThreads)
        , scene(iScene)
        , camera(iCamera)
        , stop(0)
        , targetBuffer(iTargetBuffer)
    {}

    int                        threadNo;
    int                        numThreads;
    int                        stop;
    SceneGraph3d              &scene;
    Camera3d                  &camera;
    TargetBuffer<PixelRGBA32> &targetBuffer;
};


void cube(Mesh3d &mesh)
{
    GAL::P3d vertices[8] =
    {
        GAL::P3d(-1,-1,-1),
        GAL::P3d( 1,-1,-1),
        GAL::P3d(-1, 1,-1),
        GAL::P3d( 1, 1,-1),
        GAL::P3d(-1,-1, 1),
        GAL::P3d( 1,-1, 1),
        GAL::P3d(-1, 1, 1),
        GAL::P3d( 1, 1, 1)
    };

    int indices[36] =
    {
        0,2,1,  1,2,3,
        5,1,7,  7,1,3,
        4,7,6,  4,5,7,
        6,3,2,  6,7,3,
        0,1,5,  0,5,4,
        2,0,6,  0,4,6
    };

    mesh.addVertices(vertices, sizeof(vertices) / sizeof(GAL::P3d));
    mesh.addIndices(indices, sizeof(indices) / sizeof(int));
}

double manifoldFunction(double x, double y)
{
    return -0.5 * x * x - 0.75 * y * y;
}

void manifold(Mesh<Vertex3d> &mesh)
{
    const int N = 7;
    const int M = N - 1;
    const double L = ((double)N) / 2.0;

    Vertex3d vertices[N * N];
    int indices[M * M * 6];

    int index = 0;

    for (int ix = 0; ix < N; ++ix)
    {
        for (int iy = 0; iy < N; ++iy)
        {
            double x = (1.0 * ix - L + 0.5) / L;
            double y = (1.0 * iy - L + 0.5) / L;
            double z = manifoldFunction(x,y);


            double xdx = x + 0.01;
            double ydy = y + 0.01;
            double zdx = manifoldFunction(xdx, y);
            double zdy = manifoldFunction(x, ydy);

            GAL::P3d p0(x,y,z);
            GAL::P3d pdx(xdx, y, zdx);
            GAL::P3d pdy(x, ydy, zdy);

            GAL::P3d nor = GAL::Cross(pdx - p0, pdy - p0);
            
            vertices[index].position = p0;
            vertices[index].normal = nor / GAL::Len(nor);
            ++index;
        }
    }

    index = 0;

    for (int n = 0; n < M; ++n)
    {
        for (int m = 0; m < M; ++m)
        {
            int i0 = n * N + m;
            int i1 = i0 + 1;
            int i2 = i0 + N;
            int i3 = i0 + N + 1;

            indices[index++] = i0;
            indices[index++] = i2;
            indices[index++] = i1;

            indices[index++] = i1;
            indices[index++] = i2;
            indices[index++] = i3;
        }
    }

    mesh.addVertices(vertices, sizeof(vertices) / sizeof(GAL::P3d));
    mesh.addIndices(indices, sizeof(indices) / sizeof(int));
}


Raytracer::Raytracer(): texture(0)
{
    prepareTargetBuffer(128,128);

    std::shared_ptr<Clump3d> clump(new Clump3d);
    
    //
    // Cube
    //

    std::shared_ptr<MeshGeometry3d> geom1(new MeshGeometry3d());
    cube(geom1->getMesh());
    geom1->meshChanged();
    geom1->setColor(GAL::P4d(1.0, 0.0, 0.0, 1.0));
    geom1->setReflective(true);

    std::shared_ptr<MeshGeometry3d> geom5(new MeshGeometry3d());
    cube(geom5->getMesh());
    geom5->meshChanged();
    geom5->setColor(GAL::P4d(0.0, 0.7, 1.0, 1.0));
    geom5->setReflective(true);

    //
    // Manifold
    //

    std::shared_ptr<MeshGeometry<Vertex3d> > geom3(new MeshGeometry<Vertex3d>());
    manifold(geom3->getMesh());
    geom3->meshChanged();
    geom3->setColor(GAL::P4d(1.0, 1.0, 0.0, 1.0));
    geom3->setReflective(true);

    //
    // Sphere
    //

    std::shared_ptr<SphereGeometry3d> geom2(new SphereGeometry3d(0.5));
    geom2->setColor(GAL::P4d(0.0, 1.0, 1.0, 1.0));
    geom2->setReflective(true);
    
    std::shared_ptr<SphereGeometry3d> geom4(new SphereGeometry3d(0.25));
    geom4->setColor(GAL::P4d(1.0, 0.8, 0.0, 1.0));
    geom4->setReflective(true);
    
    //
    // Cylinder
    //
    
    std::shared_ptr<CylinderGeometry3d> geom6(new CylinderGeometry3d(0.4, GAL::P3d(0.4,1.3,-0.5)));
    geom6->setColor(GAL::P4d(0.9, 0.9, 0.9, 1.0));
    geom6->setReflective(true);

    clump->addGeometry(geom1);
    clump->addGeometry(geom2);
    clump->addGeometry(geom3);
    clump->addGeometry(geom4);
    clump->addGeometry(geom5);
    clump->addGeometry(geom6);

    geom1->setTranslation(GAL::P3d(-1,-1,0));
    geom2->setTranslation(GAL::P3d(0,1,0));
    
    geom3->setTranslation(GAL::P3d(1,0.5,0));
    geom3->setLocalTransform(GAL::EulerRotationX(90.0), 0);

    geom4->setTranslation(GAL::P3d(1.2,1,0.8));
    
    geom5->setTranslation(GAL::P3d(0,1,-3));
    geom5->setLocalTransform(GAL::EulerRotationY(10.0), 0);

    geom6->setTranslation(GAL::P3d(-1.2,0.7,0.0));
    geom6->setLocalTransform(GAL::EulerRotationX(40.0) * GAL::EulerRotationY(40.0), 0);

    scene.addClump(clump);

    std::shared_ptr<Light3d> light1(new Light3d());
    light1->setPosition(GAL::P3d(1,4,-1));
    light1->setDiffuseColor(GAL::P3d(0.7, 0.7, 0.7));
    light1->setSpecularColor(GAL::P3d(1.0, 1.0, 1.0));
    light1->setShadow(true);
    light1->setSoftShadowWidth(0.05);
    scene.addLight(light1);

    std::shared_ptr<Light3d> light2(new Light3d());
    light2->setPosition(GAL::P3d(-1,4,3));
    light2->setDiffuseColor(GAL::P3d(0.7, 0.7, 0.7));
    light2->setSpecularColor(GAL::P3d(1.0, 1.0, 1.0));
    light2->setShadow(true);
    light2->setSoftShadowWidth(0.05);
    scene.addLight(light2);

    camera.setFrustum(-1, 1, -1, 1, -1, -100);

    camera.setTranslation(GAL::P3d(1,2,3));
    camera.setLocalTransform(GAL::EulerRotationX(30.0) * GAL::EulerRotationY(15.0), 0);
    camera.setFSAA(true);
    camera.setRecursionDepth(3);
}

Raytracer::~Raytracer()
{
    waitAllThreads();
    delete [] targetBuffer.pixels;
}

void Raytracer::beforeWindowShown()
{
}

void Raytracer::windowSizeChanged(int width, int height)
{
    glViewport(0, 0, width, height);

    windowWidth = width;
    windowHeight = height;

    double aspect = height / (double)width;
	double w = sqrt(sqrt(2.0) / (aspect * aspect + 1));
	double h = aspect * w;

    camera.getFrustum().mLeft   = -w;
    camera.getFrustum().mRight  = w;
    camera.getFrustum().mBottom = -h;
    camera.getFrustum().mTop    = h;

    needRedraw = true;
}

void Raytracer::mouseMotion(const MotionEvent &evt)
{
    //Console::Out() << "Mouse Motion: (" << evt.position << "), (" << evt.movement << ")";
}


void Raytracer::buttonDown(const ButtonEvent &evt)
{
    Console::Out() << "Button Down: (" << evt.position << ") " << evt.button;
}

void Raytracer::buttonUp(const ButtonEvent &evt)
{
    Console::Out() << "Button Up: (" << evt.position << ") " << evt.button;
}

void Raytracer::keyDown(int keyCode)
{
    switch(keyCode)
    {
    case '0':
        camera.setRecursionDepth(0);
        break;
    case '1':
        camera.setRecursionDepth(1);
        break;
    case '2':
        camera.setRecursionDepth(2);
        break;
    case '3':
        camera.setRecursionDepth(3);
        break;
    case '4':
        camera.setRecursionDepth(4);
        break;
    case '5':
        camera.setRecursionDepth(5);
        break;
    case 'S':
        for (SceneGraph3d::ListLights::const_iterator i = scene.getLights().begin(); i != scene.getLights().end(); ++i)
        {
            if (!(*i)->isShadow())
            {
                (*i)->setShadow(true);
                (*i)->setSoftShadowWidth(0.0);
            }
            else if (0.0 == (*i)->getSoftShadowWidth())
            {
                (*i)->setSoftShadowWidth(0.05);
            }
            else
            {
                (*i)->setShadow(false);
            }
        }
        break;
    case 'A':
        camera.setFSAA(!camera.isFSAA());
        break;
    }
    needRedraw = true;
}

void Raytracer::keyUp(int keyCode)
{
}

void Raytracer::willQuit()
{
    glDeleteTextures(1, &texture);
}

bool Raytracer::allThreadsFinished()
{
    for (ThreadListType::iterator it = threads.begin();
        it != threads.end(); ++it)
    {
        if (!(*it)->isFinished())
        {
            return false;
        }
    }
    return true;
}

void Raytracer::interruptAllThreads()
{
    for (ThreadListType::iterator it = threads.begin();
        it != threads.end(); ++it)
    {
        (*it)->interrupt();
    }
}

void Raytracer::waitAllThreads()
{
    interruptAllThreads();

    for (ThreadListType::iterator it = threads.begin();
        it != threads.end(); ++it)
    {
        (*it)->join();
    }
}

void Raytracer::prepareTargetBuffer(int width, int height)
{
    waitAllThreads();
    delete [] targetBuffer.pixels;

    targetBuffer.width  = width;
    targetBuffer.height = height;
    targetBuffer.pitch  = targetBuffer.width * PixelRGBA32::BytesPerPel;
    size_t targetBufferSize = (targetBuffer.height + 1) * targetBuffer.pitch;
    targetBuffer.pixels = new char[targetBufferSize];
    memset(targetBuffer.pixels, 0, targetBufferSize);
}

void Raytracer::startRaytrace()
{
    if (!allThreadsFinished())
    {
        interruptAllThreads();
        return;
    }

    threads.clear();
    needRedraw = false;

#ifndef _DEBUG
    prepareTargetBuffer(windowWidth, windowHeight);
    int numThreads = 6;
#else
    int numThreads = 1;
#endif


    for (int threadNo = 0; threadNo != numThreads; ++threadNo)
    {
        ThreadPtr t(new RaytraceThread(threadNo, numThreads, scene, camera, targetBuffer));
        threads.push_back(t);
    }
}

void Raytracer::processTick()
{
    if (needRedraw)
    {
        startRaytrace();
    }
    
	glClearColor(0,0,0,0);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (0 == texture)
    {
        glGenTextures(1, &texture);
    }

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, targetBuffer.width, targetBuffer.height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, targetBuffer.pixels);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, 0, 10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);


    glEnable(GL_TEXTURE_2D);

    glBegin(GL_TRIANGLE_STRIP);
    glColor4f(1.0,1.0,1.0,1.0);

    glTexCoord2f(0,1);
    glVertex2f(-1.0,1.0);

    glTexCoord2f(0,0);
    glVertex2f(-1.0,-1.0);

    glTexCoord2f(1,1);
    glVertex2f(1.0,1.0);

    glTexCoord2f(1,0);
    glVertex2f(1.0,-1.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);
}

