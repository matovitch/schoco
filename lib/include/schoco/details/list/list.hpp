#pragma once

#include "schoco/details/list/iterator.hpp"
#include "schoco/details/list/steal.hpp"
#include "schoco/details/list/cell.hpp"

#include "schoco/details/pool/pool.hpp"

#include <cstdint>

namespace schoco_details
{

template <class Traits>
class TList
{
    using Cell = typename Traits::Cell;

public:

    using CellPool = typename Traits::CellPool;
    using Type     = typename Traits::Type;

    using       iterator = list::iterator       ::TMake<Type>;
    using const_iterator = list::const_iterator ::TMake<Type>;

    friend void list::steal(TList<Traits>&,
                            TList<Traits>&,
                            list::TIterator<Cell>&);

    TList(CellPool& cellPool) :
        _cellPool{cellPool}
    {}

    template <class... Args>
    void emplace_front(Args&&... args)
    {
        _head = &(_cellPool.make(_head, args...));
    }

    void pop_front()
    {
        _cellPool.recycle(*_head);
        _head = _head->_next;
    }

    bool empty() const { return _head == nullptr; }

                iterator  end()       { return       iterator{}; }
    const       iterator  end() const { return       iterator{}; }
          const_iterator cend()       { return const_iterator{}; }
    const const_iterator cend() const { return const_iterator{}; }

                iterator  begin()       { return       iterator{_head}; }
    const       iterator  begin() const { return       iterator{_head}; }
          const_iterator cbegin()       { return const_iterator{_head}; }
    const const_iterator cbegin() const { return const_iterator{_head}; }

    iterator erase(iterator& it)
    {
        Cell* const cellPtr = it._cellPtr;

        if (cellPtr == _head)
        {
            _head = _head->_next;
            _cellPool.recycle(*cellPtr);
            return iterator{cellPtr->_next};
        }

        Cell* const prev = cellPtr->_prev;
        Cell* const next = cellPtr->_next;

        prev->_next = next;

        if (next)
        {
            next->_prev = prev;
        }

        _cellPool.recycle(*cellPtr);
        return iterator{next};
    }

private:

    Cell* _head = nullptr;

    CellPool& _cellPool;
};

namespace list
{

template <class TraitsType, std::size_t SIZE>
struct TTraits
{
    using Type     = TraitsType;
    using Cell     = list::TCell<Type>;
    using CellPool = pool::TMake<Cell, SIZE>;
};

template <class Type, std::size_t SIZE>
using TMake = TList<TTraits<Type, SIZE>>;

} // namespace list

} // namespace schoco_details
