#include "malloc.h"

t_alloc_list	*g_alloc_lst = NULL;

/*
*	Finds the first available node in offset (usually offset == g_alloc_lst).
*	If the node after that is in use, calculates the size of the gap and
*	determines if the desired allocation will fit.
*		If it won't fit, find the next available node and start over. This means calling alloc_from_zone() with offset = t
*		If at the end it still won't fit, call mmap() ????
*	If the node after that is NOT in use, simply allocates 'size' bytes in the zone.
*/

char	*alloc_from_zone(size_t size, char *zone_ptr, u_int16_t alloc_type)
{
	t_alloc_list	*t = g_alloc_lst;
	t_alloc_list	*prev = NULL;
	u_int32_t		gap_size = 0;

	while (t != NULL)
	{
		while (t != NULL && t->alloc_size != 0)
		{
			prev = t;
			t = t->next;
		}
		if (t != NULL)
		{
			if (t->next != NULL && t->next->alloc_size == 0)
			{
				t->alloc_size = size;
				if (prev != NULL)
					t->ptr = calc_alignment_padding(prev);
				else
					t->ptr = t;
				return (t->ptr);
			}
			else
			{
				if (t->next == NULL)
					gap_size = (size_t)(zone_ptr + alloc_type - t->alloc_size);
				else
					gap_size = (size_t)(prev->ptr + prev->alloc_size) - (size_t)t->next->ptr;
				if (size <= gap_size)
				{
					t->alloc_size = size;
					t->ptr = calc_alignment_padding(prev);
				}
				else
					continue ;
			}
		}
	}
	//we reached the end of g_alloc_lst without finding a single available node
	write(2, "oopsie\n", 8);
	return (NULL);
}

/*
*	For the two functions below, it's important NOT to take memory
*	from the region that's being used to store the alloc list!
*/

char	*alloc_from_small_zone(size_t size, char *zone_ptr)
{
	return (alloc_from_zone(size, zone_ptr, SMALL_ALLOC));
}

char	*alloc_from_medium_zone(size_t size, char *zone_ptr)
{
	return (alloc_from_zone(size, zone_ptr, MEDIUM_ALLOC));
}

/*
*	Preallocates the small and medium zones as specified in the subject,
*	and creates the allocation list.
*/
void	prealloc_zones(char **small_ptr, char **medium_ptr)
{
	char		 	*small = NULL;
	char 			*medium = NULL;
	t_alloc_list	*prev = NULL;

	small = mmap(NULL, SMALL_ALLOC, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	medium = mmap(NULL, MEDIUM_ALLOC, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (small == MAP_FAILED || medium == MAP_FAILED)
	{
		perror("mmap");
		exit(EXIT_FAILURE);
	}
	g_alloc_lst = (t_alloc_list *)(small + SMALL_ALLOC - (SMALL_ALLOC / 100 * 90));
	for (size_t i = (size_t)g_alloc_lst; (char *)(i + 24) < small + SMALL_ALLOC; i += 24)
	{
		g_alloc_lst->alloc_size = 0;
		g_alloc_lst->ptr = NULL;
		g_alloc_lst->next = (t_alloc_list *)(i + 24);
		prev = g_alloc_lst;
		g_alloc_lst = g_alloc_lst->next;
	}
	prev->next = (t_alloc_list *)(medium + MEDIUM_ALLOC - (MEDIUM_ALLOC / 100 * 95));
	g_alloc_lst = prev->next;
	for (size_t i = (size_t)g_alloc_lst; (char *)(i + 24) < medium + MEDIUM_ALLOC; i += 24)
	{
		g_alloc_lst->alloc_size = 0;
		g_alloc_lst->ptr = NULL;
		g_alloc_lst->next = (t_alloc_list *)(i + 24);
		prev = g_alloc_lst;
		g_alloc_lst = g_alloc_lst->next;
	}
	prev->next = NULL;
	g_alloc_lst = (t_alloc_list *)(small + SMALL_ALLOC - (SMALL_ALLOC / 100 * 90));
	*small_ptr = small;
	*medium_ptr = medium;

	// for (t_alloc_list *t = g_alloc_lst; t != NULL; t = t->next)
	// 	write(1, "hi\n", 3);
}

void	*my_malloc(size_t size)
{
	char				*addr = NULL;
	char				*small;
	char				*medium;
	static u_int64_t	g_alloc_lst_size = 0;

	if (g_alloc_lst == NULL || g_alloc_lst_size == 0)
	{
		prealloc_zones(&small, &medium);
		(void)show_alloc_mem_set(small, medium);
	}
	if (size <= MEDIUM_ENTRY)
	{
		if (size == 0)
			return (NULL);
		if (size <= SMALL_ENTRY)
			addr = alloc_from_small_zone(size, small);
		else
			addr = alloc_from_medium_zone(size, medium);
	}
	if (size > MEDIUM_ENTRY || addr == NULL)
	{
		addr = mmap(NULL, size, PROT_READ | PROT_WRITE, 
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (addr == MAP_FAILED)
		{
			perror("mmap");
			exit(EXIT_FAILURE);
		}
		printf("MMAP CALLED\n");/////////
	}
	update_alloc_lst(size, addr, ADD_ENTRY, &g_alloc_lst_size);
	return (addr);
}

 /*
int main(void)
{
	 char *a = my_malloc(13 + 1);	//small
	 char *b = my_malloc(42 + 1);	//medium
	 char *c = my_malloc(222 + 1);	//large
 show_alloc_mem();

 my_free(a);
 my_free(b);
 my_free(c);

	char *s[100];

	for (int i = 0; i < 100; i++)
	{
		s[i] = my_malloc(i + 1);
		printf("%d\n", i);
	}
	for (int i = 0; i < 100; i++)
	{
		my_free(s[i]);
	}
	return (0);
}
*/
