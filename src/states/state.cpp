#include "state.h"

void State::setNextState(int state, State *nexts)
{
    m_nexts[state] = nexts;
}

void State::setrPreviousState(int state, State *prevs)
{
    m_prevs[state] = prevs;
}


std::map<int, State *>& State::nexts()
{
    return m_nexts;
}

std::map<int, State *>& State::prevs()
{
    return m_prevs;
}

State::~State()
{

}

State::State()
{

}
