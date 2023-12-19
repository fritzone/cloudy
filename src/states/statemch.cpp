#include "statemch.h"
#include "state.h"
#include "log.h"

void Statemachine::reportError(const char *s)
{
    m_error = s;
}

const char *Statemachine::getError()
{
    return m_error.c_str();
}

void Statemachine::init(State *s)
{
    m_currentState = s;
    logStates();
}

int Statemachine::advance(void *data, int state)
{
    if(!m_currentState)
    {
        log_error() << "No current state for gui statemachine";
        return 0;
    }

    log_info() << "Trying to advance from" << m_currentState->name();

    if(m_currentState->nexts().count(state))
    {
        const std::map<int, State*> & nexts = m_currentState->nexts();
        m_currentState = ((std::map<int, State *>&)nexts)[state];
    }
    else
    {
        log_error()  << "No next for" << m_currentState->name();
        return 0;
    }
    return 1;
}

int Statemachine::previous(void *data, int state)
{
    if(!m_currentState)
    {
        log_error() << "No current state for gui statemachine";
        return 1;
    }

    if(m_currentState->prevs().count(state) && m_currentState->prevs()[state])
    {
        m_currentState = m_currentState->prevs()[state];
    }
    return 0;
}

State *Statemachine::getCurrentState() const
{
    log_info() << "Returns:" <<(void*)m_currentState;
    return m_currentState;
}

void Statemachine::addState(State *s)
{
    m_states.push_back(s);
}

void Statemachine::logStates()
{
    log_info() << "----- StateCount:" << m_states.size();
    for(int i=0; i<m_states.size(); i++)
    {
        log_info() << "State:" << m_states[i]->name() << " at " << (void*)m_states[i]
                   <<  (m_states[i] == m_currentState ? " ***" : "");
    }
    log_info() << "----- StateCount:" << m_states.size();

}

Statemachine::Statemachine()
{

}
