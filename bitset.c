#include <assert.h>
#include "bitset.h"

/**
 * @brief Creates a new bitset data structure.
 * @details Allocates memory for a bitset that can contain values from the range `[0, num_values - 1]` and initializes it using bitset_init.
 * @param num_values Number of values the set can store.
 * @returns Pointer to a bitset data structure.
 * @remark Memory needs to be free'd by bitset_free.
 */
bitset_t *bitset_new(bitset_index_t num_values)
{
    bitset_t *set = (bitset_t *) malloc(sizeof(bitset_t));
    if(set == NULL)
    {
        fprintf(stderr, "[bitset] Error: could not allocate memory to store bitset\n");
        exit(0);
    }
    
    bitset_init(set, num_values);
    return set;
}

/**
 * @brief Initializes a new bitset data structure.
 * @details Initializes a bitset so that it can contain values from the range `[0, num_values - 1]`.
 * @param set Pointer to an unitialized bitset data structure.
 * @param num_values Number of values the set can store.
 * @remark Allocates O(`num_values`) memory.
 */
void bitset_init(bitset_t *set, bitset_index_t num_values)
{
#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

    set->max = num_values;
    set->bits = (bitset_data_t *) malloc(sizeof(bitset_data_t) * BITSET_NUM_DATA_ELEMENTS(num_values));
    if(set->bits == NULL)
    {
        fprintf(stderr, "[bitset] Error: could not allocate memory to store bitset data\n");
        exit(0);
    }
    
    bitset_clear_all(set);
}

/**
 * @brief Creates a copy of a bitset data structure.
 * @details Initializes `set` with the same size of `source` and copies all values of `source` to `set`.
 * @param set Pointer to an unitialized bitset data structure.
 * @param source Pointer to an itialized bitset data structure that is to be copied.
 */
void bitset_init_copy(bitset_t *set, const bitset_t const *source)
{
#ifdef BITSET_ASSERTIONS
    assert(set);
    assert(source);
#endif

    bitset_init(set, source->max);
    bitset_copy(set, source);
}

/**
 * @brief Copies values from one set to another.
 * @details Copies all values from `source` to `set`.
 * @param set Pointer to an initialized bitset data structure.
 * @param source Pointer to an initialized bitset data structure that is to be copied.
 * @remark If boundary checking is activated, both sets need to be of the same size.
 */
void bitset_copy(bitset_t *set, const bitset_t const *source)
{
#ifdef BITSET_ASSERTIONS
    assert(set);
    assert(source);
#endif
    
#ifdef BITSET_BOUNDS_CHECKING
    if(set->max != source->max)
    {
        fprintf(stderr, "[bitset] Error: out of bounds 1\n");
        exit(0);
    }
#endif

    memcpy(set->bits, source->bits, BITSET_NUM_DATA_ELEMENTS(set->max) * sizeof(bitset_data_t));
}

/**
 * @brief Destroys a bitset data structure.
 * @details Releases all memory that is used to store the elements of this set.
 * @param set Pointer to an initialized bitset data structure.
 */
void bitset_destroy(bitset_t *set)
{
#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

    if(set->bits)
        free(set->bits);
    set->bits = NULL;
}

/**
 * @brief Destroys a bitset data structure that was created by bitset_new.
 * @details Destroys the set and then releases its memory.
 * @param set Pointer to an initialized bitset data structure.
 */
void bitset_free(bitset_t *set)
{
#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

    bitset_destroy(set);
    if(set)
        free(set);
}

/**
 * @brief Adds an element to a set.
 * @details Adds the element `index` to the bitset.
 * @param set Pointer to an initialized bitset data structure.
 * @param index Element to be added to the bitset.
 */
void bitset_set(bitset_t *set, bitset_index_t index)
{
    bitset_index_t byte = index / BITSET_BITS_PER_ELEMENT;
    bitset_index_t bit = index % BITSET_BITS_PER_ELEMENT;

#ifdef BITSET_ASSERTIONS
    assert(set);
#endif
    
#ifdef BITSET_BOUNDS_CHECKING
    if(set->max < index)
    {
        fprintf(stderr, "[bitset] Error: index out of bounds 2\n");
        exit(0);
    }
#endif
    
    set->bits[byte] = set->bits[byte] | (bitset_data_t) ((bitset_data_t) 1 << (bitset_data_t) bit);
}

/**
 * @brief Adds all possible elements to a set.
 * @details Every element from the range `[0, num_values - 1]` is added to `set` if it can store `num_values` elements.
 * @param set Pointer to an initialized bitset data structure.
 */
