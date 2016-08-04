#include "MPI_section.h"



int main( int argc, char ** argv )
{
	MPI_Init( &argc, &argv );

	int rank;
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );

	MPI_Section_enter( MPI_COMM_WORLD, "main");
	
	int i;

	for( i = 0 ; i < 8000 ; i++ )
	{
	MPI_Section_enter( MPI_COMM_WORLD, "sub");

	usleep( i/500 + rank * 10  );

	MPI_Section_pop( MPI_COMM_WORLD );
	}

	MPI_Section_pop( MPI_COMM_WORLD );

	MPI_Finalize();

	return 0;
}
