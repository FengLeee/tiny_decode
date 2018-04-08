/*
 * wfst.cc
 *
 *  Created on : 7/3/2014
 *      Author : zhiming.wang
 */

# include "wfst.h"

WFST::WFST() : graph(NULL) , num_state_node(0) , start_state(0)
{
	initialize() ;
}

WFST::~WFST()
{
	release() ;
}

/* initialize */
void WFST::initialize()
{
	graph = NULL ;
}

/* release */
void WFST::release()
{
	if(NULL != graph)
	{
		for(int64 i = start_state ; i < num_state_node ; ++i)
		{
			free(graph[i].arc) ;
			graph[i].arc = NULL ;
		}
		free(graph) ;
		graph = NULL ;
	}
}

/* read wfst file */
void WFST::read(const char* wfst_file)
{
	FILE* f = fopen(wfst_file , "rb") ;
	if(NULL == f)
	{
		char err[100] ;
		sprintf(err , "error : failed to open wfst file : %s" , wfst_file) ;
		error(err , __FILE__ , __LINE__) ;
	}

	unsigned char quad_bytes[4] ;
	char fst_type[128] , arc_type[128] ;
	int32 type_len , version , flag ;
	int64 property , num_arc ;

	/* the first 4 bytes are flags for indicating fst type , e.g. "vector" or "const" */
	fread(quad_bytes , 1 , 4 , f) ;
	assert((0xD6 == quad_bytes[0]) && (0xFD == quad_bytes[1]) && (0xB2 == quad_bytes[2]) && (0x7E == quad_bytes[3])) ;

	fread(&type_len , sizeof(int32) , 1 , f) ;

	fread(fst_type , 1 , type_len , f) ;
	fst_type[type_len] = '\0' ;

	/* arc type , e.g. , "standard" */
	fread(&type_len , sizeof(int32) , 1 , f) ;

	fread(arc_type , 1 , type_len , f) ;
	arc_type[type_len] = '\0' ;

	/* version */
	fread(&version , sizeof(int32) , 1 , f) ;

	/* flag --- file format bits */
	fread(&flag , sizeof(int32) , 1 , f) ;

	/* property */
	fread(&property , sizeof(int64) , 1 , f) ;

	/* start state */
	fread(&start_state , sizeof(int64) , 1 , f) ;

	/* number of states(nodes) */
	fread(&num_state_node , sizeof(int64) , 1 , f) ;

	/* number of arcs */
	fread(&num_arc , sizeof(int64) , 1 , f) ;

	graph = (StateNode*)malloc(sizeof(StateNode) * static_cast<size_t>(num_state_node)) ;
	if(NULL == graph)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

	for(int64 i = start_state ; i < num_state_node ; ++i)
	{
		/* state weight , e.g. , final weight */
		fread(&graph[i].weight , sizeof(BaseFloat) , 1 , f) ;

		/* number of arcs related to current state */
		fread(&num_arc , sizeof(int64) , 1 , f) ;
		graph[i].num_arc = num_arc ;

		graph[i].arc = (Arc_*)malloc(sizeof(Arc_) * static_cast<size_t>(num_arc)) ;
		if(NULL == graph[i].arc)  error("no memory to be allocated" , __FILE__ , __LINE__) ;

		for(int64 j = 0 ; j < num_arc ; j++)
		{
			/* input label */
			fread(&graph[i].arc[j].input_label , sizeof(int32) , 1 , f) ;

			/* output label */
			fread(&graph[i].arc[j].output_label , sizeof(int32) , 1 , f) ;

			/* arc weight */
			fread(&graph[i].arc[j].arc_weight , sizeof(BaseFloat) , 1 , f) ;

			/* next state */
			fread(&graph[i].arc[j].next_state , sizeof(int32) , 1 , f) ;
		}
	}

	fclose(f) ;
}

const StateNode& WFST::state(int64 state_index) const
{
	assert(state_index >= start_state && state_index < num_state_node) ;
	return graph[state_index] ;
}

int64 WFST::number_of_state_node() const
{
	return num_state_node ;
}

int64 WFST::get_start_state() const
{
	return start_state ;
}
