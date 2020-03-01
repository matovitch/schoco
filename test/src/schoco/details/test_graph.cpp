#include "schoco/details/graph/pool_set.hpp"
#include "schoco/details/graph/graph.hpp"

#include "doctest/doctest_fwd.h"

#include <iostream>

using CharGraph   = schoco_details::graph::TMake<char, 4>;
using CharPoolSet = typename CharGraph::PoolSet;


TEST_CASE("schoco_details::graph")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');

    charGraph.attach(aAsNode, bAsNode);

    auto&& edge = charGraph.attach(bAsNode, cAsNode);

    charGraph.detach(edge);

    const char* ptr = "BAC";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}

TEST_CASE("schoco_details::graph")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');
    auto&& dAsNode = charGraph.makeNode('D');

    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(cAsNode, aAsNode);
    charGraph.attach(aAsNode, dAsNode);

    CHECK(charGraph.top()->_value == 'D');
    charGraph.pop(charGraph.top());

    CHECK(charGraph.empty());

    CHECK(charGraph.isCyclic());

    const auto& cycle = charGraph.makeCycle();

    CHECK(cycle);

    const char* ptr = "CBA";

    for (const auto& nodeIt : *(cycle))
    {
        CHECK(nodeIt->_value == *ptr++);
    }
}


TEST_CASE("schoco_details::graph BA")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');

    charGraph.attach(aAsNode, bAsNode);

    const char* ptr = "BA";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}

TEST_CASE("topo::graph")
{
    CharPoolSet charPoolSet;

    CharGraph charGraph{charPoolSet};

    auto&& aAsNode = charGraph.makeNode('A');
    auto&& bAsNode = charGraph.makeNode('B');
    auto&& cAsNode = charGraph.makeNode('C');
    auto&& dAsNode = charGraph.makeNode('D');

    charGraph.attach(aAsNode, bAsNode);
    charGraph.attach(bAsNode, cAsNode);
    charGraph.attach(bAsNode, dAsNode);

    charGraph.pop(cAsNode);

    const char* ptr = "DBA";

    while (!charGraph.empty())
    {
        CHECK(charGraph.top()->_value == *ptr++);
        charGraph.pop(charGraph.top());
    }

    CHECK(!charGraph.isCyclic());
}
