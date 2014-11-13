#include <errno.h>
#include "graph.h"
#include "union_find.h"

bool graph_save_binary(const graph_t const *graph, const char *filename)
{
    FILE *fd;
    unsigned char buffer[4];
    uint32_t value;

    if((fd = fopen(filename, "wb")) == NULL)
        return false;

    /* write 0xBFBFBFBF to indicate binary format */
    buffer[0] = 0xBF;
    buffer[1] = 0xBF;
    buffer[2] = 0xBF;
    buffer[3] = 0xBF;
    fwrite(buffer, 4, 1, fd);
    /* write number of vertices */
    value = graph_num_vertices(graph);
    buffer[0] = (value >> 24) % 256;
    buffer[1] = (value >> 16) % 256;
    buffer[2] = (value >> 8) % 256;
    buffer[3] = (value >> 0) % 256;
    /* write number of edges */
    fwrite(buffer, 4, 1, fd);
    value = graph_num_edges(graph);
    buffer[0] = (value >> 24) % 256;
    buffer[1] = (value >> 16) % 256;
    buffer[2] = (value >> 8) % 256;
    buffer[3] = (value >> 0) % 256;
    fwrite(buffer, 4, 1, fd);

    /* for each vertex: write label and degree */
    for(uint32_t i = 0; i < graph_num_vertices(graph); i++)
    {
        value = graph_get_label(graph, i);
        buffer[0] = (value >> 24) % 256;
        buffer[1] = (value >> 16) % 256;
        buffer[2] = (value >> 8) % 256;
        buffer[3] = (value >> 0) % 256;
        fwrite(buffer, 4, 1, fd);
        value = graph->positions[i];
        buffer[0] = (value >> 24) % 256;
        buffer[1] = (value >> 16) % 256;
        buffer[2] = (value >> 8) % 256;
        buffer[3] = (value >> 0) % 256;
        fwrite(buffer, 4, 1, fd);
        for(int i = 0; i < 4; i++)
            printf("%u\n", buffer[i]);
    }

    if(graph_num_vertices(graph) < 65536)
    {
        for(uint32_t i = 0; i < 2 * graph_num_edges(graph); i++)
        {
            value = graph->edges[i];
            buffer[0] = (value >> 8) % 256;
            buffer[1] = (value >> 0) % 256;
            fwrite(buffer, 2, 1, fd);
        }
    }
    else
    {
        for(uint32_t i = 0; i < 2 * graph_num_edges(graph); i++)
        {
            value = graph->edges[i];
            buffer[0] = (value >> 24) % 256;
            buffer[1] = (value >> 16) % 256;
            buffer[2] = (value >> 8) % 256;
            buffer[3] = (value >> 0) % 256;
            fwrite(buffer, 4, 1, fd);
        }
    }
    
    fclose(fd);
    return true;
}

uint32_t graph_num_vertices(const graph_t const *graph)
{
    return graph->num_vertices;
}

uint32_t graph_num_edges(const graph_t const *graph)
{
    return graph->num_edges;
}

uint32_t graph_degree(const graph_t const *graph, vertex_t vertex)
{
    if(graph_num_vertices(graph) <= vertex)
        return 0;
    
    if((vertex + 1) == graph_num_vertices(graph))
        return (2 * graph_num_edges(graph) - graph->positions[vertex]);
    else
        return (graph->positions[vertex + 1] - graph->positions[vertex]);
}

vertex_t graph_get_edge(const graph_t const *graph, uint32_t index)
{
    return graph->edges[index];
}

uint32_t graph_get_label(const graph_t const *graph, const vertex_t vertex)
{
    if(graph_num_vertices(graph) <= vertex)
        return 0;
    else
        return graph->labels[vertex];
}

void graph_destroy(graph_t *graph)
{
    if(graph->labels)
    {
        free(graph->labels);
        graph->labels = NULL;
    }
    
    if(graph->positions)
    {
        free(graph->positions);
        graph->positions = NULL;
    }
    
    if(graph->edges)
    {
        free(graph->edges);
        graph->edges = NULL;
    }
    
    graph->num_vertices = 0;
    graph->num_edges = 0;
}

