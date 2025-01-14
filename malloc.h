#ifndef MALLOC_H
#define MALLOC_H

/*
*	SMALL_ALLOC and MEDIUM_ALLOC refer to the sizes of the preallocated zones
*	SMALL_ENTRY and MEDIUM_ENTRY refer to the size of a single entry in their respective zones
*/

#define PAGESIZE sysconf(_SC_PAGESIZE)
#define SMALL_ALLOC PAGESIZE
#define MEDIUM_ALLOC PAGESIZE * 5
#define SMALL_ENTRY 36
#define MEDIUM_ENTRY 194
#define ADD_ENTRY 1
#define RM_ENTRY 0


#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "libft/libft.h"

typedef struct s_alloc_list
{
	size_t	alloc_size;
	void	*ptr;
	struct s_alloc_list	*next;
}	t_alloc_list;

extern t_alloc_list	*g_alloc_lst;

void	*my_malloc(size_t size);
void	my_free(void *ptr);
void	*my_realloc(void *ptr, size_t size);

int		update_alloc_lst(size_t size, void *ptr, bool method, u_int64_t *g_alloc_lst_size);

void	show_alloc_mem();

#endif
