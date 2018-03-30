#include "gfxShape.h"

namespace swf
{

namespace gfx
{
    /**
     * Check if a horizontal line starting from a_Point intersects with the straight edge.
     */
    bool intersectsStraightEdge(const Point_t<int32_t> & a_Point,
        const Edge_t & a_Edge,
        Point_t<int32_t> & a_IntersectionPoint)
    {
        int32_t ymin = std::min(a_Edge.Start.Y, a_Edge.Stop.Y), 
            ymax = std::max(a_Edge.Start.Y, a_Edge.Stop.Y);
        if (a_Point.Y >= ymin && a_Point.Y <= ymax) /**< overlaps Y */
        {
            if (a_Edge.Start.Y == a_Edge.Stop.Y) {
                int xmax = std::max(a_Edge.Start.X, a_Edge.Stop.X);
                /** TODO, calculate the distance */
                return xmax >= a_Point.X;
            } else {
                /** check where the line intersects the Y axis */
                int32_t dy = a_Edge.Stop.Y - a_Edge.Start.Y, dx = a_Edge.Stop.X - a_Edge.Start.X;
                float t             = float(a_Point.Y - a_Edge.Start.Y) / dy;
                float x             = float(a_Edge.Start.X) + dx * t;
                a_IntersectionPoint = Point_t<int32_t>(int32_t(x), a_Point.Y);

                return (x >= a_Point.X);
            }
        }
        return false;
    }

    /**
     * Counts how many times a horizontal line starting from a_Point 
     * intersects a quadratic bezier curve.
     */
    int IntersectionCountWithCurve(const Point_t<int32_t> & a_Point,
        const Point_t<int32_t> & a_Start, const Point_t<int32_t> & a_Control, const Point_t<int32_t> & a_Stop)
    {
        int     count = 0;
        int     a = a_Start.Y - (2 * a_Control.Y) + a_Stop.Y;
        int     b = (2 * a_Control.Y) - (2 * a_Start.Y);
        int     c = a_Start.Y - a_Point.Y;

        if (a == 0) { /**< single solution */
            if (b != 0) {
                float t = -c / float(b);
                if ((t >= 0.0f) && (t <= 1.0f)) { /**< check if root is inside [0, 1] range */
                    float dt = 1.0f - t;
                    float x = dt*dt*float(a_Start.X) + (2 * dt * t * float(a_Control.X)) + (t * t * float(a_Stop.X));
                    if (x >= a_Point.X) {
                        return 1; /**< ray intersects curve */
                    }
                }
            }
            return 0;
        }
        int fac = (b*b) - (4*a*c);
        if (fac < 0) { /**< can't take the square root of a negative number */
            return 0;
        }
        /** Calculate the roots of the quadratic equation */
        float root1 = (-b + sqrtf(float(fac))) / float(2*a);
        float root2 = (-b - sqrtf(float(fac))) / float(2*a);

        /** The roots are valid if the lie in the [0, 1] range */
        if (root1 >= 0.0f && root1 <= 1.0f) { /**< first root is valid */
            float dt = 1.0f - root1;
            float x = dt*dt*float(a_Start.X) + (2 * dt * root1 * float(a_Control.X)) + (root1 * root1 * float(a_Stop.X));
            if (x >= a_Point.X) {
                ++count;
            }
        }
        if (root2 >= 0.0f && root2 <= 1.0f) { /**< second root is valid */
            float dt = 1.0f - root2;
            float x = dt*dt*float(a_Start.X) + (2 * dt * root2 * float(a_Control.X)) + (root2 * root2 * float(a_Stop.X));
            if (x >= a_Point.X) {
                ++count;
            }
        }
        return count;
    }

    /**
     * Counts how many times a horizontal line starting at the point intersects a edge.
     */
    size_t countIntersections(const Point_t<int32_t> & a_Point, const Edge_t & a_Edge)
    {
        /** handle the corner case that the ray intersects the vertex between to edges,
            in this case just ignore the intersection with the stop edge */
        if ((a_Point.Y == a_Edge.Stop.Y) && (a_Point.X < a_Edge.Stop.X)) {
            return 0;
        }
        Point_t<int32_t> intersectionPoint;
        if (a_Edge.StraightEdge) {
            return intersectsStraightEdge(a_Point, a_Edge, intersectionPoint) ? 1 : 0;
        } else {
            return IntersectionCountWithCurve(a_Point, 
                a_Edge.Start, a_Edge.Control, a_Edge.Stop);
        }
    }

    /**
     * Checks if the point is inside the closed path defined by the supplied edges.
     * Uses the odd-even rule to determine if a shape is inside or outside.
     *
     * \param [in] a_Point  The point to check if it's inside.
     * \param [in] a_Edges  The edges defining a closed shape.
     */
    bool isPointInside(const Point_t<int32_t> & a_Point, const std::vector<Edge_t> & a_Edges)
    {
        size_t count = 0;
        for(size_t i = 0, num = a_Edges.size(); i < num; ++i) {
            count += countIntersections(a_Point, a_Edges[i]);
        }
        if (count & 1) { /**< lowest bit is set, so the count is odd */
            return true;
        } else {
            return false;
        }
    }

    /**
     * Checks that the shape defined by the a_OuterEdges fully enclose the shape defined
     * by the a_InnerEdges. This is a very simple test that only tests that the Start and Stop points
     * are inside. This should be sufficient to determine if two SWF shapes overlap, since the Flash toolset 
     * doesn't generate overlapping paths.
     */
    bool isPathInside(const std::vector<Edge_t> & a_OuterEdges, 
        const std::vector<Edge_t> & a_InnerEdges)
    {
        // for all the inner edges.
        for(size_t innerEdge = 0, numInner = a_InnerEdges.size(); 
            innerEdge < numInner;
            ++innerEdge)
        {
            // check that the Start, Control and Stop point is inside the a_OuterEdges.
            if (!isPointInside(a_InnerEdges[innerEdge].Start, a_OuterEdges) ||
                !isPointInside(a_InnerEdges[innerEdge].Stop, a_OuterEdges)) 
            {
                return false;   
            }
        }
        return true;
    }

    /**
     * Tests if a point (specified in Twips) is inside the shape.
     */
    bool TestPointTwips(const Shape_t & a_Shape, Point_t<int32_t> & a_Point)
    {
        if (a_Shape.OuterPaths.empty()) { /**< Nothing to test with, cannot be inside */
            return false;
        }
        // A shape can have more than one actual shapes.
        const std::vector<size_t> & outer = a_Shape.OuterPaths;
        for(size_t i = 0, num = outer.size(); i < num; ++i)
        {
            const Path_t & path = a_Shape.Paths[outer[i]];
            if (path.ClosedPath) /**< This is a closed path */
            {
                if (isPointInside(a_Point, path.Edges)) /**< inside this outer shape */
                {
                    // TODO: Test with inner shapes.
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Tests if a point (specified in pixels) is inside the shape.
     */
    bool TestPoint(const Shape_t & a_Shape, Point_t<float> & a_Point)
    {
        Point_t<int32_t> twipsPoint((int32_t)a_Point.X * 20,
            (int32_t) (a_Point.Y * 20));
        return TestPointTwips(a_Shape, twipsPoint);
    }

}

}