void subgraph_init_induced(subgraph_t *subgraph, const graph_t const *base_graph)
{
    subgraph->base = base_graph;
    subgraph->num_vertices = 0;
    bitset_init(&(subgraph->vertices), (bitset_index_t) graph_num_vertices(base_graph));
}

void subgraph_init_copy(subgraph_t *subgraph, const subgraph_t const *source)
{
    subgraph->base = source->base;
    subgraph->num_vertices = source->num_vertices;
    bitset_init_copy(&(subgraph->vertices), &(source->vertices));
}

void subgraph_copy(subgraph_t *subgraph, const subgraph_t const *source)
{
    subgraph->base = source->base;
    subgraph->num_vertices = source->num_vertices;
    bitset_copy(&(subgraph->vertices), &(source->vertices));
}

void subgraph_add_vertex(subgraph_t *subgraph, vertex_t vertex)
{
    if(!bitset_get(&(subgraph->vertices), (bitset_index_t) vertex))
        subgraph->num_vertices++;

    bitset_set(&(subgraph->vertices), (bitset_index_t) vertex);
}

void subgraph_remove_vertex(subgraph_t *subgraph, vertex_t vertex)
{
    if(bitset_get(&(subgraph->vertices), (bitset_index_t) vertex))
        subgraph->num_vertices--;

    bitset_clear(&(subgraph->vertices), (bitset_index_t) vertex);
}

void subgraph_destroy(subgraph_t *subgraph)
{
    subgraph->base = NULL;
    bitset_destroy(&(subgraph->vertices));
}

bool subgraph_is_connected(const subgraph_t const *subgraph)
{
    uint32_t num_components;
    uf_t components;
    subgraph_iter_t iter_vert, iter_neigh;
    vertex_t vertex, neighbor;
    
    num_components = subgraph_num_vertices(subgraph);
    uf_init(&components, subgraph_base_num_vertices(subgraph));

    subgraph_iter_all_vertices(subgraph, &iter_vert);
    while(subgraph_iter_next(subgraph, &iter_vert, &vertex))
    {
        subgraph_iter_neighborhood(subgraph, &iter_neigh, vertex);
        while(subgraph_iter_next(subgraph, &iter_neigh, &neighbor))
        {
            if(uf_find(&components, vertex) != uf_find(&components, neighbor))
            {
                uf_union(&components, vertex, neighbor);
                num_components--;
            }
        }
    }
    
    subgraph_iter_destroy(&iter_vert);
    
    uf_destroy(&components);

    fprintf(stdout, "[debug] graph has %u component(s)\n", num_components);
    return (num_components == 1);
}

bool subgraph_contains_vertex(const subgraph_t const *subgraph, vertex_t vertex)
{
    return bitset_get(&(subgraph->vertices), (bitset_index_t) vertex);
}

void subgraph_print(const subgraph_t const *subgraph)
{
    subgraph_iter_t iterator;
    vertex_t vertex;
    
    fprintf(stdout, "--- subgraph ---\n");
    subgraph_iter_all_vertices(subgraph, &iterator);
    while(subgraph_iter_next(subgraph, &iterator, &vertex))
    {
        subgraph_iter_t iter_neighborhood;
        vertex_t neighbor;
        bool first = true;
        
        fprintf(stdout, "[vertex] %u: [", vertex);
        subgraph_iter_neighborhood(subgraph, &iter_neighborhood, vertex);
        while(subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor))
        {
            if(first)
            {
                first = false;
                fprintf(stdout, "%u", neighbor);
            }
            else
                fprintf(stdout, ", %u", neighbor);
        }
        subgraph_iter_destroy(&iter_neighborhood);
        fprintf(stdout, "]\n");
    }
    fprintf(stdout, "---   end   ---\n");
    subgraph_iter_destroy(&iterator);
}

