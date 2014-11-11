#include "union_find.h"
#include <string.h>

/**
 * @brief Initializes a union-find data structure.
 * @details Each node belongs to its own component after initialization.
 * @param uf Pointer to uninitialized uf data structure.
 * @param num_nodes Number of nodes that need to be kept track of.
 * @remark Allocates O(`num_nodes`) memory.
 */
void uf_init(uf_t *uf, uf_node_t num_nodes)
{
    uf_node_t i;
    
    if(uf == NULL)
    {
        fprintf(stderr, "Error: null pointer in uf_init\n");
        exit(0);
    }
    
    if(num_nodes == 0)
    {
        fprintf(stderr, "Error: can't create union-find data structure without nodes\n");
        exit(0);
    }
    
    uf->num_nodes = num_nodes;
    uf->parents = (uf_node_t *) malloc(sizeof(uf_node_t) * num_nodes);
    if(uf->parents == NULL)
    {
        fprintf(stderr, "Error: could not allocate memory for union-find data structure\n");
        exit(0);
    }
    
    for(i = 0; i < num_nodes; i++)
        uf->parents[i] = i;
}

/**
 * @brief Create a copy of  a union-find data structure.
 * @details Copies data from one uf structure to another. Both structures need to be initialized with the same size.
 * @param dest Pointer to initialized uf data structure.
 * @param src Pointer to initialized uf data structure of same size as dest
 */
void uf_copy(uf_t *dest, uf_t *src)
{
    if((dest == NULL) || (src == NULL))
    {
        fprintf(stderr, "Error: null pointer in uf_copy\n");
        exit(0);
    }
    
    if(dest->num_nodes != src->num_nodes)
    {
        fprintf(stderr, "Error: can't copy union-find data structure if initialized with different size\n");
        exit(0);
    }
    
    memcpy(dest->parents, src->parents, sizeof(uf_node_t) * dest->num_nodes);
}

/**
 * @brief Destroys a union-find data structure.
 * @details Releases all memory that was allocated by uf_find.
 * @param uf Pointer to initialized uf data structure.
 */
void uf_destroy(uf_t *uf)
{
    if(uf == NULL)
    {
        fprintf(stderr, "Error: null pointer in uf_destroy\n");
        exit(0);
    }
    
    if(uf->parents)
        free(uf->parents);

    uf->num_nodes = 0;
    uf->parents = NULL;
}

/**
 * @brief Finds the component a node belongs to.
 * @details Returns the node that is the parent of the component the node with id `node` belongs to.
 * @param uf Pointer to initialized uf data structure.
 * @param node Node for which the component is to be determined.
 * @remark Performs path compression to speed up further queries.
 */
uf_node_t uf_find(uf_t *uf, uf_node_t node)
{
    uf_node_t x, p;
    
    if(uf == NULL)
    {
        fprintf(stderr, "Error: null pointer in uf_find\n");
        exit(0);
    }
    
    if(node >= uf->num_nodes)
    {
        fprintf(stderr, "Error: out of bounds in uf_find\n");
        exit(0);
    }
    
    /* traverse up to the root of the tree */
    x = node;
    while(uf->parents[x] != x)
        x = uf->parents[x];

    /* path compression */
    p = x;
    x = node;
    while(uf->parents[x] != x)
    {
        node = x;
        x = uf->parents[x];
        uf->parents[node] = p;
    }
    
    return p;
}

/**
 * @brief Unites two components.
 * @details Unites the compoments the nodes `u` and `v` belong to.
 * @param uf Pointer to initialized uf data structure.
 * @param u Node that belongs to the first component.
 * @param v Node that belongs to the second component.
 */
void uf_union(uf_t *uf, uf_node_t u, uf_node_t v)
{
    if(uf == NULL)
    {
        fprintf(stderr, "Error: null pointer in uf_union\n");
        exit(0);
    }
    
    u = uf_find(uf, u);
    v = uf_find(uf, v);
    
    if(u != v)
        uf->parents[u] = v;
}

