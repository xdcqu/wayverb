#include "recursive_tetrahedral_program.h"

using namespace std;

TetrahedralProgram::TetrahedralProgram(const cl::Context & context,
                                       bool build_immediate)
        : Program(context, source, build_immediate) {
}

const string TetrahedralProgram::source{
#ifdef DIAGNOSTIC
    "#define DIAGNOSTIC\n"
#endif
    R"(

    #define PORTS (4)

    typedef struct {
        int ports[PORTS];
        float3 position;
    } Node;

    kernel void waveguide
    (   unsigned long write
    ,   float value
    ,   float attenuation
    ,   global float * next
    ,   global float * current
    ,   global float * previous
    ,   global Node * nodes
    ,   unsigned long read
    ,   global float * output
    ) {
        size_t index = get_global_id(0);
        global Node * node = nodes + index;

        if (index == write) {
            current[index] += value;
        }

        barrier(CLK_GLOBAL_MEM_FENCE);

        //  TODO I have NO IDEA whether this difference equation is correct

        float temp = 0;
        for (int i = 0; i != PORTS; ++i) {
            int port_index = node->ports[i];
            if (port_index >= 0) {
                temp += current[port_index];
            }
        }

        temp /= 2;
        temp -= previous[index];

        //  TODO probably not right way to damp?
        temp *= attenuation;

        next[index] = temp;

        if (index == read) {
            *output = next[index];
        }
    }
    )"};