uint32_t subgraph_num_vertices(const subgraph_t const *subgraph)
{
    return subgraph->num_vertices;
}

uint32_t subgraph_base_num_vertices(const subgraph_t const *subgraph)
{
    return graph_num_vertices(subgraph->base);
}

const graph_t *subgraph_get_base_graph(const subgraph_t const *subgraph)
{
    return subgraph->base;
}

void subgraph_find_components(const subgraph_t const *subgraph, stack_t *components)
{
    subgraph_t subgraph_current;
    subgraph_iter_t iter_vertices, iter_neigh;
    vertex_t vertex, neighbor;
    stack_t stack_dfs;
    
    stack_init(&stack_dfs, sizeof(vertex_t));
    
    subgraph_iter_all_vertices(subgraph, &iter_vertices);
    while(subgraph_iter_next(subgraph, &iter_vertices, &vertex))
    {
        subgraph_init_induced(&subgraph_current, subgraph_get_base_graph(subgraph));
        stack_push(&stack_dfs, &vertex);
        
        while(stack_pop(&stack_dfs, &vertex))
        {
            subgraph_add_vertex(&subgraph_current, vertex);
            
            subgraph_iter_neighborhood(subgraph, &iter_neigh, vertex);
            while(subgraph_iter_next(subgraph, &iter_neigh, &neighbor))
            {
                if(subgraph_iter_contains_vertex(&iter_vertices, neighbor))
                {
                    subgraph_iter_remove_vertex(&iter_vertices, neighbor);
                    stack_push(&stack_dfs, &neighbor);
                }
            }
            subgraph_iter_destroy(&iter_neigh);
        }
        
        stack_push(components, &subgraph_current);
    }
    
    subgraph_iter_destroy(&iter_vertices);
    stack_destroy(&stack_dfs);
}

uint32_t subgraph_degree(const subgraph_t const *subgraph, vertex_t vertex)
{
    subgraph_iter_t iter_neighborhood;
    uint32_t degree = 0;
    vertex_t neighbor;

    subgraph_iter_neighborhood(subgraph, &iter_neighborhood, vertex);
    while(subgraph_iter_next(subgraph, &iter_neighborhood, &neighbor))
        degree++;

    subgraph_iter_destroy(&iter_neighborhood);
    return degree;
}

void subgraph_iter_init_vertices(const subgraph_t const *subgraph, subgraph_iter_t *iterator)
{
    iterator->type = GRAPH_ITER_ALL_VERTICES;
    iterator->last_vertex = 0;
    iterator->next_vertex = 0;
    bitset_init(&(iterator->vertices), (bitset_index_t) graph_num_vertices(subgraph->base));
}

void subgraph_iter_all_vertices(const subgraph_t const *subgraph, subgraph_iter_t *iterator)
{
    iterator->type = GRAPH_ITER_ALL_VERTICES;
    iterator->last_vertex = 0;
    iterator->next_vertex = 0;
    bitset_init_copy(&(iterator->vertices), &(subgraph->vertices));
}

void subgraph_iter_neighborhood(const subgraph_t const *subgraph, subgraph_iter_t *iterator, vertex_t vertex)
{
    iterator->type = GRAPH_ITER_NEIGHBORHOOD;
    iterator->edge_start = subgraph->base->positions[vertex];
    iterator->edge_end = iterator->edge_start + graph_degree(subgraph->base, vertex);
}

bool subgraph_iter_next(const subgraph_t const *subgraph, subgraph_iter_t *iterator, vertex_t *vertex)
{
    if(iterator->type == GRAPH_ITER_ALL_VERTICES)
    {
        if(bitset_iterate_set_and_clear(&(iterator->vertices), &(iterator->next_vertex), &(iterator->last_vertex)))
        {
            *vertex = (vertex_t) iterator->next_vertex;
            return true;
        }
        
        return false;
    }
    else
    {
        while(iterator->edge_start < iterator->edge_end)
        {
            vertex_t neighbor = graph_get_edge(subgraph->base, iterator->edge_start);
            iterator->edge_start++;
            
            if(subgraph_contains_vertex(subgraph, neighbor))
            {
                *vertex = neighbor;
                return true;
            }
        }
        
        return false;
    }
}

