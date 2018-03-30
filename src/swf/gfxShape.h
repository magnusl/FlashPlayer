#ifndef _GFX_H_
#define _GFX_H_

#include "gfxPoint.h"
#include <vector>
#include <stdint.h>
#include <memory>
#include "swfTypes.h"
#include "gfxRect.h"

namespace swf
{

namespace gfx
{

struct Color_t
{
    uint8_t r,g,b,a;
};

/**
 * Defines the fill style for a path
 */
struct FillStyle_t {
    enum {
        NoFill,         /**< Path does not have a fill */
        SolidFill,      /**< Path is filled with a solid RGBA color */
        BitmapFill,     /**< Path is filled with a bitmap */
        LinearGradient, /**< Path is filled with a linear gradient */
        RadialGradient
    } FillType;

    union {
        uint32_t    BitmapId;   /**< Id for bitmap fill */
        Color_t     Color;      /**< Color for solid fill */
        struct {
            uint8_t ControlPoints;
            struct {
                Color_t     Color;
                uint8_t     Ratio;
            } points[16];
        } gradient;
    } u;
};

/**
 * Defines the line style for a path.
 */
struct LineStyle_t
{
    Color_t     Color;      /**< Line color */
    uint16_t    LineWidth;  /**< Line width, in twips */
};

/**
 * A straight or curved edge.
 */
struct Edge_t
{
    Point_t<int32_t>    Start, Control, Stop;
    uint8_t             StraightEdge : 1;
    int32_t             FillStyle0;
    int32_t             FillStyle1;
    int32_t             LineStyle;
};

typedef std::vector<Edge_t> EdgeList_t;

/**
 *
 */
struct Path_t
{
    std::vector<Edge_t>         Edges;          /**< A Path has one or more edges */
    std::vector<size_t>         InnerPaths;     /**< indicies to the shape's path */
    swf::gfx::LineStyle_t       LineStyle;      /**< The line style for this path */
    swf::gfx::FillStyle_t       FillStyle;      /**< The fill style for this path */

    uint8_t                     ClosedPath : 1; /**< Indicates if the path is closed or open */
    uint8_t                     CCW;            /**< Indicates if the path is CCW */
};

/**
 * Represents a path which is defined by one or more paths.
 */
struct Shape_t
{
    std::vector<size_t> OuterPaths;
    std::vector<Path_t> Paths;
    int                 ScaleFactor;
    int                 UserData;   // user data, default value is -1.
    bool                Glyph;
    Rect_t<int32_t>     Bounds;

    bool isPointInside(gfx::Point_t<int32_t> &) const;
};

/*****************************************************************************/
/*                                  Functions                                */
/*****************************************************************************/

std::shared_ptr<gfx::Shape_t> CreateShape(const swf::DefinedShape4 & swfShape);

void tesselatePath(const std::vector<Edge_t> & _edges, 
                   const std::vector<swf::gfx::LineStyle_t> & _lineStyles,
                   const std::vector<swf::gfx::FillStyle_t> & _fillStyles,
                   Shape_t & _shape);

void recordsToEdges(const std::vector<ShapeRecord> records,
                    std::vector<Edge_t> & edges);

}

}

#endif