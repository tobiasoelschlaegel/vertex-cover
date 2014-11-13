#include <stdio.h>
#include "graph.h"
#include "queue.h"

//#define VC_SIMPLE_DEBUG
//#define VC_MAXDEG_DEBUG
//#define VC_DEGREE2_DEBUG

/*
    gcc -o vc -Wall -O2 main.c graph.c stack.c bitset.c union_find.c queue.c -std=c99
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

bool find_minmaxdeg_vertex(subgraph_t *subgraph, vertex_t *maxvert, int *maxdeg, vertex_t *minvert, int *mindeg)
{
    vertex_t vertex;
    subgraph_iter_t iter_vertices, iter_neighborhood;

    *maxdeg = 0;
    *mindeg = 0;

    subgraph_iter_all_vertices(subgraph, &iter_vertices);
    while(subgraph_iter_next(subgraph, &iter_vertices, &vertex))
    {
        int deg = 0;
        vertex_t neighbor;

        subgraph_iter_neighborhood(subgraph, &iter_neighborhood, vertex);
        while(subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor))
            deg++;

        subgraph_iter_destroy(&iter_neighborhood);
        if(deg > *maxdeg)
        {
            *maxdeg = deg;
            *maxvert = vertex;
        }
        
        if(deg == 0)
            subgraph_remove_vertex(subgraph, vertex);
        else if((*mindeg == 0) || (deg < *mindeg))
        {
            *mindeg = deg;
            *minvert = vertex;
        }
    }
    subgraph_iter_destroy(&iter_vertices);
    return (*maxdeg > 0);
}

bool vc_tree_cycle(subgraph_t *subgraph, int k)
{
    vertex_t vertex, minvertex;
    int maxdeg, mindeg;
    
    while((k >= 0) && find_minmaxdeg_vertex(subgraph, &vertex, &maxdeg, &minvertex, &mindeg))
    {
        if(maxdeg == 1)
            return (2 * k >= subgraph_num_vertices(subgraph));
        else if(mindeg == 1)
        {
            /* add neighbor of 'minvertex' to the vc */
            vertex_t neighbor;
            subgraph_iter_t iter_neighborhood;

            /* find any neighbor of 'minvertex' */
            subgraph_iter_neighborhood(subgraph, &iter_neighborhood, minvertex);
            subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor);
            subgraph_iter_destroy(&iter_neighborhood);
            
            k--;
            subgraph_remove_vertex(subgraph, minvertex);
            subgraph_remove_vertex(subgraph, neighbor);
        }
        else
        {
            int cycle_length = 1;
            bool cycle_removed = false;
            /* 'vertex' belongs to a cycle, so find out its length */
            
            while(!cycle_removed)
            {
                vertex_t neighbor;
                subgraph_iter_t iter_neighborhood;

                /* find any neighbor of 'vertex' */
                subgraph_iter_neighborhood(subgraph, &iter_neighborhood, vertex);
                if(subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor))
                {
                    subgraph_remove_vertex(subgraph, vertex);
                    cycle_length++;
                    vertex = neighbor;
                }
                else
                {
                    /* 'vertex' does not have a neighbor */
                    subgraph_remove_vertex(subgraph, vertex);
                    cycle_removed = true;
                }

                subgraph_iter_destroy(&iter_neighborhood);
            }
            
            if(cycle_length % 2)
                k = k - ((cycle_length + 1) / 2);
            else
                k = k - (cycle_length / 2);
        }
    }
    
    return (k >= 0);
}

void compute_discs(const subgraph_t const *subgraph, vertex_t root, int levels)
{
    int *distances = (int * ) malloc(sizeof(int) * subgraph_base_num_vertices(subgraph));
    queue_t bfs_queue;
    vertex_t vertex;
    subgraph_t discs;

    subgraph_init_induced(&discs, subgraph_get_base_graph(subgraph));

    for(int i = 0; i < subgraph_base_num_vertices(subgraph); i++)
        distances[i] = -1;

    distances[root] = 0;

    queue_init(&bfs_queue, sizeof(vertex_t));
    queue_enqueue(&bfs_queue, &root);

    while(queue_dequeue(&bfs_queue, &vertex))
    {
        subgraph_iter_t iter_neighborhood;
        vertex_t neighbor;

        subgraph_add_vertex(&discs, vertex);
        fprintf(stdout, "[debug] vertex %u is on layer %i\n", vertex, distances[vertex]);

        subgraph_iter_neighborhood(subgraph, &iter_neighborhood, vertex);
        while(subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor))
        {
            if(distances[neighbor] >= 0)
                continue;

            distances[neighbor] = distances[vertex] + 1;

            if(distances[neighbor] <= levels)
                queue_enqueue(&bfs_queue, &neighbor);
        }
        subgraph_iter_destroy(&iter_neighborhood);
    }

    subgraph_print(&discs);
    subgraph_destroy(&discs);
    queue_destroy(&bfs_queue);
    free(distances);
}

