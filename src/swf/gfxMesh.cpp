#include "gfxMesh.h"
#include <vector>
#include <list>
#include "gfxPoint.h"
#include "gfxBezier.h"

extern "C" 
{
#include <glutess\tesselator.h>
}


using namespace std;

namespace swf
{
namespace gfx
{

#define MAX_SUBDIVISION_DEPTH (200)

/******************************************************************************/
/*                              Forward declaration                           */
/******************************************************************************/
static void generatePolyline(const std::vector<Edge_t> & a_Edges, 
                      std::vector<Edge_t> & a_StraightEdges,
                      float a_MaxError);

static bool triangulate(const EdgeList_t & outerEdges,
                       const std::vector<EdgeList_t> & innerEdges,
                       std::vector<Triangle_t> & indicies,
                       std::vector<Point_t<float> > & verticies,
                       float scaleFactor);

static bool triangulate(const EdgeList_t & outerEdges,
                       std::vector<Triangle_t> & indicies,
                       std::vector<Point_t<float> > & verticies,
                       float scaleFactor);

static void generateTextureCoords(const std::vector<Point_t<float> > & a_Verticies, 
    std::vector<Point_t<float> > & a_TexCoords);

static void SetMeshMaterial(const Path_t a_Path, Mesh & a_Mesh);
static void SetLineMaterial(const Path_t a_Path, Mesh & a_Mesh);

static void calculateBounds(Mesh & a_Mesh);

static void mergeMeshes(list<pair<Material, gfx::Mesh> > & a_Meshes,const gfx::Mesh & a_Mesh);

static void GetStrokeMesh(const std::vector<Edge_t> & a_Edges, 
                          const LineStyle_t & a_LineStyle, 
                          bool a_IsPathClosed,
                          gfx::Mesh & a_Mesh);

/*****************************************************************************/
/*                              Exported functions                           */
/*****************************************************************************/

// returns the memory footprint of the mesh data, in bytes.
size_t Mesh::GetMemoryFootprint() const
{
    return (VertexData.size() * sizeof(Point_t<float>)) +
        (TextureCoords.size() * sizeof(Point_t<float>)) +
        (Indicies.size() * sizeof(Triangle_t));
}

/**
 * Triangulates a shape to one or more triangles meshes.
 */
bool triangulateShape(const gfx::Shape_t & shape, std::vector<gfx::Mesh> & meshes, 
                      float ErrorTolerance)
{
    for(size_t outIt = 0, numOuter = shape.OuterPaths.size();
        outIt < numOuter;
        ++outIt)
    {
        const Path_t & outerPath = shape.Paths[shape.OuterPaths[outIt]];
        if (outerPath.ClosedPath) {
            if (outerPath.Edges.size() <= 2) {
                continue;
            }

            list<pair<Material, gfx::Mesh> > innerMeshes;

            // generate straight edges that approximate the curved edges.
            vector<Edge_t> outerStraights;
            generatePolyline(outerPath.Edges, outerStraights, ErrorTolerance);

            Mesh mesh;
            list<size_t> holes, interiorShapes;
            // iterate over the inner shapes and determine if they define holes in the shape,
            // or just inner shapes.
            for(size_t inIt = 0, numInner = outerPath.InnerPaths.size();
                inIt < numInner;
                ++inIt) 
            {
                const Path_t & innerPath = shape.Paths[outerPath.InnerPaths[inIt]];
                if ((shape.Glyph == false) && ((innerPath.ClosedPath == false) || (innerPath.CCW == 0))) {
                    // defines a interior shape
                    interiorShapes.push_back(inIt);
                } else if (innerPath.Edges.size() > 2) {
                    // defines a hole
                    holes.push_back(inIt);
                }
            }
            if (holes.empty()) { // there is no holes in this shape
                if (outerStraights.size() > 2) {
                    if (!triangulate(outerStraights, mesh.Indicies, mesh.VertexData, (float) shape.ScaleFactor)) {
                        return false;
                    }
                } else {
                    // Todo, generate stroke.
                }
            } else { // they are holes in the shape.
                vector<EdgeList_t> innerPaths(holes.size());
                for(size_t hIt = 0, numHoles = holes.size();
                    hIt < numHoles;
                    ++hIt)
                {
                    const Path_t & hole = shape.Paths[outerPath.InnerPaths[hIt]];
                    generatePolyline(hole.Edges, innerPaths[hIt], ErrorTolerance);
                }
                // now triangulate the shape with the holes.
                if (!triangulate(outerStraights, innerPaths, mesh.Indicies, mesh.VertexData, (float) shape.ScaleFactor)) {
                    return false;
                }
            }
            // Now generate texture coordinates.
            generateTextureCoords(mesh.VertexData, mesh.TextureCoords);
            SetMeshMaterial(outerPath, mesh);
            meshes.push_back(mesh);

            // Now handle interior shapes that aren't holes
            for(list<size_t>::iterator it = interiorShapes.begin();
                it != interiorShapes.end();
                it++)
            {
                const Path_t & path = shape.Paths[outerPath.InnerPaths[*it]];
                if (path.ClosedPath) {
                    gfx::Mesh innerMesh;
                    vector<Edge_t> edges;
                    generatePolyline(path.Edges, edges, ErrorTolerance);
                    if (edges.size() <= 2) {
                        continue;
                    }
                    if (!triangulate(edges, innerMesh.Indicies, innerMesh.VertexData, (float) shape.ScaleFactor)) {
                        return false;
                    }
                    if (!innerMesh.Indicies.empty() && !innerMesh.VertexData.empty()) {
                        // TODO: determine why this is required, Why does it produce no vertex data?
                        generateTextureCoords(innerMesh.VertexData, innerMesh.TextureCoords);
                        SetMeshMaterial(path, innerMesh);
                        mergeMeshes(innerMeshes, innerMesh);
                    }
                } else {
                    gfx::Mesh innerMesh;
                    vector<Edge_t> edges;
                    generatePolyline(path.Edges, edges, ErrorTolerance);
                    GetStrokeMesh(edges, path.LineStyle, true, innerMesh);
                    SetMeshMaterial(path, innerMesh);
                    mergeMeshes(innerMeshes, innerMesh);
                }
            }
            for(list<pair<Material, gfx::Mesh> >::iterator it = innerMeshes.begin();
                it != innerMeshes.end();
                it++)
            {
                meshes.push_back(it->second);
            }
        } else {
            gfx::Mesh mesh;
            vector<Edge_t> edges;
            generatePolyline(outerPath.Edges, edges, ErrorTolerance);
            GetStrokeMesh(edges, outerPath.LineStyle, true, mesh);
            SetLineMaterial(outerPath, mesh);
            meshes.push_back(mesh);
        }
    }

    for(size_t i = 0; i < meshes.size(); ++i) {
        calculateBounds(meshes[i]); 
    }
    return true;
}

static void calculateBounds(Mesh & a_Mesh)
{
    gfx::Rect_t<float> & bounds = a_Mesh.Bounds;
    if (a_Mesh.VertexData.empty()) {
        bounds = gfx::Rect_t<float>(0.0f, 0.0f, 0.0f, 0.0f);
    } else {
        bounds.left = bounds.right = a_Mesh.VertexData[0].X;
        bounds.top = bounds.bottom = a_Mesh.VertexData[0].Y;
        for(size_t i = 1, num = a_Mesh.VertexData.size(); i < num; ++i) 
        {
            bounds.left     = std::min(bounds.left, a_Mesh.VertexData[i].X);
            bounds.right    = std::max(bounds.right, a_Mesh.VertexData[i].X);
            bounds.top      = std::min(bounds.top, a_Mesh.VertexData[i].Y);
            bounds.bottom   = std::max(bounds.bottom, a_Mesh.VertexData[i].Y);
        }
    }
}

/*****************************************************************************/
/*                              Static functions                             */
/*****************************************************************************/
void generateTextureCoords(const std::vector<Point_t<float> > & a_Verticies, 
    std::vector<Point_t<float> > & a_TexCoords)
{
    Point_t<float> minValue(FLT_MAX, FLT_MAX);
    Point_t<float> maxValue(FLT_MIN, FLT_MIN);

    /** get the extreme values for the verticies */
    a_TexCoords.resize(a_Verticies.size());
    for(size_t i = 0, count = a_Verticies.size(); i < count; ++i) {
        minValue.X = std::min(minValue.X, a_Verticies[i].X);
        minValue.Y = std::min(minValue.Y, a_Verticies[i].Y);
        maxValue.X = std::max(maxValue.X, a_Verticies[i].X);
        maxValue.Y = std::max(maxValue.Y, a_Verticies[i].Y);
    }

    float w = maxValue.X - minValue.X;
    float h = maxValue.Y - minValue.Y;

    /** calculate the texture coordinate of each vertex */
    for(size_t i = 0, count = a_Verticies.size(); i < count; ++i) {
        a_TexCoords[i].X = (a_Verticies[i].X - minValue.X) / w;
        a_TexCoords[i].Y = (a_Verticies[i].Y - minValue.Y) / h;
    }
}

static void translateColor(const gfx::Color_t & a_Color, float a_fColor[4])
{
    a_fColor[0] = float(a_Color.r) / 255;
    a_fColor[1] = float(a_Color.g) / 255;
    a_fColor[2] = float(a_Color.b) / 255;
    a_fColor[3] = float(a_Color.a) / 255;
}

static void SetLineMaterial(const Path_t a_Path, Mesh & a_Mesh)
{
    a_Mesh.Material.MaterialType = gfx::Material::MATERIAL_SOLID_COLOR;
    translateColor(a_Path.LineStyle.Color, a_Mesh.Material.Color);
}

static void SetMeshMaterial(const Path_t a_Path, Mesh & a_Mesh)
{
    switch(a_Path.FillStyle.FillType) 
    {
    case gfx::FillStyle_t::SolidFill:
        a_Mesh.Material.MaterialType = gfx::Material::MATERIAL_SOLID_COLOR;
        translateColor(a_Path.FillStyle.u.Color, a_Mesh.Material.Color);
        break;
    case gfx::FillStyle_t::LinearGradient:
    case gfx::FillStyle_t::RadialGradient:
        a_Mesh.Material.Gradient.Count  = a_Path.FillStyle.u.gradient.ControlPoints;
        if (a_Path.FillStyle.FillType == gfx::FillStyle_t::LinearGradient) {
            a_Mesh.Material.MaterialType        = gfx::Material::MATERIAL_LINEAR_GRADIENT;
        } else {
            a_Mesh.Material.MaterialType        = gfx::Material::MATERIAL_RADIAL_GRADIENT;
        }
        for(uint8_t i = 0; i < a_Path.FillStyle.u.gradient.ControlPoints; ++i) {
            a_Mesh.Material.Gradient.Points[i].Ratio = a_Path.FillStyle.u.gradient.points[i].Ratio;
            translateColor(a_Path.FillStyle.u.gradient.points[i].Color, a_Mesh.Material.Gradient.Points[i].Color);
        }
        break;
    }
}

static void addPoints(const EdgeList_t & a_Edges, const Point_t<int32_t> & a_StartPoint,
        std::vector<Point_t<int32_t> > & a_Points)
{
    size_t startIndex = 0, numEdges = a_Edges.size();
    for(size_t i = 0; i < numEdges; ++i) {
        if (a_StartPoint == a_Edges[i].Start) {
            startIndex = i;
        }
    }
    /** add points [startIndex, end] */
    for(size_t i = startIndex; i < numEdges; ++i) {
        a_Points.push_back(a_Edges[i].Start);
    }
    /** add points [0, startIndex] */
    for(size_t i = 0; i < startIndex; ++i) {
        a_Points.push_back(a_Edges[i].Start);
    }
}

static bool triangulate(const EdgeList_t & outerEdges,
                       const std::vector<EdgeList_t> & innerEdges,
                       std::vector<Triangle_t> & indicies,
                       std::vector<Point_t<float> > & verticies,
                       float scaleFactor)
{
    if (outerEdges.empty()) {
        return false;
    }
    vector<Point_t<int32_t> > points;
    addPoints(outerEdges, outerEdges[0].Start, points);
    vector<Point_t<float> > fPoints(points.size());

    // scale the points correctly.
    float scaleInv = 1.0f / scaleFactor;
    for(size_t i = 0; i < fPoints.size(); ++i) {
        fPoints[i].X = float(points[i].X) * scaleInv;
        fPoints[i].Y = float(points[i].Y) * scaleInv;
    }

    // scale the interior points
    vector<vector<Point_t<float> > > fInnerPoints(innerEdges.size());
    for(size_t i = 0; i < innerEdges.size(); ++i) {
        vector<Point_t<int32_t> > InnerPoints;
        addPoints(innerEdges[i], innerEdges[i][0].Start, InnerPoints);
        vector<Point_t<float> > & ref = fInnerPoints[i];
        ref.resize(InnerPoints.size());
        for(size_t i = 0; i < InnerPoints.size(); ++i) {
            ref[i].X = float(InnerPoints[i].X) * scaleInv;
            ref[i].Y = float(InnerPoints[i].Y) * scaleInv;
        }
    }
    
    // create a tesselator instance.
    TESStesselator * tess = tessNewTess(0);
    if (!tess) {
        return false;
    }
    // add the outer points
    tessAddContour(tess, 2, &fPoints[0], 8, (int) fPoints.size());
    // add the interior points
    for(size_t i = 0; i < fInnerPoints.size(); ++i) {
        tessAddContour(tess, 2, &fInnerPoints[i][0], 8, (int)fInnerPoints[i].size()); 
    }
    int res = tessTesselate(tess, TESS_WINDING_ODD, TESS_POLYGONS, 3, 2, NULL);
    if (res == 0) {
        tessDeleteTess(tess);
        return false;
    }

    const float* verts = tessGetVertices(tess);
    const int* elems = tessGetElements(tess);
    const int nverts = tessGetVertexCount(tess);
    const int nelems = tessGetElementCount(tess);

    // copy indicies data
    indicies.resize(nelems);
    if (nelems) {
        memcpy(&indicies[0], elems, nelems * sizeof(int) * 3);
    }
    // copy vertex data
    verticies.resize(nverts);
    if (nverts) {
        memcpy(&verticies[0], verts, nverts * sizeof(float) * 2);
    }
    tessDeleteTess(tess);
    return true;
}

static bool triangulate(const EdgeList_t & outerEdges,
                       std::vector<Triangle_t> & indicies,
                       std::vector<Point_t<float> > & verticies,
                       float scaleFactor)
{
    vector<EdgeList_t> inner;
    return triangulate(outerEdges, inner, indicies, verticies, scaleFactor);
}



void subdivide(float t0, float t1, Bezier<float> & bezier, 
               float a_MaxError, std::vector<Edge_t> & a_StraightEdges, size_t a_RecursiveCount)
{
    float mid = (t0 + t1) / 2.0f;
    Point_t<float> midBezier = bezier.Get(mid); /**< actual midpoint on Bezier */
    Point_t<float> p0 = bezier.Get(t0);
    Point_t<float> p1 = bezier.Get(t1);

    Point_t<float> diff = p1 - p0;
    Point_t<float> midLine = p0 + (diff * 0.5f); /**< midpoint on the current line segment */

    /** Calculate the error of this line segment using the Manhattan distance between the
        midpoint on the Bezier curve and the midpoint on the current line segment */
    float distance = fabs(midBezier.X - midLine.X) + fabs(midBezier.Y - midLine.Y);
        
    if ((distance <= a_MaxError) || /**< The error is below the limit */
        (a_RecursiveCount > MAX_SUBDIVISION_DEPTH)) { /**< Or we have reach the recursion depth limit */
        Edge_t edge;
        edge.StraightEdge = 1;
        /** round to integer twips */ 
        edge.Start  = Point_t<int32_t>(int32_t(p0.X), int32_t(p0.Y));
        edge.Stop   = Point_t<int32_t>(int32_t(p1.X), int32_t(p1.Y));
        a_StraightEdges.push_back(edge);
    } else { /**< Need to subdivide */
        subdivide(t0, mid, bezier, a_MaxError, a_StraightEdges, a_RecursiveCount + 1);
        subdivide(mid, t1, bezier, a_MaxError, a_StraightEdges, a_RecursiveCount + 1);
    }
}
    
/**
 * \brief   Subdivides a Quadratic Bezier curve to one or more straight edges using
 *      recursive subdivision. The subdivision will terminate when a error is below the
 *      max allowed error or the recursion depth limit has been reached.
 *
 * \param [in] a_Edge   The curved edge that defines the Quadratic Bezier curve. The
 *      edge is defined in twips (1/20th of a pixel).
 * \param [out] a_StraightEdges     The generated edges will be added to this list.
 * \param [in] a_MaxError           The max allowed error when subdividing a edge.
 */
void subdivideCurvedEdge(const Edge_t & a_Edge, std::vector<Edge_t> & a_StraightEdges, float a_MaxError)
{
    Bezier<float> bezier(Point_t<float>(float(a_Edge.Start.X), float(a_Edge.Start.Y)),
        Point_t<float>(float(a_Edge.Control.X), float(a_Edge.Control.Y)),
        Point_t<float>(float(a_Edge.Stop.X), float(a_Edge.Stop.Y)));

    subdivide(0.0f, 1.0f, bezier, a_MaxError, a_StraightEdges, 0);
}

static void generatePolyline(const std::vector<Edge_t> & a_Edges, 
                      std::vector<Edge_t> & a_StraightEdges,
                      float a_MaxError)
{
    for(size_t i = 0, num = a_Edges.size(); i < num; ++i) {
        if (a_Edges[i].StraightEdge) {
            a_StraightEdges.push_back(a_Edges[i]);
        } else {
            subdivideCurvedEdge(a_Edges[i], a_StraightEdges, a_MaxError);
        }
    }
}


static void mergeMeshes(list<pair<Material, gfx::Mesh> > & a_Meshes,const gfx::Mesh & a_Mesh)
{
    for(std::list<std::pair<Material, gfx::Mesh> >::iterator it = a_Meshes.begin();
        it != a_Meshes.end();
        it++)
    {
        if (it->first == a_Mesh.Material) {
            size_t initialCount = it->second.VertexData.size();
            std::copy(a_Mesh.VertexData.begin(), a_Mesh.VertexData.end(), std::back_inserter(it->second.VertexData));
            std::copy(a_Mesh.TextureCoords.begin(), a_Mesh.TextureCoords.end(), std::back_inserter(it->second.TextureCoords));

            for(size_t i = 0; i < a_Mesh.Indicies.size(); ++i)
            {
                gfx::Triangle_t tri;
                tri.p1 = a_Mesh.Indicies[i].p1 + static_cast<uint32_t>(initialCount);
                tri.p2 = a_Mesh.Indicies[i].p2 + static_cast<uint32_t>(initialCount);
                tri.p3 = a_Mesh.Indicies[i].p3 + static_cast<uint32_t>(initialCount);
                it->second.Indicies.push_back(tri);
            }
            return;
        }
    }
    a_Meshes.push_back(std::pair<Material, gfx::Mesh>(a_Mesh.Material, a_Mesh));
}


template<class T>
inline Point_t<float> Normalize(const Point_t<T> & a_Vec)
{
    Point_t<float> res;
    float length = sqrtf(float(a_Vec.X * a_Vec.X + a_Vec.Y * a_Vec.Y));
    res.X = float(a_Vec.X) / length;
    res.Y = float(a_Vec.Y) / length;
    return res;
}

static void getNormal(const gfx::Point_t<int32_t> & a_Start, 
                      const gfx::Point_t<int32_t> & a_End,
                      gfx::Point_t<float> & a_Normal)
{
    int32_t dx = a_End.X - a_Start.X;
    int32_t dy = a_End.Y - a_Start.Y;
    a_Normal = Normalize(gfx::Point_t<int32_t>(-dy, dx));
}

static void getEdgeCorners(const gfx::Point_t<int32_t> & a_P1, 
                           const gfx::Point_t<int32_t> & a_P2,
                           gfx::Point_t<float> a_Corners[4],
                           float a_HalfWidth)
{
    gfx::Point_t<float> normal;
    getNormal(a_P1, a_P2, normal);
    normal = normal * a_HalfWidth;
    a_Corners[0] = Point_t<float>(a_P1.X - normal.X, a_P1.Y - normal.Y) * (1.0f/20.0f);
    a_Corners[1] = Point_t<float>(a_P1.X + normal.X, a_P1.Y + normal.Y) * (1.0f/20.0f);
    a_Corners[2] = Point_t<float>(a_P2.X + normal.X, a_P2.Y + normal.Y) * (1.0f/20.0f);
    a_Corners[3] = Point_t<float>(a_P2.X - normal.X, a_P2.Y - normal.Y) * (1.0f/20.0f);
}

static void GetStrokeMesh(const std::vector<Edge_t> & a_Edges, 
                          const LineStyle_t & a_LineStyle, 
                          bool a_IsPathClosed,
                          gfx::Mesh & a_Mesh)
{
    uint32_t numEdges = static_cast<uint32_t>(a_Edges.size());
    a_Mesh.VertexData.resize(4 * numEdges); /**< make room for the verticies */
    size_t indiciesIndex = 0;
    for(uint32_t i = 0, num = numEdges; i < num; ++i)
    {
        gfx::Point_t<float> corners[4];
        const Edge_t & edge = a_Edges[i];
        /** calculate the corner positions of the rectangle defined by the edge */
        getEdgeCorners(edge.Start, edge.Stop, &a_Mesh.VertexData[i*4], float(a_LineStyle.LineWidth)/ 2.0f);
        uint32_t base = i * 4;
        a_Mesh.Indicies.push_back(Triangle_t(base, base + 3, base + 1));
        a_Mesh.Indicies.push_back(Triangle_t(base + 1, base + 3, base + 2));

        if (i != (numEdges - 1)) {
            /** this is not the last one, handle the join between this edge and the next one */
            a_Mesh.Indicies.push_back(Triangle_t(base + 3, base + 4, base + 2));
            a_Mesh.Indicies.push_back(Triangle_t(base + 2, base + 3, base + 5));
        }
        if ((i == (numEdges - 1)) && (a_IsPathClosed)) {
            /** this is a last one in a closed path */
            a_Mesh.Indicies.push_back(Triangle_t(base + 3, 0, base + 2));
            a_Mesh.Indicies.push_back(Triangle_t(base + 2, base + 3, 1));
        }
    }
}

}
}