void subgraph_iter_add_vertex(subgraph_iter_t *iterator, vertex_t vertex)
{
    if(iterator->type == GRAPH_ITER_ALL_VERTICES)
    {
        bitset_set(&(iterator->vertices), vertex);
        if(iterator->last_vertex > vertex)
            iterator->last_vertex = vertex;
    }
}

void subgraph_iter_remove_vertex(subgraph_iter_t *iterator, vertex_t vertex)
{
    if(iterator->type == GRAPH_ITER_ALL_VERTICES)
        bitset_clear(&(iterator->vertices), vertex);
}

bool subgraph_iter_contains_vertex(subgraph_iter_t *iterator, vertex_t vertex)
{
    if(iterator->type == GRAPH_ITER_NEIGHBORHOOD)
        return false;
    else
        return bitset_get(&(iterator->vertices), vertex);
}

void subgraph_iter_destroy(subgraph_iter_t *iterator)
{
    if(iterator->type == GRAPH_ITER_ALL_VERTICES)
        bitset_destroy(&(iterator->vertices));
}

/**
 * @brief Compares two vertex labels.
 * @param a First label.
 * @param b second label.
 * @returns `-1` if `a` is smaller than `b`, `0` if they are equal, or `1` if `a` is greater than `b`.
 */
int graph_cmp_vertex_labels(const void *a, const void *b)
{
    const vertex_label_t *f = (const vertex_label_t *) a;
    const vertex_label_t *g = (const vertex_label_t *) b;
    
    if(*f < *g)
        return -1;
    else if(*f > *g)
        return 1;
    else
        return 0;
}

/**
 * @brief Compares two vertex ids.
 * @param a First label.
 * @param b second label.
 * @returns `-1` if `a` is smaller than `b`, `0` if they are equal, or `1` if `a` is greater than `b`.
 */
int graph_cmp_vertices(const void *a, const void *b)
{
    const vertex_t *f = (const vertex_t *) a;
    const vertex_t *g = (const vertex_t *) b;
    
    if(*f < *g)
        return -1;
    else if(*f > *g)
        return 1;
    else
        return 0;
}

/**
 * @brief Compares two edges.
 * @param a First edge.
 * @param b second edge.
 * @returns `-1` if `a` is smaller than `b`, `0` if they are equal, or `1` if `a` is greater than `b`.
 */
int graph_cmp_edges(const void *a, const void *b)
{
    const edge_t *f = (const edge_t *) a;
    const edge_t *g = (const edge_t *) b;
    
    if(f->from < g->from)
        return -1;
    else if(f->from > g->from)
        return 1;
    else if(f->to < g->to)
        return -1;
    else if(f->to > g->to)
        return 1;
    else
        return 0;
}

/**
 * @brief Initializes a graph builder structure.
 * @param builder Pointer to uninitialized builder structure.
 */
void gbuild_init(gbuild_t *builder)
{
    builder->max_vertex_label = 0;
    stack_init(&(builder->vertex_labels), sizeof(vertex_label_t));
    stack_init(&(builder->edges), sizeof(edge_t));
}

/**
 * @brief Destroys a graph builder.
 * @details Releases all memory that was allocated.
 * @param builder Builder to destroy.
 */
void gbuild_destroy(gbuild_t *builder)
{
    stack_destroy(&(builder->vertex_labels));
    stack_destroy(&(builder->edges));
}

/**
 * @brief Adds a vertex to a graph builder.
 * @param builder Pointer to initialized builder structure.
 * @param vertex_label Label of the vertex to be added.
 */
