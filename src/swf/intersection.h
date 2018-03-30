#include <vector>
#include "gfxShape.h"

namespace swf
{
namespace gfx
{

bool isPathInside(const std::vector<Edge_t> & a_OuterEdges, const std::vector<Edge_t> & a_InnerEdges);
bool isPointInside(const Point_t<int32_t> & a_Point, const std::vector<Edge_t> & a_Edges);
bool TestPointTwips(const Shape_t & a_Shape, Point_t<int32_t> & a_Point);

}
}