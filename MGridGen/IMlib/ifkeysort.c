#include "IMlib.h"


void ifkeysort2(int, FKeyValueType *);

/* Byte-wise swap two items of size SIZE. */
#define QSSWAP(a, b, stmp) do { stmp = (a); (a) = (b); (b) = stmp; } while (0)

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 20

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct {
  FKeyValueType *lo;
  FKeyValueType *hi;
} stack_node;


/* The next 4 #defines implement a very fast in-line stack abstraction. */
#define STACK_SIZE	(32 * sizeof(unsigned long int))
#define PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(bottom < top)


void ifkeysort(int total_elems, FKeyValueType *pbase)
{
  int i, nflips;
  FKeyValueType pivot, stmp;

  for (nflips = 0, i=0; i<total_elems-1; i++) {
    if (pbase[i].key > pbase[i+1].key)
      nflips++;
  }

  if (nflips < .05*total_elems) {
    ifkeysort2(total_elems, pbase);
    return;
  }
  
  if (total_elems == 0)
    /* Avoid lossage with unsigned arithmetic below.  */
    return;

  if (total_elems > MAX_THRESH) {
    FKeyValueType *lo = pbase;
    FKeyValueType *hi = &lo[total_elems - 1];
    stack_node stack[STACK_SIZE]; 
    stack_node *bottom = stack+1, *top = stack + 2;

    stack[1].lo = stack[1].hi = NULL;
    while (STACK_NOT_EMPTY) {
      FKeyValueType *left_ptr;
      FKeyValueType *right_ptr;
      FKeyValueType *mid = lo + ((hi - lo) >> 1);

      if (mid->key < lo->key) 
        QSSWAP(*mid, *lo, stmp);
      if (hi->key < mid->key)
        QSSWAP(*mid, *hi, stmp);
      else
        goto jump_over;
      if (mid->key < lo->key)
        QSSWAP(*mid, *lo, stmp);

jump_over:;
      pivot = *mid;
      left_ptr  = lo + 1;
      right_ptr = hi - 1;

      /* Here's the famous ``collapse the walls'' section of quicksort.
	 Gotta like those tight inner loops!  They are the main reason
	 that this algorithm runs much faster than others. */
      do {
	while (left_ptr->key < pivot.key)
	  left_ptr++;

	while (pivot.key < right_ptr->key)
	  right_ptr--;

	if (left_ptr < right_ptr) {
	  QSSWAP (*left_ptr, *right_ptr, stmp);
	  left_ptr++;
	  right_ptr--;
	}
	else if (left_ptr == right_ptr) {
	  left_ptr++;
	  right_ptr--;
	  break;
	}
      } while (left_ptr <= right_ptr);

      /* Set up pointers for next iteration.  First determine whether
         left and right partitions are below the threshold size.  If so,
         ignore one or both.  Otherwise, push the larger partition's
         bounds on the stack and continue sorting the smaller one. */

      if ((size_t) (right_ptr - lo) <= MAX_THRESH) {
        if ((size_t) (hi - left_ptr) <= MAX_THRESH) {
	  /* Ignore both small partitions. */
          POP (lo, hi);
        }
        else
	  /* Ignore small left partition. */
          lo = left_ptr;
      }
      else if ((size_t) (hi - left_ptr) <= MAX_THRESH)
	/* Ignore small right partition. */
        hi = right_ptr;
      else if ((right_ptr - lo) > (hi - left_ptr)) {
       /* Push larger left partition indices. */
       PUSH (lo, right_ptr);
       lo = left_ptr;
      }
      else {
	/* Push larger right partition indices. */
        PUSH (left_ptr, hi);
        hi = right_ptr;
      }
    }
  }

  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */

  {
    FKeyValueType *end_ptr = &pbase[total_elems - 1];
    FKeyValueType *tmp_ptr = pbase;
    FKeyValueType *thresh = (end_ptr < pbase + MAX_THRESH ? end_ptr : pbase + MAX_THRESH);
    register FKeyValueType *run_ptr;

    /* Find smallest element in first threshold and place it at the
       array's beginning.  This is the smallest array element,
       and the operation speeds up insertion sort's inner loop. */

    for (run_ptr = tmp_ptr + 1; run_ptr <= thresh; run_ptr++)
      if (run_ptr->key < tmp_ptr->key)
        tmp_ptr = run_ptr;

    if (tmp_ptr != pbase)
      QSSWAP(*tmp_ptr, *pbase, stmp);

    /* Insertion sort, running from left-hand-side up to right-hand-side.  */
    run_ptr = pbase + 1;
    while (++run_ptr <= end_ptr) {
      tmp_ptr = run_ptr - 1;
      while (run_ptr->key < tmp_ptr->key)
        tmp_ptr--;

      tmp_ptr++;
      if (tmp_ptr != run_ptr) {
        FKeyValueType elmnt = *run_ptr;
        FKeyValueType *mptr;

        for (mptr=run_ptr; mptr>tmp_ptr; mptr--)
          *mptr = *(mptr-1);
        *mptr = elmnt;
      }
    }
  }

#ifdef DEBUG
  {
    int i;
    for (i=1; i<total_elems; i++) {
      if (pbase[i].key < pbase[i-1].key)
        printf("Sorting Error! [%f %f]\n", pbase[i-1].key, pbase[i].key);
    }
  }
#endif
}

void ifkeysort2(int total_elems, FKeyValueType *pbase)
{
  FKeyValueType stmp;
  FKeyValueType *end_ptr = &pbase[total_elems - 1];
  FKeyValueType *tmp_ptr = pbase;
  FKeyValueType *thresh = end_ptr;
  register FKeyValueType *run_ptr;

  /* Find smallest element in first threshold and place it at the
     array's beginning.  This is the smallest array element,
     and the operation speeds up insertion sort's inner loop. */

  for (run_ptr = tmp_ptr + 1; run_ptr <= thresh; run_ptr++)
    if (run_ptr->key < tmp_ptr->key)
      tmp_ptr = run_ptr;

  if (tmp_ptr != pbase)
    QSSWAP(*tmp_ptr, *pbase, stmp);

  /* Insertion sort, running from left-hand-side up to right-hand-side.  */
  run_ptr = pbase + 1;
  while (++run_ptr <= end_ptr) {
    tmp_ptr = run_ptr - 1;
    while (run_ptr->key < tmp_ptr->key)
      tmp_ptr--;

    tmp_ptr++;
    if (tmp_ptr != run_ptr) {
      FKeyValueType elmnt = *run_ptr;
      FKeyValueType *mptr;

      for (mptr=run_ptr; mptr>tmp_ptr; mptr--)
        *mptr = *(mptr-1);
      *mptr = elmnt;
    }
  }

#ifdef DEBUG
  {
    int i;
    for (i=1; i<total_elems; i++) {
      if (pbase[i].key < pbase[i-1].key)
        printf("Sorting Error! [%f %f]\n", pbase[i-1].key, pbase[i].key);
    }
  }
#endif
}
