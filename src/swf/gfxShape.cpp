#include "gfxShape.h"
#include <memory>
#include "swfTypes.h"
#include <map>
#include <list>
#include "intersection.h"

using namespace std;

namespace swf
{
namespace gfx
{

/*****************************************************************************/
/*                                  Forward declarations                     */
/*****************************************************************************/
void tesselatePath(const std::vector<Edge_t> & _edges, 
                   const std::vector<swf::gfx::LineStyle_t> & _lineStyles,
                   const std::vector<swf::gfx::FillStyle_t> & _fillStyles,
                   Shape_t & _shape);

void checkOverlaps(gfx::Shape_t & a_Shape);
void swfFillToGfxFill(const swf::FillStyle & a_Style, gfx::FillStyle_t & a_FillStyle);
void swfLineStyleToGfx(const swf::LineStyle & a_Style,gfx::LineStyle_t & a_LineStyle);
bool TestPointTwips(const Shape_t & a_Shape, Point_t<int32_t> & a_Point);

/*****************************************************************************/
/*                                  Definitions                              */
/*****************************************************************************/

bool Shape_t::isPointInside(gfx::Point_t<int32_t> & point) const
{
    return TestPointTwips(*this, point);
}

/**
 * Translates a set of shape records to a set of edges that define a shape.
 */
void recordsToEdges(const std::vector<ShapeRecord> records,
                    std::vector<Edge_t> & edges)
{
    Point_t<int32_t> currentPos(0,0);
    int32_t lineStyle = -1, fillStyle0 = -1, fillStyle1 = -1;
    for(size_t i = 0, numRecords = records.size(); i < numRecords; ++i) {
        const ShapeRecord & record = records[i];
        switch(record.m_RecordType) {
        case ShapeRecord::STYLE_CHANGE_RECORD:
            {
                if (record.u.StyleChanged.StateMoveTo) {
                    currentPos = gfx::Point_t<int32_t>(
                        record.u.StyleChanged.MoveDeltaX, record.u.StyleChanged.MoveDeltaY);
                }
                if (record.u.StyleChanged.StateFillStyle0) {
                    fillStyle0 = record.u.StyleChanged.FillStyle0;
                }
                if (record.u.StyleChanged.StateFillStyle1) {
                    fillStyle1 = record.u.StyleChanged.FillStyle1;
                }
                if (record.u.StyleChanged.StateLineStyle) {
                    lineStyle = record.u.StyleChanged.LineStyle;
                }
                break;
            }
        case ShapeRecord::STRAIGHT_EDGE_RECORD: /**< straight edge */
            {
                Edge_t edge;
                edge.StraightEdge   = 1;/**< This is a straight edge */
                edge.Start          = currentPos;
                edge.Stop           = currentPos + gfx::Point_t<int32_t>(
                    record.u.StraightEdge.DeltaX, record.u.StraightEdge.DeltaY);
                currentPos              = edge.Stop;
                edge.FillStyle0         = fillStyle0;
                edge.FillStyle1         = fillStyle1;
                edge.LineStyle          = lineStyle;
                edges.push_back(edge);
            }
            break;
        case ShapeRecord::CURVED_EDGE_RECORD: /** Quadratic Bezier curve */
            {
                Edge_t edge;
                edge.StraightEdge   = 0;/**< This is not a straight edge */
                edge.Start          = currentPos;
                edge.Control        = currentPos + gfx::Point_t<int32_t>(
                    record.u.CurvedEdge.ControlDeltaX, record.u.CurvedEdge.ControlDeltaY);
                edge.Stop           = edge.Control + gfx::Point_t<int32_t>(
                    record.u.CurvedEdge.AnchorDeltaX, record.u.CurvedEdge.AnchorDeltaY);
                currentPos              = edge.Stop;
                edge.FillStyle0         = fillStyle0;
                edge.FillStyle1         = fillStyle1;
                edge.LineStyle          = lineStyle;
                edges.push_back(edge);
                break;
            }
        }
    }
}

/**
 * Creates a gfx::Shape_t instance from a SWF shape.
 */
std::shared_ptr<gfx::Shape_t> CreateShape(const swf::DefinedShape4 & swfShape)
{
    std::shared_ptr<gfx::Shape_t> shape = std::make_shared<gfx::Shape_t>();
    const ShapeWithStyle & ref = swfShape.Shapes;

    shape->Glyph = false;

    size_t numFills     = swfShape.Shapes.FillStyles.size();
    size_t numStyles    = swfShape.Shapes.LineStyles.size();
    // translate linestyle and fillstyle
    vector<gfx::FillStyle_t> FillStyles(numFills);
    vector<gfx::LineStyle_t> LineStyles(numStyles);

    for(size_t i = 0; i < numFills; ++i) {
        swfFillToGfxFill(swfShape.Shapes.FillStyles[i], FillStyles[i]);
    }
    for(size_t i = 0; i < numStyles; ++i) {
        swfLineStyleToGfx(swfShape.Shapes.LineStyles[i], LineStyles[i]);
    }

    // translate shape records to actual edges.
    vector<Edge_t> edges;
    recordsToEdges(swfShape.Shapes.Shapes, edges);
    // now "tesselate" the path
    tesselatePath(edges, LineStyles, FillStyles, *shape);
    shape->UserData         = -1;
    shape->ScaleFactor      = 20;
    shape->Bounds.left      = swfShape.ShapeBounds.Xmin;
    shape->Bounds.right     = swfShape.ShapeBounds.Xmax;
    shape->Bounds.top       = swfShape.ShapeBounds.Ymin;
    shape->Bounds.bottom    = swfShape.ShapeBounds.Ymax;
    return shape;
}

inline Edge_t reverseEdge(const Edge_t & a_Edge)
{
    Edge_t edge;
    edge.Control        = a_Edge.Control;
    edge.Start          = a_Edge.Stop;
    edge.Stop           = a_Edge.Start;
    edge.StraightEdge   = a_Edge.StraightEdge;
    return edge;
}

bool isCCW(const std::vector<Edge_t> & a_Edges)
{
    int32_t sum = 0;
    for(size_t i = 0, count = a_Edges.size(); i < count; ++i)
    {
        const Point_t<int32_t> & current = a_Edges[i].Start;
        const Point_t<int32_t> & next = (i == (count - 1)) ? a_Edges[0].Start : a_Edges[i + 1].Start;
        sum += (current.X * next.Y - next.X * current.Y);
    }
    return sum < 0;
}

void tesselatePath(const std::vector<Edge_t> & edges, 
                   const std::vector<swf::gfx::LineStyle_t> & lineStyles,
                   const std::vector<swf::gfx::FillStyle_t> & fillStyles,
                   Shape_t & shape)
{
    std::map<int16_t, std::list<Edge_t> > SortedEdges;
    for(size_t i = 0, num = edges.size(); i < num; ++i)
    {
        const Edge_t & edge = edges[i];
        if ((edge.FillStyle1 > 0) && (edge.FillStyle0 < 1)) { // We only have a right fillstyle.
            SortedEdges[edge.FillStyle1].push_back(edge);
        } else if ((edge.FillStyle1 < 1) && (edge.FillStyle0 > 0)) {
            // We only have a left fillstyle, convert it to a edge with a right
            Edge_t reversed = reverseEdge(edge);
            reversed.LineStyle      = edge.LineStyle;
            reversed.FillStyle0     = -1;
            reversed.FillStyle1     = edge.FillStyle0;
            SortedEdges[reversed.FillStyle1].push_back(reversed);
        } else if ((edge.FillStyle0 > 0) && (edge.FillStyle1 > 0)) {
            // We have two different fill styles, create two distinct edges,
            // first the right edge.
            Edge_t rightEdge = edge;
            rightEdge.FillStyle0     = -1;
            rightEdge.FillStyle1     = edge.FillStyle1;
            rightEdge.LineStyle = edge.LineStyle;
            SortedEdges[rightEdge.FillStyle1].push_back(rightEdge);
            // now create the left edge.
            Edge_t leftEdge = reverseEdge(edge);
            leftEdge.LineStyle  = edge.LineStyle;
            leftEdge.FillStyle1 = edge.FillStyle0;
            leftEdge.FillStyle0 = -1;
            SortedEdges[leftEdge.FillStyle1].push_back(leftEdge);
        } else { // No fill style.
            SortedEdges[0].push_back(edge);
        }
    }
    // Now find the connected edges in the different sorted slots
    for(map<int16_t, list<Edge_t> >::iterator it = SortedEdges.begin();
        it != SortedEdges.end();
        it++)
    {
        list<Edge_t> & edgeList = it->second;
        while(!edgeList.empty()) 
        {
            Edge_t current = edgeList.front(); edgeList.pop_front();
            gfx::Path_t path;
            path.Edges.push_back(current);  /**< add the starting edge */
            bool foundConnectedEdge;

            Point_t<int32_t> setStart(current.Start);
            Point_t<int32_t> setStop(current.Stop);

            path.ClosedPath = 0;
            do {
                foundConnectedEdge = false;

                for(list<Edge_t>::iterator testedEdge = edgeList.begin();
                    testedEdge != edgeList.end();
                    testedEdge++)
                {
                    if (testedEdge->Stop == setStart) {
                        /** prepend this edge */
                        path.Edges.insert(path.Edges.begin(), *testedEdge);
                        /** check if the edge closes the path */
                        if (testedEdge->Start == setStop) {
                            foundConnectedEdge = false;
                            path.ClosedPath = 1;
                        } else {
                            foundConnectedEdge = true;
                        }
                        setStart = testedEdge->Start;
                        edgeList.erase(testedEdge);
                        break;
                    } else if (testedEdge->Start == setStop) {
                        /** Append this edge */
                        path.Edges.push_back(*testedEdge);      /**< Add the connected edge */
                        foundConnectedEdge = true;
                        setStop = testedEdge->Stop;
                        edgeList.erase(testedEdge);
                        break;
                    }
                }
            } while(foundConnectedEdge);
            /** Set the linestyle of the path */
            if (current.LineStyle < 1) {
                path.LineStyle.LineWidth = 0;
            } else {
                path.LineStyle = lineStyles[current.LineStyle - 1];
            }
            /** Set the fillstyle of the path */
            if (it->first < 1) {
                path.FillStyle.FillType = FillStyle_t::NoFill;
            } else if ((it->first - 1) < (int16_t) fillStyles.size()) {
                path.FillStyle = fillStyles[it->first-1];
            } else {
                throw std::runtime_error("FillStyle out of bounds.");
            }
            path.CCW = isCCW(path.Edges) ? 1 : 0;
            shape.Paths.push_back(path);
        }
    }
    checkOverlaps(shape);
}

/**
 * \brief   Translates a swf::FillStyle to a gfx::FillStyle.
 */
void swfFillToGfxFill(const swf::FillStyle & a_Style,
    gfx::FillStyle_t & a_FillStyle)
{
    switch(a_Style.FillType) 
    {
    case swf::FillStyle::eSOLID_RGBA:
        a_FillStyle.FillType    = gfx::FillStyle_t::SolidFill;
        a_FillStyle.u.Color.r   = a_Style.color.rgba.r;
        a_FillStyle.u.Color.g   = a_Style.color.rgba.g;
        a_FillStyle.u.Color.b   = a_Style.color.rgba.b;
        a_FillStyle.u.Color.a   = a_Style.color.rgba.a;
        break;
    case swf::FillStyle::eLINEAR_GRADIENT:
    case swf::FillStyle::eRADIAL_GRADIENT:
        if (a_Style.FillType == swf::FillStyle::eLINEAR_GRADIENT) {
            a_FillStyle.FillType = gfx::FillStyle_t::LinearGradient;
        } else {
            a_FillStyle.FillType = gfx::FillStyle_t::RadialGradient;
        }
        a_FillStyle.u.gradient.ControlPoints    = a_Style.LinearGradient.numControlPoints;
        for(size_t i = 0; i < a_FillStyle.u.gradient.ControlPoints; ++i) {
            const swf::Color_t & color = a_Style.LinearGradient.controlPoint[i].color;
            a_FillStyle.u.gradient.points[i].Ratio = a_Style.LinearGradient.controlPoint[i].ratio;
            a_FillStyle.u.gradient.points[i].Color.r = color.rgba.r;
            a_FillStyle.u.gradient.points[i].Color.g = color.rgba.g;
            a_FillStyle.u.gradient.points[i].Color.b = color.rgba.b;
            a_FillStyle.u.gradient.points[i].Color.a = color.rgba.a;
        }
        break;
    default:
        break;
    }
}

void swfLineStyleToGfx(const swf::LineStyle & a_Style,
    gfx::LineStyle_t & a_LineStyle)
{
    switch(a_Style.type) 
    {
    case swf::LineStyle::LINESTYLE1:
        a_LineStyle.LineWidth = a_Style.LineStyle1.width;
        a_LineStyle.Color.r = a_Style.LineStyle1.color.rgba.r;
        a_LineStyle.Color.g = a_Style.LineStyle1.color.rgba.g;
        a_LineStyle.Color.b = a_Style.LineStyle1.color.rgba.b;
        a_LineStyle.Color.a = a_Style.LineStyle1.color.rgba.a;
        break;
    case swf::LineStyle::LINESTYLE2:
        a_LineStyle.LineWidth = a_Style.Linestyle2.Width;
        a_LineStyle.Color.r = a_Style.Linestyle2.Color.rgba.r;
        a_LineStyle.Color.g = a_Style.Linestyle2.Color.rgba.g;
        a_LineStyle.Color.b = a_Style.Linestyle2.Color.rgba.b;
        a_LineStyle.Color.a = a_Style.Linestyle2.Color.rgba.a;
        break;
    }
}

void checkOverlaps(gfx::Shape_t & a_Shape)
{
    size_t numPaths = a_Shape.Paths.size();
    for(size_t current = 0; current < numPaths; ++current) { /**< for each path */
        bool isInsideAnother = false;
        for(size_t tested = 0; tested < numPaths; ++tested) { /**< test against all other paths */
            if (current != tested) { /**< test if the current one is inside the tested one */
                if (a_Shape.Paths[tested].ClosedPath) {
                    if (isPathInside(a_Shape.Paths[tested].Edges, a_Shape.Paths[current].Edges)) {
                        /** The current one is inside the tested one */
                        a_Shape.Paths[tested].InnerPaths.push_back(current);
                        isInsideAnother = true;
                    }
                }
            }
        }
        if (!isInsideAnother) { 
            /**< the current shape is not inside another one, so it's a outer shape */
            a_Shape.OuterPaths.push_back(current);
        }
    }
}

}
}