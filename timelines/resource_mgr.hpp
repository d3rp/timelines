#pragma once
#include "calendars.hpp"
#include "entities.hpp"

namespace tl
{

///////////////////////////
// Global resource manager
class Global
{
    Global() { years = std::make_unique<Years>(); }

  public:
    ~Global() { std::cout << "DTOR " << __PRETTY_FUNCTION__ << "\n"; }

    static Global& instance();

    std::vector<std::unique_ptr<Entity>> data;
    std::unique_ptr<Years> years;
};

static Years&
years()
{
    return *Global::instance().years;
}
}