void gbuild_add_vertex(gbuild_t *builder, vertex_label_t vertex_label)
{
    if(vertex_label > builder->max_vertex_label)
        builder->max_vertex_label = vertex_label;
    
    /* check if vertex was added before, only add if not already contained in list */
    if(!stack_binsearch(&(builder->vertex_labels), graph_cmp_vertex_labels, &vertex_label, NULL))
    {
        /* vertex_label is not in list yet, so add it and resort the list */
        stack_push(&(builder->vertex_labels), &vertex_label);
        stack_sort(&(builder->vertex_labels), graph_cmp_vertex_labels);
    }
}

/**
 * @brief Adds an edge to a graph builder.
 * @param builder Pointer to an initialized builder structure.
 * @param from Vertex label of the first endpoint of the edge.
 * @param to Vertex label of the second endpoint of the edge.
 */
void gbuild_add_edge(gbuild_t *builder, vertex_label_t from, vertex_label_t to)
{
    bool add_from, add_to;
    edge_t edge;
    
    if(from > to)
    {
        vertex_label_t tmp = from;
        from = to;
        to = tmp;
    }
    
    add_from = !stack_binsearch(&(builder->vertex_labels), graph_cmp_vertex_labels, &from, NULL);
    add_to = !stack_binsearch(&(builder->vertex_labels), graph_cmp_vertex_labels, &to, NULL);
    
    if(add_from)
        stack_push(&(builder->vertex_labels), &from);
    
    if(add_to)
        stack_push(&(builder->vertex_labels), &to);
    
    if(add_from || add_to)
        stack_sort(&(builder->vertex_labels), graph_cmp_vertex_labels);

    edge.from = from;
    edge.to = to;
        
    stack_push(&(builder->edges), &edge);

}

/**
 * @brief Returns the number of distince vertices that were added to the builder.
 * @param builder Pointer to an initialized builder structure.
 * @returns Number of distinct vertices.
 */
uint32_t gbuild_num_vertices(gbuild_t *builder)
{
    return stack_height(&(builder->vertex_labels));
}

/**
 * @brief Returns the number of edges that were added to the builder.
 * @param builder Pointer to an initialized builder structure.
 * @returns Number of edges that were added.
 */
uint32_t gbuild_num_edges(gbuild_t *builder)
{
    return stack_height(&(builder->edges));
}

/**
 * @brief Tests whether the builder has no edges.
 * @param builder Pointer to an initialized builder structure.
 * @returns `true` if the builder does not contain any edges, `false` otherwise.
 */
bool gbuild_is_trivial(gbuild_t *builder)
{
    return (stack_height(&(builder->edges)) == 0);
}

/**
 * @brief Finds the vertex id of a vertex given its label.
 * @param builder Pointer to an initialized builder structure.
 * @param vertex_label Vertex label to find the vertex id for.
 * @param result Address to store the vertex id.
 * @remark Vertex labels may change if new vertices are added later on.
 * @returns `true` if the vertex label was found, `false` otherwise.
 */
bool gbuild_get_vertex_by_label(gbuild_t *builder, const vertex_label_t vertex_label, vertex_t *result)
{
    uint32_t position;
    bool found = false;
    
    found = stack_binsearch(&(builder->vertex_labels), graph_cmp_vertex_labels, &vertex_label, &position);
    if(found)
        *result = position;
    
    return found;
}

/**
 * @brief Creates a simple, undirected graph from a builder structure.
 * @param builder Pointer to an initialized builder structure.
 * @param graph Pointer to an uninitialized graph structure.
 */
