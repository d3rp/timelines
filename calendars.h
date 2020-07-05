#pragma once

#include <array>

#include "globals.h"
#include "utilities.h"
#include "entities.h"

int yearToIndex(int year);
int indexToYear(int index);

struct TimeInterface
{

};
struct JulianCalendar : public TimeInterface
{};
class Years
{
    Years();

    static Years* instance;

public:
    ~Years();
    static Years* getInstance();

    typedef std::unique_ptr<Entity> EntityPtr;

//    std::array<std::vector<Entity*>, MAX_BINS> bins{};
    std::array<size_t, MAX_BINS> year_bins{ 0 };

    void insert(Entity* e);
    void clear();
    struct IdEquals
    {
        const int id;
        IdEquals(Entity* e)
            : id(e->id)
        {
        }
        bool operator()(Entity* x) const { return x->id == id; }
    };

    std::set<Entity*> getEntitiesInInterval(int start, int end);
};
