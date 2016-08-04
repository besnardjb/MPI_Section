#ifndef MPI_SECTION_H
#define MPI_SECTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mpi.h>
#include <stdint.h>

/**
 * MPI_Section definition
 */

/** Opaque forward-reference of the 
 *  MPI_Section structure describing
 *  an instance of a section */
struct MPI_Section_s;

/** The definition of the MPI_Section data-type */
typedef struct MPI_Section_s* MPI_Section;

/**
 * MPI_Section labelling
 */

/** This defines if section calls are checked (matching)
 * 
 * \arg this call is collective over COMM_WORLD
 * 
 * \arg do_check set the mode
 * \return MPI_SUCCESS if node changed
 */

int MPI_Section_check( int do_check );
int PMPI_Section_check( int do_check );

/** This is the main MPI_Section call
 * 
 * \warning This call is a collective on the communicator
 *           The fact that the call is done with the same
 *           label on every processes is checked by \ref MPI_Section_check
 *           MPMD programs may use different labels though
 * 
 * Use this call to enter a section with a given label
 * on a given communicator. This call is non-blocking
 * and the implementation shall limit its overhead.
 * 
 * 
 * \arg comm The communicator where the section is outlined
 * \arg label The label of the section
 * \return MPI_SUCCESS when successful
 */
int MPI_Section_enter( MPI_Comm comm, char * label );
int PMPI_Section_enter( MPI_Comm comm, char * label );

/** This Pops an MPI_Section context
 *
 * \warning This function shall be called once
 *           for each entered section by all the
 *           members of the communicator passed in
 *           parameter not doing so would lead to an error
 * 
 * This call leaves a section and triggers the
 * associated handlers. Calls are doing nothing
 * by default and can be overridden by a tool to start
 * collecting events (with a cost associated to the tool).
 */
int MPI_Section_pop( MPI_Comm comm ); 
int PMPI_Section_pop( MPI_Comm comm ); 

/**
 * MPI_Section notification
 */

/** This function is called for each MPI rank when entering a section
 * \arg label Label of the section
 * \return Tools shall retunrn MPI_SUCCESS
 */
int MPI_Section_enter_cb( char * label, uint64_t *arg  );
int PMPI_Section_enter_cb( char * label, uint64_t *arg  );

/** This function is called for each MPI rank when leaving a section
 * \arg label Label of the section
 * \return Tools shall retunrn MPI_SUCCESS
 */
int MPI_Section_leave_cb( char * label,  uint64_t arg );
int PMPI_Section_leave_cb( char * label,  uint64_t arg );


#ifdef __cplusplus
}
#endif

#endif