void bitset_set_all(bitset_t *set)
{
    bitset_index_t last_bit, last_element, i;
#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

    last_bit = set->max % BITSET_BITS_PER_ELEMENT;
    last_element = BITSET_NUM_DATA_ELEMENTS(set->max);
    for(i = 0; i < last_element; i++)
        set->bits[i] = set->bits[i] | ~set->bits[i];
    
    if(last_bit > 0)
        set->bits[last_element - 1] = set->bits[last_element - 1] & (((bitset_data_t) 1 << (bitset_data_t) (last_bit)) - 1);
}

/**
 * @brief Removes an element from a set.
 * @details Removes the element `index` from the bitset.
 * @param set Pointer to an initialized bitset data structure.
 * @param index Element to be removed from the bitset.
 */
void bitset_clear(bitset_t *set, bitset_index_t index)
{
    bitset_index_t byte = index / BITSET_BITS_PER_ELEMENT;
    bitset_index_t bit = index % BITSET_BITS_PER_ELEMENT;

#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

#ifdef BITSET_BOUNDS_CHECKING
    if(set->max < index)
    {
        fprintf(stderr, "[bitset] Error: index out of bounds 3\n");
        exit(0);
    }
#endif

    set->bits[byte] = set->bits[byte] & (bitset_data_t) ~((bitset_data_t) 1 << (bitset_data_t) bit);
}

/**
 * @brief Removes all elements from a set.
 * @details Removes all elements that are contained in `set`.
 * @param set Pointer to an initialized bitset data structure.
 */
void bitset_clear_all(bitset_t *set)
{
#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

    memset(set->bits, 0, BITSET_NUM_DATA_ELEMENTS(set->max) * sizeof(bitset_data_t));
}

/**
 * @brief Checks if an element is contained in a set.
 * @details Checks whether the the element `index` is contained in `set`.
 * @param set Pointer to an initialized bitset data structure.
 * @param index Element to be checked for containment.
 */
bool bitset_get(const bitset_t const *set, const bitset_index_t index)
{
    bitset_index_t byte = index / BITSET_BITS_PER_ELEMENT;
    bitset_index_t bit = index % BITSET_BITS_PER_ELEMENT;

#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

#ifdef BITSET_BOUNDS_CHECKING
    if(set->max < index)
    {
        fprintf(stderr, "[bitset] Error: index out of bounds 4\n");
        exit(0);
    }
#endif
    
    return ((set->bits[byte] & (bitset_data_t) ((bitset_data_t) 1 << (bitset_data_t) bit)) > 0);
}

/**
 * @brief Toggles an element in a set.
 * @details Removes an element from a set if it is present, or adds it if it is not.
 * @param set Pointer to an initialized bitset data structure.
 * @param index Element to be toggled in the bitset.
 */
void bitset_toggle(bitset_t *set, bitset_index_t index)
{
    bitset_index_t byte = index / BITSET_BITS_PER_ELEMENT;
    bitset_index_t bit = index % BITSET_BITS_PER_ELEMENT;

#ifdef BITSET_ASSERTIONS
    assert(set);
#endif

#ifdef BITSET_BOUNDS_CHECKING
    if(set->max < index)
    {
        fprintf(stderr, "[bitset] Error: index out of bounds 5\n");
        exit(0);
    }
#endif

    set->bits[byte] = set->bits[byte] ^ (bitset_data_t) ((bitset_data_t) 1 << (bitset_data_t) bit);
}

/**
 * @brief Finds the first element contained in a set.
 * @details Finds the smallest element that is contained in `set`.
 * @param set Pointer to an initialized bitset data structure.
 * @param result_bit Address to store the smallest element that is contained in the set.
 * @returns `true` if an element was found, `false` if the set is empty.
 */