void gbuild_create_graph(gbuild_t *builder, graph_t *graph)
{
    uint32_t num_vertices = 0, num_edges = 0, num_builder_edges = 0, pos;
    uint32_t *vertex_degrees, i;
    edge_t *edge_iter;
    
    graph->num_vertices = 0;
    graph->num_edges = 0;
    graph->labels = NULL;
    graph->positions = NULL;
    graph->edges = NULL;
    
    num_vertices = gbuild_num_vertices(builder);
    if(num_vertices == 0)
        return;
    else
        graph->num_vertices = num_vertices;
        
    graph->labels = (vertex_label_t *) malloc(sizeof(vertex_label_t) * num_vertices);
    if(graph->labels == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory to store vertex labels\n");
        exit(0);
    }
    
    memcpy(graph->labels, builder->vertex_labels.data, sizeof(vertex_label_t) * num_vertices);
    
    /* if graph is trivial, we have isolated vertices and then store no edges at all */
    if(gbuild_is_trivial(builder))
    {
        graph->num_edges = 0;
        graph->positions = (uint32_t *) malloc(sizeof(uint32_t) * num_vertices);
        if(graph->positions == NULL)
        {
            fprintf(stderr, "Error: could not allocate memory to store edge positions\n");
            exit(0);
        }
        
        /* no edges => all positions = 0 */
        for(i = 0; i < num_vertices; i++)
            graph->positions[i] = 0;
            
        return;
    }
    
    /* count vertex degrees, use auxiliary array, size is O(n) */
    vertex_degrees = (uint32_t *) malloc(sizeof(uint32_t) * num_vertices);
    if(vertex_degrees == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory to store vertex degrees\n");
        exit(0);
    }
    
    /* set all degrees to zero */
    for(i = 0; i < num_vertices; i++)
        vertex_degrees[i] = 0;
    
    /* sort edge list, so we can iterate and skip duplicates */
    stack_sort(&(builder->edges), graph_cmp_edges);
    num_builder_edges = gbuild_num_edges(builder);
    
    edge_iter = (edge_t *) builder->edges.data;
    num_edges = 0;
    
    for(i = 0; i < num_builder_edges; i++, edge_iter++)
    {
        vertex_t vert_from = 0, vert_to = 0;
        
        /* skip duplicates */
        if((i > 0) && (graph_cmp_edges(edge_iter - 1, edge_iter) == 0))
            continue;
        
        /* we assume all vertex_labels can be found in the list, NO ERROR CHECKING */
        graph_get_vertex_by_label(graph, edge_iter->from, &vert_from);
        graph_get_vertex_by_label(graph, edge_iter->to, &vert_to);
        
        /* increase degree count of edge endpoints */
        vertex_degrees[vert_from]++;
        vertex_degrees[vert_to]++;
        num_edges++;
    }
    
    graph->num_edges = num_edges;
    
    /* allocate memory */
    graph->positions = (uint32_t *) malloc(sizeof(uint32_t) * num_vertices);
    if(graph->positions == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory to store edge positions\n");
        exit(0);
    }
    
    graph->positions[0] = 0;
    pos = vertex_degrees[0];
    for(i = 1; i < num_vertices; i++)
    {
        graph->positions[i] = pos;
        pos = pos + vertex_degrees[i];
    }
    
    graph->edges = (vertex_t *) malloc(sizeof(vertex_t) * 2 * num_edges);
    if(graph->edges == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory to store edges\n");
        exit(0);
    }
    
    /* go through edge list, insert edges into edge list */
    edge_iter = (edge_t *) builder->edges.data;
    
    for(i = 0; i < num_builder_edges; i++, edge_iter++)
    {
        vertex_t vert_from = 0, vert_to = 0;
        
        /* skip duplicates */
        if((i > 0) && (graph_cmp_edges(edge_iter - 1, edge_iter) == 0))
            continue;
        
        /* we assume all vertex_labels can be found in the list, NO ERROR CHECKING */
        graph_get_vertex_by_label(graph, edge_iter->from, &vert_from);
        graph_get_vertex_by_label(graph, edge_iter->to, &vert_to);
        
        graph->edges[graph->positions[vert_from]] = vert_to;
        graph->edges[graph->positions[vert_to]] = vert_from;
        graph->positions[vert_from]++;
        graph->positions[vert_to]++;
    }
    
    for(i = 0; i < num_vertices; i++)
        graph->positions[i] = graph->positions[i] - vertex_degrees[i];
    
    free(vertex_degrees);
}

