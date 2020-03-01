#pragma once

#include "schoco/details/list/list.hpp"
#include "schoco/details/pool/pool.hpp"
#include "schoco/details/graph/node.hpp"
#include "schoco/details/graph/edge.hpp"

namespace schoco_details
{

template <class>
class TGraph;

namespace graph
{

template <class Traits>
class TPoolSet
{
    template <class>
    friend class schoco_details::TGraph;

    typename Traits::Nodes   _nodes;
    typename Traits::Edges   _edges;
    typename Traits::EdgeIts _edgeIts;
};

namespace pool_set
{

template <class Type, std::size_t SIZE>
struct TTraits
{
    using Node = node::TMake<Type, SIZE>;
    using Edge = edge::TMake<Type, SIZE>;

    using Nodes   = typename schoco_details::list::TMake<Node   , SIZE>::CellPool;
    using Edges   = typename schoco_details::list::TMake<Edge   , SIZE>::CellPool;
    using EdgeIt  = typename schoco_details::list::TMake<Edge   , SIZE>::iterator;
    using EdgeIts = typename schoco_details::list::TMake<EdgeIt , SIZE>::CellPool;
};

template <class Type, std::size_t SIZE>
using TMake = TPoolSet<TTraits<Type, SIZE>>;

} // namespace pool_set

} // namespace graph

} // namespace schoco_details
