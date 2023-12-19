#ifndef _STATE_H_
#define _STATE_H_

#include <map>

class State
{
public:

    State();
    virtual ~State();

    void setNextState(int state, State* nexts);
    void setrPreviousState(int state, State* prevs);

    virtual const char* name() const = 0;

    std::map<int, State *>& nexts();
    std::map<int, State *>& prevs();

private:
    std::map<int, State*> m_nexts;
    std::map<int, State*> m_prevs;

};

#endif
