#include <stdio.h>
#include "graph.h"

/*#define VC_SIMPLE_DEBUG*/
/*
    gcc -o vc -Wall -O2 main.c graph.c stack.c bitset.c union_find.c -std=c99
 */

struct _vc_simple_state_s
{
    vertex_t u, v; /* endpoints of an uncovered edge */
    int k;
};
typedef struct _vc_simple_state_s vc_simple_state_t;

bool find_uncovered_edge(const subgraph_t const *subgraph, const subgraph_t const *vc_partial, vertex_t *u, vertex_t *v)
{
    subgraph_iter_t iter_vertices, iter_neighborhood;
    bool edge_found = false;

    subgraph_iter_all_vertices(subgraph, &iter_vertices);
    while(!edge_found && subgraph_iter_next(subgraph, &iter_vertices, u))
    {
        /* skip this vertex if it is already part of the vertex cover */
        if(subgraph_contains_vertex(vc_partial, *u))
            continue;

        /* check if any neighbor is also not part of the vertex cover */
        subgraph_iter_neighborhood(subgraph, &iter_neighborhood, *u);
        while(!edge_found && subgraph_iter_next(subgraph, &iter_neighborhood, v))
        {
            if(subgraph_contains_vertex(vc_partial, *v))
                continue;
            /* the edge (vertex, neighbor) is uncovered */
            edge_found = true;
        }
        subgraph_iter_destroy(&iter_neighborhood);
    }
    subgraph_iter_destroy(&iter_vertices);
    
    return edge_found;
}

bool vc_maxdeg(const subgraph_t const *subgraph, int k)
{
    bool solution_found = false;

    return solution_found;
}

bool vc_simple(const subgraph_t const *subgraph, int k)
{
    stack_t tree_stack;
    subgraph_t vc_partial;
    bool solution_found = false;
    vc_simple_state_t state;

    subgraph_init_induced(&vc_partial, subgraph_get_base_graph(subgraph));
    stack_init(&tree_stack, sizeof(vc_simple_state_t));

    state.k = k;

    do
    {
        /* first: find any uncovered edge in the graph. we have a solution if no edge exists */
        if(!find_uncovered_edge(subgraph, &vc_partial, &state.u, &state.v))
        {
#ifdef VC_SIMPLE_DEBUG
            fprintf(stdout, "[debug] found solution: ");
            subgraph_print(&vc_partial);
#endif
            solution_found = true;
        }
        else
        {
#ifdef VC_SIMPLE_DEBUG
            fprintf(stdout, "[debug] found uncovered edge (%u, %u)\n", state.u, state.v);
#endif
            if(state.k > 0)
            {
                state.k--;
                stack_push(&tree_stack, &state);
#ifdef VC_SIMPLE_DEBUG
                fprintf(stdout, "[debug] adding %u to the vertex cover\n", state.u);
#endif
                subgraph_add_vertex(&vc_partial, state.u);
            }
            else
            {
                bool next_state_found = false;
                /* it's not possible to create more branches */
                while(!next_state_found && stack_pop(&tree_stack, &state))
                {
                    if(subgraph_contains_vertex(&vc_partial, state.u))
                    {
#ifdef VC_SIMPLE_DEBUG
                        fprintf(stdout, "[debug] first branch did not succeed\n");
                        fprintf(stdout, "[debug] adding %u to the vertex cover\n", state.v);
#endif
                        subgraph_remove_vertex(&vc_partial, state.u);
                        subgraph_add_vertex(&vc_partial, state.v);
                        stack_push(&tree_stack, &state);
                        next_state_found = true;
                    }
                    else
                    {
#ifdef VC_SIMPLE_DEBUG
                        fprintf(stdout, "[debug] second branch did not succeed\n");
#endif
                        subgraph_remove_vertex(&vc_partial, state.v);
                    }
                }
            }
        }
    }
    while(!solution_found && (stack_height(&tree_stack) > 0));

    stack_destroy(&tree_stack);
    subgraph_destroy(&vc_partial);

    return solution_found;
}

int main(int argc, char **argv)
{
    graph_t graph;
    subgraph_t subgraph;
    int k;

    if(argc < 4)
    {
        fprintf(stdout, "Usage: %s <graph.dgf> <size-of-vc> <algorithm>\n", argv[0]);
        fprintf(stdout, " Available algorithms:\n");
        fprintf(stdout, "  simple     chooses edges and branches on their endpoints\n");
        fprintf(stdout, "  maxdeg     chooses vertex of maximum degree\n");
        fprintf(stdout, "  maxdegred  same as 'maxdeg' but also uses reduction rules\n");
        return 0;
    }

    k = atoi(argv[2]);
    if(k < 0)
        k = 0;

    if(!graph_load_dimacs(&graph, argv[1], false))
        return 0;

    fprintf(stdout, "[info] input graph has %u vertices and %u edges\n", graph_num_vertices(&graph), graph_num_edges(&graph));

    subgraph_init_induced(&subgraph, &graph);
    for(vertex_t v = 0; v < graph_num_vertices(&graph); v++)
        subgraph_add_vertex(&subgraph, v);

    if(!strcmp(argv[3], "simple"))
    {
        if(vc_simple(&subgraph, k))
            fprintf(stdout, "vc-simple: YES\n");
        else
            fprintf(stdout, "vc-simple: NO\n");
    }
    else if(!strcmp(argv[3], "maxdeg"))
    {
        if(vc_maxdeg(&subgraph, k))
            fprintf(stdout, "vc-maxdeg: YES\n");
        else
            fprintf(stdout, "vc-maxdeg: NO\n");
    }
    else
        fprintf(stdout, "[error] unknown algorithm was selected\n");

    subgraph_destroy(&subgraph);
    graph_destroy(&graph);

    return 0;
}
