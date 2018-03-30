#ifndef _DEPGRAPH_H_
#define _DEPGRAPH_H_

#include <list>
#include <vector>
#include <memory>

namespace swf
{
template<class T>
struct DependencyNode
{
    DependencyNode(const T & a_Value) : Value(a_Value), DepCount(0), CurrentCount(0)
    {}

    void AddDependency(DependencyNode * a_Node)
    {
        // This node is a successor to out dependency
        a_Node->Successors.push_back(this);
        ++DepCount;
        ++CurrentCount;
    }
        
    void ResetCount() { CurrentCount = DepCount; }

    T                                   Value;          /**< The value associated with this node */
    int                                 DepCount;       /**< The number of nodes this node depends on */
    int                                 CurrentCount;   /**< Dynamic updated count */
    std::vector<DependencyNode<T> *>    Successors;     /**< The successor nodes */
};

template<class T>
class ITopologicalSorter_Result
{
public:
    virtual void OnResultSet(
        const std::list<swf::DependencyNode<T> * > & a_Result) = 0;
};

template<class Type>
void TopologicalSort(std::vector<std::shared_ptr<DependencyNode<Type> > > & Nodes, 
                     ITopologicalSorter_Result<Type> & Result)
{
    std::list<DependencyNode<Type> *> availableNodes;
        
    // The first step is to iterate over all the nodes to fine the ones which doesn't
    // have any dependecies to other nodes.
    std::for_each(Nodes.begin(), Nodes.end(), 
        [&availableNodes](std::shared_ptr<DependencyNode<Type> > pNode) {
        if (pNode && (pNode->DepCount == 0)) {
            availableNodes.push_back(pNode.get());
        }
    });

    static int k = 0;
    // continue until we reached the end  of the graph
    while(!availableNodes.empty()) {
        // The items on the list should now we sorted and added to the render queue.
        Result.OnResultSet(availableNodes);

        // decrement the successors.
        for(size_t i = 0, count = availableNodes.size(); i < count; ++i) {
            DependencyNode<Type> * Node = availableNodes.front(); 
            availableNodes.pop_front();

            for(auto it = Node->Successors.begin(); 
                it != Node->Successors.end(); 
                it++) 
            {
                if ((--(*it)->CurrentCount) == 0) {
                    // add this node to the back of the list.
                    availableNodes.push_back(*it);
                }
            }
        }
    }
}
    
}

#endif