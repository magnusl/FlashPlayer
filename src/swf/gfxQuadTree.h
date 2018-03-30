#include <memory>
#include "gfxRect.h"

namespace swf
{
namespace gfx
{

template <class Item, size_t ItemLimit = 4, class Type = float>
class QuadTree
{
protected:
    typedef std::pair<Item, gfx::Rect_t<Type> > NodeItem;
    struct Node {
        std::list<NodeItem>     m_Items;
        gfx::Rect_t<Type>       m_Boundry;
        std::shared_ptr<Node>   m_Children[4];
    };

    bool split(std::shared_ptr<Node> a_Node) {
        std::list<Item> ignored;
        Type halfWidth  = (a_Node->m_Boundry.right - a_Node->m_Boundry.left) / 2;
        Type halfHeight = (a_Node->m_Boundry.bottom - a_Node->m_Boundry.top) / 2;

        const Rect_t<Type> & rect = a_Node->m_Boundry;

        for(size_t i = 0; i < 4; ++i) {
            a_Node->m_Children[i] = std::make_shared<Node>();
        }

        a_Node->m_Children[0]->m_Boundry = Rect_t<Type>(rect.left, rect.left + halfWidth, rect.top, rect.top + halfHeight);
        a_Node->m_Children[1]->m_Boundry = Rect_t<Type>(rect.left + halfWidth, rect.right, rect.top, rect.top + halfHeight);
        a_Node->m_Children[2]->m_Boundry = Rect_t<Type>(rect.left, rect.left + halfWidth, rect.top + halfHeight, rect.bottom);
        a_Node->m_Children[3]->m_Boundry = Rect_t<Type>(rect.left + halfWidth, rect.right, rect.top + halfWidth, rect.bottom);

        for(std::list<NodeItem>::iterator it = a_Node->m_Items.begin(); 
            it != a_Node->m_Items.end(); 
            it++)
        {
            for(size_t i = 0; i < 4; ++i) {
                if (it->second.inside(a_Node->m_Children[i]->m_Boundry) == 1) {
                    a_Node->m_Children[i]->m_Items.push_back(*it);
                    it = a_Node->m_Items.erase(it);
                    if (it == a_Node->m_Items.end()) {
                        return true; /**< no more items, just return */
                    }
                    break; /**< continue with next item */
                }
            }
        }
        return true;
    }

    bool insert(std::shared_ptr<Node> a_Node,
        const gfx::Rect_t<Type> & a_Boundry, 
        const Item & a_Item, 
        std::list<Item> & a_Overlaps)
    {
        for(std::list<NodeItem>::iterator it = a_Node->m_Items.begin(); 
            it != a_Node->m_Items.end(); 
            it++)
        {
            if (it->second.inside(a_Boundry) >= 0) {
                a_Overlaps.push_back(it->first);
            }
        }
        /** now check were we should place the item. If the new item is completely inside a child
            then it should be added to the child, otherwise it will be added to the current node */
        for(size_t i = 0; i < 4; ++i) {
            if (a_Node->m_Children[i]) {
                int res = a_Node->m_Children[i]->m_Boundry.inside(a_Boundry);
                if (res == 1) { /**< completely inside this child */
                    return insert(a_Node->m_Children[i], a_Boundry, a_Item, a_Overlaps);
                }
            }
        }
        if (!a_Node->m_Children[0]) { /**< We don't children, split */
            if (!split(a_Node)) {
                return false;
            }
            for(size_t i = 0; i < 4; ++i) {
                if (a_Node->m_Children[i]) {
                    int res = a_Node->m_Children[i]->m_Boundry.inside(a_Boundry);
                    if (res == 1) { /**< completely inside this child */
                        return insert(a_Node->m_Children[i], a_Boundry, a_Item, a_Overlaps);
                    }
                }
            }
        }
        a_Node->m_Items.push_back(std::pair<Item, gfx::Rect_t<Type> >(a_Item, a_Boundry));
        return true;
    }

public:
    QuadTree(const gfx::Rect_t<Type> & a_Boundry) : m_Boundry(a_Boundry)
    {
    }

    bool insert(const gfx::Rect_t<Type> & a_Boundry, 
        const Item & a_Item, 
        std::list<Item> & a_Overlaps)
    {
        if (!m_Node) {
            m_Node = std::make_shared<Node>();
            m_Node->m_Boundry = m_Boundry;
        }
        return insert(m_Node, a_Boundry, a_Item, a_Overlaps);
    }

    void reset()
    {
        m_Node = std::shared_ptr<Node>();
    }

private:
    std::shared_ptr<Node>   m_Node;
    gfx::Rect_t<Type>       m_Boundry;
};


}
}