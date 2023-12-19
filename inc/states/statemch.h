#ifndef _STATEMACHINE_H_
#define _STATEMACHINE_H_

#include <string>
#include <vector>

class State;

class Statemachine
{
public:

    void reportError(const char* s);

    const char* getError();

    void init(State* s);

    int advance(void* data, int state);

    int previous(void* data, int state);


    void addState(State*);

    void logStates();

protected:

    State *getCurrentState() const;

    Statemachine();

private:

    State* m_currentState;
    std::vector<State*> m_states;
    std::string m_error;
};

#endif
