#pragma once

#include "schoco/details/list/iterator.hpp"
#include "schoco/details/list/list.hpp"

namespace schoco_details
{

template <class>
class TGraph;

namespace graph
{

template <class>
class TNode;

namespace node
{

template <class, std::size_t>
struct TTraits;

template <class Type, std::size_t SIZE>
using TMake = TNode<TTraits<Type, SIZE>>;

} // namespace node

template <class>
class TEdge;

namespace edge
{

template <class, std::size_t>
struct TTraits;

template <class Type, std::size_t SIZE>
using TMake = TEdge<TTraits<Type, SIZE>>;

template <class Type, std::size_t SIZE>
struct TTraits
{
    using NodeListIt       = schoco_details::list::iterator::TMake<node::TMake<Type, SIZE> >;
    using EdgeListIt       = schoco_details::list::iterator::TMake<edge::TMake<Type, SIZE> >;
    using EdgeListItListIt = schoco_details::list::iterator::TMake<EdgeListIt              >;

    using EdgeList         = schoco_details::list::TMake<edge::TMake<Type, SIZE>, SIZE>;
};

} // namespace edge

template <class Traits>
class TEdge
{
    template <class>
    friend class schoco_details::TGraph;

    using EdgeList         = typename Traits::EdgeList;
    using NodeListIt       = typename Traits::NodeListIt;
    using EdgeListItListIt = typename Traits::EdgeListItListIt;

public:

    TEdge(const NodeListIt lhs,
          const NodeListIt rhs) :
        _dependerNode{lhs},
        _dependeeNode{rhs}
    {}

    void update()
    {
        _dependerEdge = _dependerNode->_dependees.begin();
        _dependeeEdge = _dependeeNode->_dependers.begin();
    }

    void detach()
    {
        _dependerNode->_dependees.erase(_dependerEdge);
        _dependeeNode->_dependers.erase(_dependeeEdge);
    }

private:

    NodeListIt _dependerNode;
    NodeListIt _dependeeNode;

    EdgeListItListIt _dependerEdge;
    EdgeListItListIt _dependeeEdge;
};

} // namespace graph

} // namespace schoco_details
