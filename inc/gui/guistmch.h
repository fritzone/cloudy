#ifndef _GUISTATEMACHNIE_H_
#define _GUISTATEMACHNIE_H_

#include <statemch.h>

class GuiState;

class GuiStatemachine : public Statemachine
{
public:

    GuiState* getCurrentState() const;

    static GuiStatemachine& instance();

};


#endif
