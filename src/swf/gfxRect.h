#ifndef _GFXRECT_H_
#define _GFXRECT_H_

#include <cmath>
#include <Eigen\Dense>

namespace swf
{
namespace gfx
{

template<class T>
struct Rect_t {
    Rect_t()
    {}

    Rect_t(T a_Left, T a_Right, T a_Top, T a_Bottom) :
        left(a_Left), right(a_Right), top(a_Top), bottom(a_Bottom)
    {}

    Rect_t<T> concatenate(const Rect_t<T> & a_Rhs) {
        Rect_t<T> res;
        res.left    = std::min(left, a_Rhs.left);
        res.top     = std::min(top, a_Rhs.top);
        res.right   = std::max(right, a_Rhs.right);
        res.bottom  = std::max(bottom, a_Rhs.bottom);
        return res;
    }

    gfx::Rect_t<T> Transform(const Eigen::Matrix3f & a_Transformation) const {
        gfx::Rect_t<float> transformedBoundry;
        Eigen::Vector3f topLeft2(left, top, 1);
        Eigen::Vector3f topRight2(right,top, 1);
        Eigen::Vector3f bottomLeft2(left, bottom, 1);
        Eigen::Vector3f bottomRight2(right, bottom, 1);

        Eigen::Vector3f topLeft     = a_Transformation * topLeft2;
        Eigen::Vector3f topRight    = a_Transformation * topRight2;
        Eigen::Vector3f bottomLeft  = a_Transformation * bottomLeft2;
        Eigen::Vector3f bottomRight = a_Transformation * bottomRight2;

        /** construct the boundry based on the new extreme values */
        transformedBoundry.left = 
            std::min(topLeft.x(), std::min(topRight.x(), std::min(bottomRight.x(), bottomLeft.x())));
        transformedBoundry.right = 
            std::max(topLeft.x(), std::max(topRight.x(), std::max(bottomRight.x(), bottomLeft.x())));
        transformedBoundry.top 
            = std::min(topLeft.y(), std::min(topRight.y(), std::min(bottomRight.y(), bottomLeft.y())));
        transformedBoundry.bottom 
            = std::max(topLeft.y(), std::max(topRight.y(), std::max(bottomRight.y(), bottomLeft.y())));

        return transformedBoundry;
    }


    /**
     * \brief   Checks if the rectangle overlaps the current one.
     *
     * \param [in] a_Rhs    The rectangle to test against the 'this' rectangle.
     *
     * \return  A value indicating if the rectangles overlap
     * \retval  -1  Does not overlap
     * \retval  1   The supplied rectangle is completely inside this one.
     * \retval  0   The rectangles overlap.
     */
    int inside(const Rect_t<T> & a_Rhs) const {
        /**< check X axis */
        if (a_Rhs.right <= left) return -1;
        if (a_Rhs.left >= right) return -1;
        if (a_Rhs.bottom <= top) return -1;
        if (a_Rhs.top >= bottom) return -1;

        if ((a_Rhs.left >= left) && (a_Rhs.right <= right) &&
            (a_Rhs.top >= top) && (a_Rhs.bottom <= bottom)) 
        {
            return 1;
        }
        return 0;
    }

    T left, right, top, bottom;
};

}
}

#endif