bool bitset_find_set_bit(const bitset_t const *set, bitset_index_t *result_bit)
{
    bitset_index_t byte, max_byte, bit = 0;
    bitset_data_t data;

#ifdef BITSET_ASSERTIONS
    assert(set);
    assert(result_bit);
#endif

    max_byte = BITSET_NUM_DATA_ELEMENTS(set->max);
    for(byte = 0; byte < max_byte; byte++)
    {
        if(set->bits[byte] > 0)
        {
            data = set->bits[byte];
            while((data & 1) == 0)
            {
                data = data >> 1;
                bit++;
            }
            
            *result_bit = byte * BITSET_BITS_PER_ELEMENT + bit;
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Finds the smallest element that is contained in set and removes it.
 * @details Finds the smallest element greater (almost) than `last_bit` that is contained in `set` and removes it.
 * @param set Pointer to an initialized bitset data structure.
 * @param next_bit Address to store the smallest element that was found and removed.
 * @param last_bit Address to element that was removed during the last call to this function; ignored if `NULL`.
 * @returns `true` if an element was found, `false` if the set is empty.
 */
bool bitset_iterate_set_and_clear(bitset_t *set, bitset_index_t *next_bit, bitset_index_t *last_bit)
{
    bitset_index_t byte = 0, max_byte, bit = 0;
    bitset_data_t data;

#ifdef BITSET_ASSERTIONS
    assert(set);
    assert(next_bit);
#endif
    
    max_byte = BITSET_NUM_DATA_ELEMENTS(set->max);
    
    if(last_bit)
        byte = *last_bit / BITSET_BITS_PER_ELEMENT;

    for(; byte < max_byte; byte++)
    {
        if(set->bits[byte] > 0)
        {
            data = set->bits[byte];
            
            while((data & 1) == 0)
            {
                data = data >> 1;
                bit++;
            }
            
            set->bits[byte] = set->bits[byte] & (bitset_data_t) ~((bitset_data_t) 1 << (bitset_data_t) bit);
            *next_bit = byte * BITSET_BITS_PER_ELEMENT + bit;
            
            if(last_bit)
                *last_bit = *next_bit;
            
            return true;
        }
    }
    
    return false;
}

/**
 * @brief Prints out all elements that are contained in a set.
 * @details Prints a list like `[1, 5, 100]`.
 * @param set Pointer to an initialized bitset data structure.
 */
void bitset_print(const bitset_t const *set)
{
    bitset_index_t i;
    bool append = false;
    
    printf("[");
    for(i = 0; i < set->max; i++)
    {
        if(!bitset_get(set, i))
            continue;
        
        if(append)
            printf(", %u", i);
        else
        {
            append = true;
            printf("%u", i);
        }
    }
    
    printf("]\n");
}

/**
 * @brief Removes a set of elements from a set.
 * @details Removes all elements from `set` that are also contained in `remove_set`,
 * @param set Pointer to an initialized bitset data structure.
 * @param remove_set Pointer to an initialized bitset data structure.
 * @remark The sets need to be of the same size.
 */
void bitset_remove_set(bitset_t *set, const bitset_t const *remove_set)
{
    bitset_index_t last_element, i;
    
#ifdef BITSET_ASSERTIONS
    assert(set);
    assert(remove_set);
#endif

#ifdef BITSET_BOUNDS_CHECKING
    if(set->max != remove_set->max)
    {
        fprintf(stderr, "Error: comparing bitsets of different size!\n");
        exit(0);
    }
#endif

    last_element = BITSET_NUM_DATA_ELEMENTS(set->max);
    for(i = 0; i < last_element; i++)
        set->bits[i] = set->bits[i] & ~remove_set->bits[i];
}

/**
 * @brief Checks whether a set of elements is contained in a set.
 * @details Checks whether all elements from `subset` are also contained in `set`.
 * @param set Pointer to an initialized bitset data structure.
 * @param subset Pointer to an initialized bitset data structure.
 * @returns `true` if `subset` is a subset of ?set, `false` otherwise.
 * @remark Both sets need to be of the same size.
 */
bool bitset_contains_set(const bitset_t const *set, const bitset_t const *subset)
{
    bitset_index_t i, last_element;

#ifdef BITSET_ASSERTIONS
    assert(set);
    assert(subset);
#endif

#ifdef BITSET_BOUNDS_CHECKING
    if(set->max != subset->max)
    {
        fprintf(stderr, "Error: checking containment of sets of different size!\n");
        exit(0);
    }
#endif

    last_element = BITSET_NUM_DATA_ELEMENTS(set->max);
    for(i = 0; i < last_element; i++)
    {
        /* check if (set & subset) == subset */
        if((set->bits[i] & subset->bits[i]) != subset->bits[i])
            return false;
    }
    
    return true;
}

/**
 * @brief Compares two sets.
 * @details Two sets are equal if they contain the same elements. A set `a` is smaller than a set `b` if `b` contains an element `x` such that `x` is not contained in `a` but every element `y` that is smaller than `x` is in `a` if and only if it also is is `b`. Such an element `x` is the smallest element for which containment in `a` and `b` differs.
 * @param a Pointer to an initialized bitset data structure.
 * @param b Pointer to an initialized bitset data structure.
 * @returns `-1? if `a < b`, `0` if `a = b`, or `1` if `a > b`.
 */
int bitset_cmp(const bitset_t const *a, const bitset_t const *b)
{
#ifdef BITSET_ASSERTIONS
    assert(a);
    assert(b);
#endif

#ifdef BITSET_BOUNDS_CHECKING
    if(a->max != b->max)
    {
        fprintf(stderr, "Error: comparing bitsets of different size!\n");
        exit(0);
    }
#endif

    return memcmp(a->bits, b->bits, BITSET_NUM_DATA_ELEMENTS(a->max) * sizeof(bitset_data_t));
}