/**
 * @brief Finds the id of a vertex given its label.
 * @param graph Graph.
 * @param vertex_label Label to which the vertex id needs to be found.
 * @param result Address to store the vertex id.
 * @returns `true` if the vertex id could be found, `false` otherwise.
 */
bool graph_get_vertex_by_label(const graph_t const *graph, const vertex_label_t vertex_label, vertex_t *result)
{
    uint32_t l, m, r;
    int res;
    
    l = 0;
    r = graph->num_vertices;
    
    while(l < r)
    {
        m = (l + r) / 2;
        res = graph_cmp_vertex_labels(&(graph->labels[m]), &vertex_label);
        
        if(res < 0)
            l = m + 1;
        else if(res > 0)
            r = m;
        else
        {
            if(result)
                *result = m;
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Reads a graph in DIMACS format from a file.
 * @param builder Pointer to uninitialized builder structure.
 * @param filename File to read from disk.
 * @param show_comments Comments inside the file will be printed if set to `true`.
 * @returns `false` on error, `true` on success.
 */
bool graph_load_dimacs(graph_t *graph, const char *filename, const bool show_comments)
{
    FILE *fd;
    char buffer[GRAPH_MAX_INPUT_BUFFER_SIZE];
    size_t len;
    uint32_t n, m, last_edge = 0;
    bool problem_desc = false;
    gbuild_t builder;
    
    fd = fopen(filename, "rb");
    if(fd == NULL)
    {
        fprintf(stderr, "Error: could not open input file for reading (%s)\n", strerror(errno));
        return false;
    }

    /* initialize graph builder structure to which edges from the file will be added */    
    gbuild_init(&builder);
    
    /* read line by line */
    while(fgets(buffer, GRAPH_MAX_INPUT_BUFFER_SIZE, fd))
    {
        len = strlen(buffer);
        
        /* skip unimportant lines */
        if(len <= 1)
            continue;

        /* chop off carriage-return-new-line */
        if(buffer[len - 1] == '\n')
            buffer[--len] = '\0';
        if(len && (buffer[len - 1] == '\r'))
            buffer[--len] = '\0';        
        
        if((buffer[0] == 'c') && show_comments)
        {
            /* c This is an example of a comment line. */
            fprintf(stdout, "Info: %s\n", buffer);
        }
        else if(buffer[0] == 'p')
        {
            /* p FORMAT NODES EDGES */
            if(problem_desc)
            {
                fprintf(stderr, "Error: multiple 'problem' descriptions in input file\n");
                gbuild_destroy(&builder);
                fclose(fd);
                return false;
            }
            else
            {
                if(sscanf(buffer, "p edge %u %u", &n, &m) != 2)
                {
                    fprintf(stderr, "Error: could not parse 'problem' description in input file\n");
                    gbuild_destroy(&builder);
                    fclose(fd);
                    return false;
                }
                else
                    problem_desc = true;
            }
        }
        else if(buffer[0] == 'e')
        {
            int u, v;
            /* e W V */
            
            if(sscanf(buffer, "e %i %i", &u, &v) != 2)
            {
                fprintf(stderr, "Error: could not parse 'edge' description\n");
                gbuild_destroy(&builder);
                fclose(fd);
                return false;
            }
            else if((u <= 0) || (v <= 0) || (u == v))
            {
                fprintf(stderr, "Error: invalid range for vertex ids in 'edge' description\n");
                gbuild_destroy(&builder);
                fclose(fd);
                return false;
            }
            else if(last_edge == m)
            {
                fprintf(stderr, "Error: too many edges in input file\n");
                gbuild_destroy(&builder);
                fclose(fd);
                return false;
            }
            else
            {
                if(u > v)
                {
                    int t = v;
                    v = u;
                    u = t;
                }
                
                gbuild_add_edge(&builder, u, v);
                last_edge++;
            }
        }
    }
    
    fclose(fd);
    
    gbuild_create_graph(&builder, graph);
    gbuild_destroy(&builder);
    return true;
}

