#ifndef GRAPH_H_INCLUDED
#define GRAPH_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include "bitset.h"
#include "stack.h"

/** @brief Maximum length of input buffer that is supposed to store one line */
#define GRAPH_MAX_INPUT_BUFFER_SIZE 1024

typedef uint32_t vertex_label_t;
typedef uint32_t vertex_t;

typedef enum GRAPH_ITERATOR_TYPES
{
    GRAPH_ITER_ALL_VERTICES,
    GRAPH_ITER_NEIGHBORHOOD
} graph_iter_type_t;

struct _graph_s
{
    uint32_t num_vertices;
    uint32_t num_edges;
    vertex_label_t *labels;
    uint32_t *positions;
    vertex_t *edges;
};
typedef struct _graph_s graph_t;

struct _subgraph_s
{
    const graph_t *base;
    bitset_t vertices;
    uint32_t num_vertices;
};
typedef struct _subgraph_s subgraph_t;

struct _subgraph_iter_s
{
    graph_iter_type_t type;
    bitset_t vertices;
    bitset_index_t last_vertex, next_vertex;
    uint32_t edge_start, edge_end;
};
typedef struct _subgraph_iter_s subgraph_iter_t;

/* ------------ start of builder -------------- */
struct _graph_edge_s
{
    /** @brief Endpoint of edge. */
    vertex_label_t from;
    /** @brief Endpoint of edge. */
    vertex_label_t to;
};
typedef struct _graph_edge_s edge_t;

struct _graph_builder_s
{
    /** @brief List of vertex labels */
    stack_t vertex_labels;
    /** @brief List of edges that were added. */
    stack_t edges;
    /** @brief Greatest vertex label of vertices that were added to the builder. */
    vertex_label_t max_vertex_label;
};
typedef struct _graph_builder_s gbuild_t;

void gbuild_init(gbuild_t *builder);
void gbuild_destroy(gbuild_t *builder);
void gbuild_add_vertex(gbuild_t *builder, vertex_label_t vertex_label);
void gbuild_add_edge(gbuild_t *builder, vertex_label_t from, vertex_label_t to);
uint32_t gbuild_num_vertices(gbuild_t *builder);
uint32_t gbuild_num_edges(gbuild_t *builder);
bool gbuild_is_trivial(gbuild_t *builder);
bool gbuild_get_vertex_by_label(gbuild_t *builder, const vertex_label_t vertex_label, vertex_t *result);
void gbuild_create_graph(gbuild_t *builder, graph_t *graph);

bool graph_get_vertex_by_label(const graph_t const *graph, const vertex_label_t vertex_label, vertex_t *result);
bool graph_load_dimacs(graph_t *graph, const char *filename, const bool show_comments);
/* ------------ end of builder -------------- */

bool graph_save_binary(const graph_t const *graph, const char *filename);
uint32_t graph_num_vertices(const graph_t const *graph);
uint32_t graph_num_edges(const graph_t const *graph);
uint32_t graph_degree(const graph_t const *graph, vertex_t vertex);
uint32_t graph_get_label(const graph_t const *graph, const vertex_t vertex);
vertex_t graph_get_edge(const graph_t const *graph, uint32_t index);
void graph_destroy(graph_t *graph);

void subgraph_init_induced(subgraph_t *subgraph, const graph_t const *base_graph);
void subgraph_init_copy(subgraph_t *subgraph, const subgraph_t const *source);
void subgraph_copy(subgraph_t *subgraph, const subgraph_t const *source);
void subgraph_add_vertex(subgraph_t *subgraph, vertex_t vertex);
void subgraph_remove_vertex(subgraph_t *subgraph, vertex_t vertex);
void subgraph_destroy(subgraph_t *subgraph);
bool subgraph_is_connected(const subgraph_t const *subgraph);
bool subgraph_contains_vertex(const subgraph_t const *subgraph, vertex_t vertex);
void subgraph_print(const subgraph_t const *subgraph);
uint32_t subgraph_num_vertices(const subgraph_t const *subgraph);
uint32_t subgraph_base_num_vertices(const subgraph_t const *subgraph);
const graph_t *subgraph_get_base_graph(const subgraph_t const *subgraph);
void subgraph_find_components(const subgraph_t const *subgraph, stack_t *components);

void subgraph_iter_init_vertices(const subgraph_t const *subgraph, subgraph_iter_t *iterator);
void subgraph_iter_all_vertices(const subgraph_t const *subgraph, subgraph_iter_t *iterator);
void subgraph_iter_neighborhood(const subgraph_t const *subgraph, subgraph_iter_t *iterator, vertex_t vertex);
bool subgraph_iter_next(const subgraph_t const *subgraph, subgraph_iter_t *iterator, vertex_t *vertex);
void subgraph_iter_add_vertex(subgraph_iter_t *iterator, vertex_t vertex);
void subgraph_iter_remove_vertex(subgraph_iter_t *iterator, vertex_t vertex);
bool subgraph_iter_contains_vertex(subgraph_iter_t *iterator, vertex_t vertex);
void subgraph_iter_destroy(subgraph_iter_t *iterator);

#endif

