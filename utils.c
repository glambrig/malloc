#include "malloc.h"

/*
*	Adds padding to the block if necessary, and returns the new allocation
*/
void	*calc_alignment_padding(t_alloc_list *node)
{
	if (node != NULL && node->ptr + node->alloc_size % WORD_SIZE != 0)
		return ((node->ptr + node->alloc_size) + ((size_t)(node->ptr + node->alloc_size) % WORD_SIZE));
	return (node->ptr + node->alloc_size);
}

/*
*	Yes, there's an overhead to the subfunctions, 
*	but it looks way better than crunching them both into update_alloc_lst()
*/

static void update_alloc_lst_add(size_t size, void *ptr, u_int64_t *g_alloc_lst_size)
{
	t_alloc_list *new = NULL;

	for (t_alloc_list *t = g_alloc_lst; t != NULL; t = t->next)
	{
		if (t->ptr == NULL)
		{
			new = t;
			break ;
		}
	}
	if (new == NULL)
	{
		//haven't implemented yet, rm printf+exit once it is
		// get_new_alloc_lst_nodes(&new);
		printf("UH OH\n");
		exit(EXIT_FAILURE);
	}
	new->alloc_size = size;
	new->ptr = ptr;
	if (g_alloc_lst == NULL || *g_alloc_lst_size == 0)
		g_alloc_lst = new;
	(*g_alloc_lst_size)++;
	printf("added entry of size %ld with address %p\n", size, ptr);//////////
}

static int update_alloc_lst_rm(void *ptr, u_int64_t *g_alloc_lst_size)
{
	t_alloc_list	*temp = g_alloc_lst;
	size_t			last_alloc_size;

	if (g_alloc_lst == NULL || g_alloc_lst_size == NULL || *g_alloc_lst_size == 0)
	{
		write(2, "free: error: double free\n", 26);
		exit(EXIT_FAILURE);
	}
	while (temp != NULL)
	{
		if (ptr == temp->ptr)
		{
			printf("removed entry of size %ld with address %p\n", temp->alloc_size, ptr);//////////
			//not sure about this. i'm assuming that allocations
			//within the zones are just marked as freed and reset to default
			last_alloc_size = temp->alloc_size;
			temp->alloc_size = 0;
			temp->ptr = NULL;
			(*g_alloc_lst_size)--;
			return (last_alloc_size);
		}
		temp = temp->next;
	}
	return (0);
}

/*
*	'size' is the size of the allocation
*	'ptr' is the pointer to the memory that was allocated (malloc()), or to be deallocated (free())
*	'method' determines whether the alloc list will be expanded, or shortened.
*	'g_alloc_lst_size' is a pointer to the size of the alloc list. free() passes NULL instead.
*
*
*	'nullcase_ptr' points to the real g_alloc_lst_size.
*	This allows us to use the real value of g_alloc_lst_size,
*		since free() passes NULL.
*/
int	update_alloc_lst(size_t size, void *ptr, bool method, u_int64_t *g_alloc_lst_size)
{
	static u_int64_t	*nullcase_ptr = NULL;

	if (method == ADD_ENTRY)
	{
		nullcase_ptr = g_alloc_lst_size;
		update_alloc_lst_add(size, ptr, g_alloc_lst_size);
	}
	else if (method == RM_ENTRY)
		return (update_alloc_lst_rm(ptr, nullcase_ptr));
	return (0);
}

/*
*	TINY : 0xA0000
*	0xA0020 - 0xA004A : 42 bytes
*	0xA006A - 0xA00BE : 84 bytes
*	SMALL : 0xAD000
*	0xAD020 - 0xADEAD : 3725 bytes
*	LARGE : 0xB0000
*	0xB0020 - 0xBBEEF : 48847 bytes
*	Total : 52698 bytes
*/

void	show_alloc_mem(void)
{
	t_show_alloc_mem ptrs;

	ptrs = show_alloc_mem_set(NULL, NULL);

	printf("TINY : %p\n", ptrs.small_ptr);
	for (t_alloc_list *t = g_alloc_lst; t != NULL && t->alloc_size <= SMALL_ENTRY; t = t->next)
		printf("%p - %p : %ld bytes\n", t->ptr, (t->ptr + t->alloc_size), t->alloc_size);

	printf("SMALL : %p\n", ptrs.medium_ptr);
	for (t_alloc_list *t = g_alloc_lst; t != NULL && t->alloc_size <= MEDIUM_ENTRY; t = t->next)
	{
		if (t->alloc_size > SMALL_ENTRY)
			printf("%p - %p : %ld bytes\n", t->ptr, (t->ptr + t->alloc_size), t->alloc_size);
	}

	for (t_alloc_list *t = g_alloc_lst; t != NULL; t = t->next)
	{
		bool print = true;

		if (t->alloc_size > MEDIUM_ENTRY)
		{
			if (print == true)
				printf("LARGE : %p\n", t->ptr);
			printf("%p - %p : %ld bytes\n", t->ptr, (t->ptr + t->alloc_size), t->alloc_size);
			print = false;
		}
	}
}

t_show_alloc_mem	show_alloc_mem_set(char *small_ptr, char *medium_ptr)
{
	static t_show_alloc_mem	ptrs;

	if (small_ptr == NULL || medium_ptr == NULL)
		return (ptrs);
	ptrs.small_ptr = small_ptr;
	ptrs.medium_ptr = medium_ptr;
	return (ptrs);
}