void vc_buss_kernel(subgraph_t *subgraph, int *k)
{
    vertex_t maxvertex, minvertex;
    int maxdeg, mindeg;

    while((*k > 0) && find_minmaxdeg_vertex(subgraph, &maxvertex, &maxdeg, &minvertex, &mindeg))
    {
        subgraph_iter_t iter_neighborhood;
        vertex_t neighbor;

        if(mindeg == 1)
        {
            subgraph_iter_neighborhood(subgraph, &iter_neighborhood, minvertex);
            subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor);
            subgraph_iter_destroy(&iter_neighborhood);
            subgraph_remove_vertex(subgraph, minvertex);
            subgraph_remove_vertex(subgraph, neighbor);
            (*k)--;
        }
        else if(maxdeg > *k)
        {
            subgraph_remove_vertex(subgraph, maxvertex);
            (*k)--;
        }
        else if(mindeg == 2)
        {
            bool found = false;
            vertex_t vertex;
            subgraph_iter_t iter_vertices;
            
            subgraph_iter_all_vertices(subgraph, &iter_vertices);
            while((*k > 0) && subgraph_iter_next(subgraph, &iter_vertices, &vertex))
            {
                if(subgraph_degree(subgraph, vertex) == 2)
                {
                    vertex_t neighbor1, neighbor2, neighbor = 0;
                    subgraph_iter_t iter_neighborhood;
#ifdef VC_DEGREE2_DEBUG
                    fprintf(stdout, "[debug] vertex %u has degree 2\n", vertex);
#endif
                    subgraph_iter_neighborhood(subgraph, &iter_neighborhood, vertex);
                    subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor1);
                    subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor2);
                    subgraph_iter_destroy(&iter_neighborhood);

                    /* check if neighbors are connected */
                    subgraph_iter_neighborhood(subgraph, &iter_neighborhood, neighbor1);
                    while(subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor))
                    {
                        if(neighbor == neighbor2)
                            break;
                    }
                    subgraph_iter_destroy(&iter_neighborhood);

                    if(neighbor == neighbor2)
                    {
#ifdef VC_DEGREE2_DEBUG
                        fprintf(stdout, "[debug] degree-2 vertex has two connected neighbors\n");
#endif
                        found = true;
                        subgraph_remove_vertex(subgraph, vertex);
                        subgraph_remove_vertex(subgraph, neighbor1);
                        subgraph_remove_vertex(subgraph, neighbor2);
                        (*k) = (*k) - 2;
                    }
                }
            }
            subgraph_iter_destroy(&iter_vertices);
            
            if(!found)
                break;
        }
        else
            break;
    }
}

bool vc_maxdeg_recursive(const subgraph_t const *subgraph, int k)
{
    vertex_t vertex, minvertex;
    int maxdeg, mindeg;
    subgraph_t graph;
    bool solution_found = false;
    
    subgraph_init_copy(&graph, subgraph);
    
    /* kernelization: remove vertices of degree 1 or degree > k */
    vc_buss_kernel(&graph, &k);

    if(find_minmaxdeg_vertex(&graph, &vertex, &maxdeg, &minvertex, &mindeg))
    {
#ifdef VC_MAXDEG_DEBUG
        fprintf(stdout, "[debug] found maximum degree vertex: %u has %u neighbors\n", vertex, maxdeg);
#endif
        if(k > 0)
        {
            /* if the graph consists of trees and cycles, we can solve it in polynomial time */
            if(maxdeg <= 2)
                solution_found = vc_tree_cycle(&graph, k);
            /*else if(maxdeg <= 8)
            {
                printf("maxvertex: %u, maxdeg: %u, %u vertices left\n", vertex, maxdeg, subgraph_num_vertices(&graph));
                compute_discs(&graph, vertex, 4);
                exit(0);
            }*/
            else
            {
                subgraph_t subcopy;
                
                /* create the first branch: include 'vertex' */
                subgraph_init_copy(&subcopy, &graph);
                subgraph_remove_vertex(&subcopy, vertex);
                solution_found = vc_maxdeg_recursive(&subcopy, k - 1);
                subgraph_destroy(&subcopy);

                /* create the second branch: include the neighborhood of 'vertex' */
                if(!solution_found && (maxdeg <= k))
                {
                    subgraph_iter_t iter_neighborhood;
                    vertex_t neighbor;
                    
                    subgraph_init_copy(&subcopy, &graph);
                    subgraph_iter_neighborhood(&subcopy, &iter_neighborhood, vertex);
                    while(subgraph_iter_next(&subcopy, &iter_neighborhood, &neighbor))
                    {
                        subgraph_remove_vertex(&subcopy, neighbor);
                        k--;
                    }

                    subgraph_iter_destroy(&iter_neighborhood);
                    subgraph_remove_vertex(&subcopy, vertex);
                    
                    solution_found = vc_maxdeg_recursive(&subcopy, k);
                    subgraph_destroy(&subcopy);
                }
            }
        }
    }
    else
        solution_found = (k >= 0);

    subgraph_destroy(&graph);
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
        if(vc_maxdeg_recursive(&subgraph, k))
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

