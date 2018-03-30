#include "CGLRenderer.h"
#include "depgraph.h"

using namespace std;

namespace swf
{
namespace ogl
{

// pushes a shape that will act as a clip mask.
void CGLRenderer::pushClipMask(std::shared_ptr<gfx::Shape_t> shape,
                               const Eigen::Matrix3f & transform)
{
    std::shared_ptr<ogl::Shape> clipShape = getCachedShape(shape, transform);
    if (!clipShape) {
        return;
    }
    Drawable clipping;
    for(vector<shared_ptr<Mesh> >::iterator it = clipShape->meshes.begin();
        it != clipShape->meshes.end();
        it++)
    {
        clipping.clipping.Meshes.push_back(*it);
    }

    float sx, sy;
    getScaling(sx, sy);
    Eigen::Matrix3f mat = Eigen::Scaling(sx, sy, 1.0f) * transform;

    clipping.clipping.Transformation = mat;
    clipping.Type                    = Drawable::eCLIP_MASK;

    shared_ptr<DependencyNode<Drawable> > clipNode = 
        make_shared<DependencyNode<Drawable> >(clipping);
    for(vector<shared_ptr<DependencyNode<Drawable> > >::iterator it = drawnMeshes.begin();
        it != drawnMeshes.end();
        it++)
    {
        clipNode->AddDependency(it->get());
    }
    sync(clipNode);
    drawnMeshes.push_back(clipNode);
    currentClipping = clipNode;
}

// pops the last pushed clip mask.
void CGLRenderer::popClipMask()
{
    if (currentClipping) {
        Drawable clipping;
        clipping.Type = Drawable::eCLIP_STOP;
        shared_ptr<DependencyNode<Drawable> > stopNode = 
            make_shared<DependencyNode<Drawable> >(clipping);
        for(vector<shared_ptr<DependencyNode<Drawable> > >::iterator it = drawnMeshes.begin();
            it != drawnMeshes.end();
            it++)
        {
            stopNode->AddDependency(it->get());
        }
        sync(stopNode);
        drawnMeshes.push_back(stopNode);
        currentClipping = 0;
        syncPoints.push_back(stopNode);
    }
}

}
}