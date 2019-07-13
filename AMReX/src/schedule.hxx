#ifndef SCHEDULE_HXX
#define SCHEDULE_HXX

#include <cctk.h>
#include <cctk_Schedule.h>

namespace AMReX {

int Initialise(tFleshConfig *config);
int Evolve(tFleshConfig *config);
int Shutdown(tFleshConfig *config);

int SyncGroupsByDirI(const cGH *restrict cctkGH, int numgroups,
                     const int *groups, const int *directions);

int CallFunction(void *function, cFunctionData *attribute, void *data);
} // namespace AMReX

#endif // #ifndef SCHEDULE_HXX
