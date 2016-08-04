#include "MPI_section.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#pragma weak MPI_Section_check = PMPI_Section_check
#pragma weak MPI_Section_enter = PMPI_Section_enter
#pragma weak MPI_Section_pop = PMPI_Section_pop
#pragma weak MPI_Section_enter_cb = PMPI_Section_enter_cb
#pragma weak MPI_Section_leave_cb = PMPI_Section_leave_cb



/** MPI_Section communicator initialization
 */

pthread_mutex_t section_init_lock = PTHREAD_MUTEX_INITIALIZER;
static volatile int mpi_section_comm_initialized = 0;
MPI_Comm mpi_section_comm;

void mpi_section_check_init()
{
	int mpi_running = 0;
	MPI_Initialized(&mpi_running);
	
	if( !mpi_running )
		return;
	
	
	/* Fast path */
	if( !mpi_section_comm_initialized )
	{
		/* Slow path */
		pthread_mutex_lock( &section_init_lock );
		if( !mpi_section_comm_initialized )
		{
			mpi_section_comm_initialized = 1;
			MPI_Comm_dup( MPI_COMM_WORLD, &mpi_section_comm );
		}
		pthread_mutex_unlock( &section_init_lock );
		
	}
	
}

/** MPI_Section Stack management
 */

struct MPI_Section_s
{
	MPI_Comm comm;
	char label[64];
	uint64_t arg;
	struct MPI_Section_s * prev;
};


pthread_mutex_t section_stack_lock = PTHREAD_MUTEX_INITIALIZER;
static __thread struct MPI_Section_s * section_stack = NULL;

struct MPI_Section_s * MPI_Section_s_push( MPI_Comm comm, char * label )
{
	struct MPI_Section_s * ret = malloc( sizeof( struct MPI_Section_s ) * 2);
	
	if( !ret )
	{
		perror("malloc");
		return NULL;
	}
	
	ret->comm = comm;
	snprintf( ret->label, 64, "%s", label);
	
	pthread_mutex_lock( &section_init_lock );
	ret->prev = section_stack;
	section_stack = ret;
	pthread_mutex_unlock( &section_init_lock );
	
	return ret;
}


struct MPI_Section_s * MPI_Section_s_pop()
{
	struct MPI_Section_s * ret = NULL;
	
	pthread_mutex_lock( &section_init_lock );
	ret = section_stack;
	if( ret )
		section_stack = ret->prev;
	pthread_mutex_unlock( &section_init_lock );
	
	return ret;
}


void MPI_Section_s_release( struct MPI_Section_s * section )
{
	memset( section, 0, sizeof( struct MPI_Section_s ));
	free( section );
}


static inline int MPI_Section_s_compare( struct MPI_Section_s * local, struct MPI_Section_s * remote )
{

	if( strcmp( local->label, remote->label ) )
	{
		return 0;
	}
	
	return 1;
}


/** MPI_Section Calls
 */

static int __section_do_check = 0;

int PMPI_Section_check( int do_check )
{
	MPI_Barrier( MPI_COMM_WORLD );
	__section_do_check = do_check;
 
	return MPI_SUCCESS;
}


int PMPI_Section_enter( MPI_Comm comm, char * label )
{
	mpi_section_check_init();
	
	struct MPI_Section_s *new = MPI_Section_s_push( comm, label );

	
	
	if( __section_do_check )
	{
		/* Now check validity */
		int right_neigh = -1;
		int left_neigh = -1;
		
		int ret;
		int size, rank;

		ret = PMPI_Comm_size( comm, &size );
		
		if( ret != MPI_SUCCESS ) return ret;
		
		ret = PMPI_Comm_rank( comm, &rank );
		
		if( ret != MPI_SUCCESS ) return ret;
		
		if( rank < (size - 1) )
		{
			right_neigh = rank + 1; 
		}
		
		if( rank )
		{
			left_neigh = rank - 1;
		}

		int did_ok = 1;
		
		struct MPI_Section_s remote;
		
		if( 0 <= right_neigh )
		{
			ret = MPI_Recv( &remote, sizeof( struct MPI_Section_s ), MPI_CHAR, right_neigh , 128, comm, MPI_STATUS_IGNORE);
			if( ret != MPI_SUCCESS ) return ret;
			
			if( MPI_Section_s_compare( new, &remote ) == 0 )
			{
				did_ok = 0;
			}
		}
		
		if( 0 <= left_neigh )
		{
			ret = MPI_Send( new, sizeof( struct MPI_Section_s ), MPI_CHAR, left_neigh , 128, comm);
			if( ret != MPI_SUCCESS ) return ret;
		}
		
		if( !did_ok )
		{
			fprintf(stderr,"Error MPI_Section_enter was called with mismatching parameters\n");
			return MPI_ERR_ARG;
		}
	
	}
	
	MPI_Section_enter_cb( label , &new->arg );
	
	
	return MPI_SUCCESS;
}


int PMPI_Section_pop( MPI_Comm comm )
{
	if( !mpi_section_comm_initialized )
	{
		fprintf(stderr,"Error MPI_Section_pop was called prior to a MPI_Section_enter\n");
		return MPI_ERR_ARG;
	}
	
	/* Check Enter/Leave matching */
	struct MPI_Section_s *popped = MPI_Section_s_pop();
	
	int ret = MPI_SUCCESS;
	
	/* Underflow case */
	if( !popped )
	{
		fprintf(stderr, "Error section undeflow detected\n");
		ret = MPI_ERR_ARG;
	}
	else
	{
		int comm_matches = MPI_UNEQUAL;
		
		int ret = MPI_Comm_compare( popped->comm, comm, &comm_matches );
		
		if( ret == MPI_SUCCESS )
		{
			if( comm_matches != MPI_IDENT )
			{
				fprintf(stderr, "Error MISMATCHING communicators in MPI_Section_pop\n");
				ret = MPI_ERR_ARG;
			}
		}
		else
		{
			fprintf(stderr, "Error when comparating communicators in MPI_Section_pop\n");
		}
	}
	
	MPI_Section_leave_cb( popped->label, popped->arg  );
	
	MPI_Section_s_release( popped );
	
	return ret;	
}


int PMPI_Section_enter_cb( char * label, uint64_t *arg )
{
	
	return MPI_SUCCESS;
}


int PMPI_Section_leave_cb( char * label,  uint64_t arg )
{
	
	return MPI_SUCCESS;
}
